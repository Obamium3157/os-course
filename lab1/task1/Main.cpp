#include <iostream>

#include "SysInfoWin.h"

void PrintInfo()
{
  PrintSystemVersion();
  PrintComputerName();
  PrintUserName();
  PrintArchitecture();
  PrintRAM(Precision::GB);
  PrintVirtualMemory(Precision::GB);
  PrintMemoryLoad();
  PrintPageFile(Precision::GB);
  PrintProcessorsAmount();
  PrintDrives(Precision::GB);
}

int main()
{
  PrintInfo();
}
