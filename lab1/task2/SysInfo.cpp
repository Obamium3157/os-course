#include "SysInfo.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <pwd.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <mntent.h>
#include <unordered_set>

#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/statvfs.h>


namespace
{
  struct OSReleaseInfo
  {
    std::string prettyName;
    std::string name;
    std::string versionId;
  };

  enum class SysInfoField
  {
    KernelVersion,
    Architecture,
    Hostname,
  };

  enum class SysInfoPairType
  {
    RAM,
    Swap,
  };

  unsigned long MBConv(const unsigned long val)
  {
    constexpr unsigned long MB = 1024UL * 1024;
    return val / MB;
  }

  unsigned long GBConv(const unsigned long val)
  {
    return MBConv(val) / 1024;
  }

  std::string MBFormat(const unsigned long val)
  {
    const std::string MB_S = "MB";
    return std::to_string(val) + " " + MB_S;
  }

  std::string GBFormat(const unsigned long val)
  {
    const std::string GB_S = "GB";
    return std::to_string(val) + " " + GB_S;
  }

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

  std::optional<unsigned long> ReadRAMFromFile(std::ifstream& file)
  {
    std::string token;
    while (file >> token)
    {
      if (token == "MemAvailable:")
      {
        if (unsigned long mem; file >> mem)
        {
          return mem;
        }
        return std::nullopt;
      }
    }

    return std::nullopt;
  }

  std::optional<unsigned long> ReadVirtualMemoryFromFile(std::ifstream& file)
  {
    std::string token;
    while (file >> token)
    {
      if (token == "VmallocTotal:")
      {
        if (unsigned long vmem; file >> vmem)
        {
          return vmem;
        }
        return std::nullopt;
      }
    }

    return std::nullopt;
  }

  std::optional<LoadAverage> ReadLoadAverageFromFile(std::ifstream& file)
  {
    if (std::string line; std::getline(file, line))
    {
      LoadAverage avg{};
      std::stringstream ss{line};
      if (!(ss >> avg.oneMinute))
      {
        return std::nullopt;
      }
      if (!(ss >> avg.fiveMinute))
      {
        return std::nullopt;
      }
      if (!(ss >> avg.fifteenMinute))
      {
        return std::nullopt;
      }

      return avg;
    }

    return std::nullopt;
  }

  std::optional<std::string> GetOSVersion()
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

  std::optional<std::string> GetField(const SysInfoField field)
  {
    utsname buffer{};
    if (uname(&buffer) != 0)
    {
      return std::nullopt;
    }

    switch (field)
    {
      case SysInfoField::KernelVersion:
        return std::string(buffer.sysname) + " " + buffer.release;
      case SysInfoField::Architecture:
        return buffer.machine;
      case SysInfoField::Hostname:
        return buffer.nodename;
    }

    return std::nullopt;
  }

  std::optional<std::string> GetUsername()
  {
    if (const passwd* pw = getpwuid(getuid()))
    {
      return pw->pw_name;
    }
    if (const char* user = std::getenv("USER"))
    {
      return user;
    }

    return std::nullopt;
  }

  std::optional<unsigned long> GetFreeRAM()
  {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open())
    {
      return std::nullopt;
    }

    return ReadRAMFromFile(file);
  }

  std::optional<unsigned long> GetVirtualMemory()
  {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open())
    {
      return std::nullopt;
    }

    return ReadVirtualMemoryFromFile(file);
  }

  std::optional<unsigned long> GetProcessorsAmount()
  {
    // TODO: что такое np
    const auto np = get_nprocs();
    if (np == -1)
    {
      return std::nullopt;
    }

    return static_cast<unsigned long>(np);
  }

  std::optional<LoadAverage> GetLoadAverage()
  {
    std::ifstream file("/proc/loadavg");
    if (!file.is_open())
    {
      return std::nullopt;
    }

    return ReadLoadAverageFromFile(file);
  }

  std::optional<SysinfoPair> GetSysInfo(const SysInfoPairType type)
  {
    struct sysinfo buffer{};
    if (sysinfo(&buffer) != 0)
    {
      return std::nullopt;
    }

    switch (type)
    {
      case SysInfoPairType::RAM:
      {
        const auto freeRAM = GetFreeRAM();
        if (!freeRAM)
        {
          return std::nullopt;
        }

        return SysinfoPair{
          .total = buffer.totalram,
          .free = *freeRAM / 1024UL,
        };
      }
      case SysInfoPairType::Swap:
        return SysinfoPair{
        .total = buffer.totalswap,
        .free = buffer.freeswap,
      };
    }

    return std::nullopt;
  }

  std::unordered_set<std::string> GetVirtualFilesystems()
  {
    std::ifstream file("/proc/filesystems");
    if (!file.is_open())
    {
      return {};
    }

    std::unordered_set<std::string> virtualFSs;
    std::string line;
    while (std::getline(file, line))
    {
      if (line.rfind("nodev\t", 0) == 0)
      {
        virtualFSs.emplace(line.substr(6));
      }
    }

    return virtualFSs;
  }

  std::optional<std::vector<DriveInfo>> GetDrives()
  {
    const auto virtualFSs = GetVirtualFilesystems();
    const bool virtualFSsListEmpty = virtualFSs.empty();

    FILE* mounts = setmntent("/proc/mounts", "r");
    if (mounts == nullptr)
    {
      return std::nullopt;
    }

    std::vector<DriveInfo> drives;

    while (const mntent* entry = getmntent(mounts))
    {
      const std::string fsType = entry->mnt_type;

      if (!virtualFSsListEmpty && virtualFSs.contains(fsType))
      {
        continue;
      }

      struct statvfs vfs{};
      if (statvfs(entry->mnt_dir, &vfs) != 0)
      {
        continue;
      }

      const unsigned long blockSize = vfs.f_frsize != 0 ? vfs.f_frsize : vfs.f_bsize;
      const unsigned long totalBytes = blockSize * vfs.f_blocks;
      if (totalBytes == 0)
      {
        continue;
      }

      struct stat st{};
      if (stat(entry->mnt_dir, &st) != 0)
      {
        continue;
      }

      drives.push_back(DriveInfo{
        .mountPoint = entry->mnt_dir,
        .filesystem = fsType,
        .totalBytes = totalBytes,
        .freeBytes = blockSize * vfs.f_bavail,
      });
    }

    // TODO: написать RAII-обертку
    endmntent(mounts);

    return drives;
  }
}

