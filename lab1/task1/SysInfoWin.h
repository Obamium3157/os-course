#ifndef OS_COURSE_SYSINFOWIN_H
#define OS_COURSE_SYSINFOWIN_H

enum class Precision
{
  KB = 1024ULL,
  MB = 1024ULL * KB,
  GB = 1024ULL * MB,
};

void PrintSystemVersion();
void PrintComputerName();
void PrintUserName();
void PrintArchitecture();
void PrintRAM(Precision precision);
void PrintVirtualMemory(Precision precision);
void PrintMemoryLoad();
void PrintPageFile(Precision precision);
void PrintProcessorsAmount();
void PrintDrives(Precision precision);

#endif //OS_COURSE_SYSINFOWIN_H
