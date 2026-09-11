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
  std::optional<std::string> GetOSVersion() const;
  std::optional<uint64_t> GetFreeMemory() const;
  std::optional<uint64_t> GetTotalMemory() const;
  std::optional<unsigned> GetProcessorCount() const;

private:
  std::string m_OSName;
  std::optional<std::string> m_OSVersion;
  std::optional<MemoryInfo> m_memoryInfo;
  std::optional<unsigned> m_processorCount;
};


#endif //OS_COURSE_SYSINFO_H
