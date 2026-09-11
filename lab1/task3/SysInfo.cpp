#if defined(_WIN32)

#include <windows.h>
#include <lmcons.h>
#include <versionhelpers.h>
#include <psapi.h>

#elif defined(__linux__)

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#endif

#include <stdexcept>
#include <optional>
#include <fstream>

#include "SysInfo.h"

namespace
{
  struct OSReleaseInfo
  {
    std::string prettyName;
    std::string name;
    std::string versionId;
  };

#if defined(_WIN32)
  std::optional<std::string> GetOSVersionWindows()
  {
    if (IsWindowsServer())
    {
      return "Windows Server";
    }
    if (IsWindows10OrGreater())
    {
      return "Windows 10 or Greater";
    }
    if (IsWindows8OrGreater())
    {
      return "Windows 8 or Greater";
    }
    if (IsWindows7OrGreater())
    {
      return "Windows 7 or Greater";
    }
    if (IsWindowsVistaOrGreater())
    {
      return "Windows Vista or Greater";
    }
    if (IsWindowsXPOrGreater())
    {
      return "Windows XP or Greater";
    }

    return std::nullopt;
  }
#elif defined(__linux__)
  OSReleaseInfo ReadOSReleaseInfoFromFile(std::ifstream& file)
  {
    OSReleaseInfo info{};
    std::string line;
    while (std::getline(file, line))
    {
      const std::size_t equals = line.find('=');
      if (equals == std::string::npos)
      {
        continue;
      }

      std::string key = line.substr(0, equals);
      std::string value = line.substr(equals + 1);

      if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
      {
        value = value.substr(1, value.size() - 2);
      }

      if (key == "PRETTY_NAME")
      {
        info.prettyName = value;
      }
      else if (key == "NAME")
      {
        info.name = value;
      }
      else if (key == "VERSION_ID")
      {
        info.versionId = value;
      }
    }

    return info;
  }
  std::optional<std::string> GetOSVersionLinux()
  {
    std::ifstream file("/etc/os-release");
    if (!file.is_open())
    {
      return std::nullopt;
    }

    const auto [prettyName, name, versionId] = ReadOSReleaseInfoFromFile(file);

    if (!prettyName.empty())
    {
      return prettyName;
    }
    if (!name.empty())
    {
      return versionId.empty()
      ? name
         : name + " " + versionId;
    }

    return std::nullopt;
  }
#endif

#if defined(_WIN32)
  std::optional<MemoryInfo> GetMemoryInfoWindows()
  {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
      return std::nullopt;
    }

    return MemoryInfo{
      .free = statex.ullAvailPhys / 1024UL / 1024,
      .total = statex.ullTotalPhys / 1024UL / 1024,
    };
  }
#elif defined(__linux__)
  std::optional<uint64_t> GetFreeMemoryLinux()
  {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open())
    {
      return std::nullopt;
    }

    std::string token;
    while (file >> token)
    {
      if (token == "MemAvailable:")
      {
        if (unsigned long mem; file >> mem)
        {
          return mem / 1024UL;
        }
        return std::nullopt;
      }
    }

    return std::nullopt;
  }

  std::optional<MemoryInfo> GetMemoryInfoLinux()
  {
    const auto freeMemory = GetFreeMemoryLinux();
    if (!freeMemory)
    {
      return std::nullopt;
    }

    struct sysinfo buffer{};
    if (sysinfo(&buffer) != 0)
    {
      return std::nullopt;
    }

    return MemoryInfo{
      .free = *freeMemory,
      .total = buffer.totalram / 1024UL / 1024
    };
  }
#endif

#if defined(_WIN32)
  std::optional<unsigned> GetProcessorCountWindows()
  {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    return systemInfo.dwNumberOfProcessors;
  }
#elif defined(__linux__)
  std::optional<unsigned> GetProcessorCountLinux()
  {
    const auto np = get_nprocs();
    if (np == -1)
    {
      return std::nullopt;
    }

    return static_cast<unsigned>(np);
  }
#endif

  std::string GetOSNameImpl()
  {
#if defined(_WIN32)
    return "Windows";
#elif defined(__linux__)
    return "Linux";
#endif
  }

  std::optional<std::string> GetOSVersionImpl()
  {
#if defined(_WIN32)
    return GetOSVersionWindows();
#elif defined(__linux__)
    return GetOSVersionLinux();
#endif
  }

  std::optional<MemoryInfo> GetMemoryInfoImpl()
  {
#if defined(_WIN32)
    return GetMemoryInfoWindows();
#elif defined(__linux__)
    return GetMemoryInfoLinux();
#endif
  }

  std::optional<unsigned> GetProcessorCountImpl()
  {
#if defined(_WIN32)
    return GetProcessorCountWindows();
#elif defined(__linux__)
    return GetProcessorCountLinux();
#endif
  }
}

SysInfo::SysInfo()
  : m_OSName(GetOSNameImpl())
  , m_OSVersion(GetOSVersionImpl())
  , m_memoryInfo(GetMemoryInfoImpl())
  , m_processorCount(GetProcessorCountImpl())
{
}

std::string SysInfo::GetOSName() const
{
  return m_OSName;
}

std::optional<std::string> SysInfo::GetOSVersion() const
{
  return *m_OSVersion;
}

std::optional<uint64_t> SysInfo::GetFreeMemory() const
{
  return m_memoryInfo->free;
}

std::optional<uint64_t> SysInfo::GetTotalMemory() const
{
  return m_memoryInfo->total;
}

std::optional<unsigned> SysInfo::GetProcessorCount() const
{
  return m_processorCount;
}