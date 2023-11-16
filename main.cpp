#include "lib/ConfigUtil.h"

int main(int argc, char* argv[]) {
  ConfigUtil conf;
  conf.SplitString(argc, argv);
  conf.PrintString();
}