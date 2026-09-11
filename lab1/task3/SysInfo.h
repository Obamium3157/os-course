#ifndef OS_COURSE_SYSINFO_H
#define OS_COURSE_SYSINFO_H

#include <string>
#include <cstdint>
#include <optional>

struct MemoryInfo
{
  uint64_t free;
  uint64_t total;
};

class SysInfo
{
public:
  SysInfo();

  std::string GetOSName() const;
  std::string GetOSVersion() const;
  uint64_t GetFreeMemory() const;
  uint64_t GetTotalMemory() const;
  unsigned GetProcessorCount() const;

private:
  std::string m_OSName;
  std::optional<std::string> m_OSVersion;
  std::optional<MemoryInfo> m_memoryInfo;
};


#endif //OS_COURSE_SYSINFO_H
