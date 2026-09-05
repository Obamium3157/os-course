#ifndef OS_COURSE_SYSINFO_H
#define OS_COURSE_SYSINFO_H

#include "SysInfoWin.h"

#include <string>
#include <vector>

#include <windows.h>

struct OSInfo
{
  std::string osName;
  std::string computerName;
  std::string userName;
  std::string architecture;
};

struct RAMInfo
{
  DWORDLONG totalRAM;
  DWORDLONG freeRAM;
};

struct PagefileInfo
{
  SIZE_T commitLimit;
  SIZE_T commitTotal;
  SIZE_T pageSize;
};

struct HardwareInfo
{
  RAMInfo ramInfo;
  SIZE_T virtualMemory;
  DWORD memoryLoad;
  PagefileInfo pageFileInfo;
  DWORD processorsAmount;
};

struct DiskUsageInfo
{
  ULARGE_INTEGER freeBytesAvailableToCaller;
  ULARGE_INTEGER totalNumberOfBytes;
  ULARGE_INTEGER totalNumberOfFreeBytes;
};

struct DriveState
{
  std::string driveName;
  DiskUsageInfo usageInfo;
};

enum class Measurement
{
  KB = 1024ULL,
  MB = 1024ULL * KB,
  GB = 1024ULL * MB,
};

class SysInfo
{
public:
  SysInfo(Measurement measurement);

  void PrintInfo() const;

private:
  std::string RAMInfoToString() const;
  std::string VMToString() const;
  std::string MemoryLoadToString() const;
  std::string PagefileToString() const;
  std::string DriveStatesToString() const;

  const Measurement m_measurement;

  const OSInfo m_osInfo;
  HardwareInfo m_hwInfo;

  std::vector<DriveState> m_driveStates;
};


#endif //OS_COURSE_SYSINFO_H
