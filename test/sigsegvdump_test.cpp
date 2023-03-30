#include <bfd.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <memory>
#include <sstream>
// void signal_handler(int sig) {
//   void* callstack[128];
//   int num_frames = backtrace(callstack, 128);
//   char** strs = backtrace_symbols(callstack, num_frames);

//   printf("Caught signal %d\n", sig);
//   printf("Call stack:\n");
//   for (int i = 0; i < num_frames; i++) {
//     printf("%s\n", strs[i]);
//   }

//   printf("Stack data:\n");
//   bfd_init();

//   for (int i = 0; i < num_frames; i++) {
//     Dl_info info;
//     dladdr(callstack[i], &info);

//     const char* symname = info.dli_sname;
//     int status;
//     char* demangled = abi::__cxa_demangle(symname, NULL, 0, &status);

//     if (demangled == NULL) {
//       demangled = (char*)symname;
//     }
//     printf("[%d] %s: %s\n", i, demangled, info.dli_fname);

//     void* sp = callstack[i];
//     int* bp = (int*)__builtin_frame_address(1);
//     while (bp != NULL && sp != NULL) {
//       for (int* p = (int*)sp; p < bp; p++) {
//         Dl_info info;
//         dladdr((void*)*p, &info);

//         const char* symname = info.dli_sname;
//         int status;
//         char* demangled = abi::__cxa_demangle(symname, NULL, 0, &status);

//         if (demangled == NULL) {
//           demangled = (char*)symname;
//         }

//         printf("\t%p: %s\n", (void*)p, demangled);

//         free(demangled);
//       }
//       sp = bp;
//       bp = (int*)(*bp);
//     }
//   }

//   free(strs);

//   exit(1);
// }
#include <bfd.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void signal_handler(int sig) {
  pthread_mutex_lock(&mutex);

  void* callstack[128];
  int num_frames = backtrace(callstack, 128);
  char** strs = backtrace_symbols(callstack, num_frames);

  printf("Caught signal %d\n", sig);
  printf("Call stack:\n");
  for (int i = 0; i < num_frames; i++) {
    printf("%s\n", strs[i]);
  }

  printf("Stack data:\n");
  bfd_init();

  for (int i = 0; i < num_frames; i++) {
    Dl_info info;
    dladdr(callstack[i], &info);

    const char* symname = info.dli_sname;
    int status;
    char* demangled = abi::__cxa_demangle(symname, NULL, 0, &status);

    if (demangled == NULL) {
      demangled = (char*)symname;
    }
    printf("[%d] %s: %s\n", i, demangled, info.dli_fname);

    void* sp = callstack[i];
    int* bp = (int*)__builtin_frame_address(1);
    while (bp != NULL && sp != NULL) {
      for (int* p = (int*)sp; p < bp; p++) {
        std::stringstream cmd;
        Dl_info info;
        dladdr((void*)*p, &info);

        const char* symname = info.dli_sname;
        int status;
        char* demangled = abi::__cxa_demangle(symname, NULL, 0, &status);

        if (demangled == NULL) {
          demangled = (char*)symname;
        }

        printf("\t%p: %s\n", (void*)p, demangled);
        // cmd << "addr2line -e " << info.dli_fname << " -f -C -p "
        //     << callstack[i];
        // std::cout << cmd.str().c_str() << std::endl;

        free(demangled);
      }
      sp = bp;
      bp = (int*)(*bp);
    }
  }

  free(strs);

  pthread_mutex_unlock(&mutex);

  exit(1);
}

void fun1(){
  char* ptr = "aaabbb";
  *ptr = 'a';

}
int main() {
  // signal(SIGSEGV, signal_handler);
  fun1();
  return 0;
}