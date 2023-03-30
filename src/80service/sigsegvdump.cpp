#include <cxxabi.h>
#include <execinfo.h>
#include <signal.h>
#include <unistd.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>

// #include"../../Include/80service/sigsegvdump.h"

#ifdef __cplusplus
extern "C" {
#endif

void initializeSigsegvHandler(const char *appName, int dumpLines);

#ifdef __cplusplus
}
#endif

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&...args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class SigsegvHandler {
 public:
  SigsegvHandler(const char *appName, int dumpLines)
      : m_appName(appName), m_dumpLines(dumpLines) {}

  void operator()(int sig) {
    fprintf(stderr, "\nSegmentation fault detected, dumping stack trace...\n");
    dumpStackTrace(m_appName, m_dumpLines);
    exit(EXIT_FAILURE);
  }

 private:
  const char *m_appName;
  int m_dumpLines;

  void dumpStackTrace(const char *appName, int dumpLines) {
    // ... (same as before)
    void *buffer[dumpLines];
    int nptrs = backtrace(buffer, dumpLines);

    char **strings = backtrace_symbols(buffer, nptrs);
    if (strings == nullptr) {
      perror("backtrace_symbols");
      exit(EXIT_FAILURE);
    }

    // 生成导出文件名
    time_t t = time(nullptr);
    char timeBuffer[64];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d_%H%M%S", localtime(&t));
    char fileName[256];
    snprintf(fileName, sizeof(fileName), "%s_%s.log", appName, timeBuffer);

    // 打开文件
    FILE *file = fopen(fileName, "w");
    if (file == nullptr) {
      perror("fopen");
      exit(EXIT_FAILURE);
    }
    // 解析并导出调用堆栈
    for (int i = 0; i < nptrs; ++i) {
      std::stringstream cmd;
      cmd << "addr2line -e " << appName << " -f -C -p " << buffer[i];
      std::array<char, 128> lineBuffer;
      std::string result;
      FILE *pipe = popen(cmd.str().c_str(), "r");
      if (!pipe) {
        perror("popen");
        exit(EXIT_FAILURE);
      }
      while (fgets(lineBuffer.data(), lineBuffer.size(), pipe) != nullptr) {
        result += lineBuffer.data();
      }
      pclose(pipe);
      fprintf(file, "%s", result.c_str());
    }

    // 关闭文件并释放资源
    fclose(file);
    free(strings);
  }
};

SigsegvHandler *g_handler = nullptr;

extern "C" void sigsegvHandler(int sig, siginfo_t *info, void *context) {
  // auto handler = reinterpret_cast<SigsegvHandler
  // *>(info->si_value.sival_ptr);
  (*g_handler)(sig);
}

void initializeSigsegvHandler(const char *appName, int dumpLines) {
  g_handler = new SigsegvHandler(appName, dumpLines);
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = &sigsegvHandler;
  sa.sa_flags = SA_SIGINFO;
  sigfillset(&sa.sa_mask);

  sigval sv;
  // sv.sival_ptr = g_handler.get();
  sigaction(SIGSEGV, &sa, nullptr);

  // Intentionally leak the handler object, as we can't free it safely.
  // handler.release();
}
