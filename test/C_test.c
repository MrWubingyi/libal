#include <stdio.h>

#include "../include/sigsegvdump.h"

void fun2() {
  char *ptr = NULL;
  *ptr = 'a';
}

void fun1() { fun2(); }

int main(int argc, char **argv) {
  initializeSigsegvHandler("a.out", 100);
  // 在这里执行你的应用程序代码
  // 如果发生段错误，sigsegvHandler将被调用并导出堆栈信息

  // 产生段错误
  fun1();
  return 0;
}