SysInfo::SysInfo()
  : m_osInfo({
    .osVersion = GetOSVersion(),
    .kernelVersion = GetField(SysInfoField::KernelVersion),
    .architecture = GetField(SysInfoField::Architecture),
    .hostname = GetField(SysInfoField::Hostname),
    .username = GetUsername(),
  })
  , m_hwInfo({
    .ramInfo = GetSysInfo(SysInfoPairType::RAM),
    .swapInfo = GetSysInfo(SysInfoPairType::Swap),
    .virtualMemory = GetVirtualMemory(),
    .processorsAmount = GetProcessorsAmount(),
    .loadAverage = GetLoadAverage(),
    .drives = GetDrives(),
  })
{
}

void SysInfo::PrintInfo() const
{
  std::cout << "OS: " << m_osInfo.osVersion.value_or("unknown") << "\n";
  std::cout << "Kernel: " << m_osInfo.kernelVersion.value_or("unknown") << "\n";
  std::cout << "Architecture: " << m_osInfo.architecture.value_or("unknown") << "\n";
  std::cout << "Hostname: " << m_osInfo.hostname.value_or("unknown") << "\n";
  std::cout << "User: " << m_osInfo.username.value_or("unknown") << "\n";
  std::cout << "RAM: " << RAMInfoToString() << "\n";
  std::cout << "Swap: " << SwapToString() << "\n";
  std::cout << "Virtual Memory: " << VirtualMemoryToString() << "\n";
  std::cout << "Processors: " << m_hwInfo.processorsAmount.value_or(0) << "\n";
  std::cout << "Load average: " << LoadAverageToString() << "\n";
  std::cout << "Drives: \n";
  std::cout << DrivesToString();
}

std::string SysInfo::RAMInfoToString() const
{
  if (!m_hwInfo.ramInfo)
  {
    return "unavailable";
  }

  return MBFormat(m_hwInfo.ramInfo->free) + " free / "
    + MBFormat(MBConv(m_hwInfo.ramInfo->total)) + " total";
}

std::string SysInfo::SwapToString() const
{
  if (!m_hwInfo.swapInfo)
  {
    return "unavailable";
  }

  return MBFormat(MBConv(m_hwInfo.swapInfo->free)) + " free / "
    + MBFormat(MBConv(m_hwInfo.swapInfo->total)) + " total";
}

std::string SysInfo::VirtualMemoryToString() const
{
  if (!m_hwInfo.virtualMemory)
  {
    return "unavailable";
  }

  return MBFormat(*m_hwInfo.virtualMemory / 1024UL);
}

std::string SysInfo::LoadAverageToString() const
{
  if (!m_hwInfo.loadAverage)
  {
    return "unavailable";
  }

  std::stringstream out;

  out << std::fixed << std::setprecision(2)
    << m_hwInfo.loadAverage->oneMinute << " "
    << m_hwInfo.loadAverage->fiveMinute << " "
    << m_hwInfo.loadAverage->fifteenMinute;

  return out.str();
}

std::string SysInfo::DrivesToString() const
{
  if (!m_hwInfo.drives)
  {
    return "  unavailable";
  }

  std::size_t maxFsLen = 0;
  std::size_t maxMountLen = 0;
  for (const auto& drive : *m_hwInfo.drives)
  {
    maxFsLen = std::max(maxFsLen, drive.filesystem.size());
    maxMountLen = std::max(maxMountLen, drive.mountPoint.size());
  }

  std::stringstream result;
  for (const auto& [
    mountPoint,
    filesystem,
    totalBytes,
    freeBytes] : *m_hwInfo.drives)
  {
    result << "  "
      << std::left << std::setw(static_cast<int>(maxMountLen)) << mountPoint << "   "
      << std::setw(static_cast<int>(maxFsLen)) << filesystem << "   "
      << GBFormat(GBConv(freeBytes)) << " free / "
      << GBFormat(GBConv(totalBytes)) << " total\n";
  }

  return result.str();
}