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
}

SysInfo::SysInfo()
  : m_OSName(GetOSNameImpl())
  , m_OSVersion(GetOSVersionImpl())
{
}

std::string SysInfo::GetOSName() const
{
  return m_OSName;
}

std::string SysInfo::GetOSVersion() const
{
  if (!m_OSVersion)
  {
    return "unknown";
  }

  return *m_OSVersion;
}
