#ifndef _SIGSEGVDUMP_H_
#define _SIGSEGVDUMP_H_

#ifdef __cplusplus
extern "C" {
#endif

void initializeSigsegvHandler(const char *appName, int dumpLines);

#ifdef __cplusplus
}
#endif
#endif  // !
