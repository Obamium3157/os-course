#ifndef OS_COURSE_SYSINFO_H
#define OS_COURSE_SYSINFO_H

#include <string>
#include <optional>
#include <vector>

struct OSInfo
{
  std::optional<std::string> osVersion;
  std::optional<std::string> kernelVersion;
  std::optional<std::string> architecture;
  std::optional<std::string> hostname;
  std::optional<std::string> username;
};

struct SysinfoPair
{
  unsigned long total;
  unsigned long free;
};

struct LoadAverage
{
  double oneMinute;
  double fiveMinute;
  double fifteenMinute;
};

struct DriveInfo
{
  std::string mountPoint;
  std::string filesystem;
  unsigned long totalBytes;
  unsigned long freeBytes;
};

struct HardwareInfo
{
  std::optional<SysinfoPair> ramInfo;
  std::optional<SysinfoPair> swapInfo;
  std::optional<unsigned long> virtualMemory;
  std::optional<unsigned long> processorsAmount;
  std::optional<LoadAverage> loadAverage;
  std::optional<std::vector<DriveInfo>> drives;
};

class SysInfo
{
public:
  SysInfo();

  void PrintInfo() const;

private:
  std::string RAMInfoToString() const;
  std::string SwapToString() const;
  std::string VirtualMemoryToString() const;
  std::string LoadAverageToString() const;
  std::string DrivesToString() const;

  const OSInfo m_osInfo;
  const HardwareInfo m_hwInfo;
};


#endif //OS_COURSE_SYSINFO_H
