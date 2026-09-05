#include "SysInfo.h"

int main()
{
  const SysInfo sysInfo(Measurement::MB);
  sysInfo.PrintInfo();
}
