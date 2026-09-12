#include <iostream>

#include "SysInfo.h"

//TODO: выяснить, какие именно ядра (физ или лог) выводятся


int main()
{
  try
  {
    const SysInfo sysInfo(Measurement::MB);
    sysInfo.PrintInfo();
  }
  catch (std::exception& e)
  {
    std::cout << "Error when calling SysInfo: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
