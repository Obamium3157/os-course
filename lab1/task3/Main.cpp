#include <iostream>

#include "SysInfo.h"

int main()
{
  SysInfo sysInfo{};
  std::cout << sysInfo.GetOSName() << std::endl;
  std::cout << sysInfo.GetOSVersion() << std::endl;
}
