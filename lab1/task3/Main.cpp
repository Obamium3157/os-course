#include <iostream>

#include "SysInfo.h"

// TODO: сделать общий интерфейс для SysInfo и сделать две реализации (Windows & Linux)

int main()
{
  SysInfo sysInfo{};
  std::cout << sysInfo.GetOSName() << std::endl;
  std::cout << *sysInfo.GetOSVersion() << std::endl;
  std::cout << *sysInfo.GetFreeMemory() << std::endl;
  std::cout << *sysInfo.GetTotalMemory() << std::endl;
  std::cout << *sysInfo.GetProcessorCount() << std::endl;
}
