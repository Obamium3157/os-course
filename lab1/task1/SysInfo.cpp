#include "SysInfo.h"

#include <iostream>
#include <optional>
#include <cstring>

#include <lmcons.h>
#include <versionhelpers.h>
#include <psapi.h>

namespace
{
  DWORDLONG GetMeasurementValue(const Measurement p)
  {
    return static_cast<DWORDLONG>(p);
  }
  std::string MeasurementToString(const Measurement precision)
  {
    switch (precision)
    {
      case Measurement::KB: return "KB";
      case Measurement::MB: return "MB";
      case Measurement::GB: return "GB";
    }

    return "";
  }

  std::string ProcessorArchitectureToString(const WORD architecture)
  {
    switch (architecture)
    {
      case PROCESSOR_ARCHITECTURE_AMD64:
        return "x64 (AMD or Intel)";
      case PROCESSOR_ARCHITECTURE_ARM:
        return "ARM";
      case PROCESSOR_ARCHITECTURE_ARM64:
        return "ARM64";
      case PROCESSOR_ARCHITECTURE_IA64:
        return "Intel Itanium-based";
      case PROCESSOR_ARCHITECTURE_INTEL:
        return "x86";
      case PROCESSOR_ARCHITECTURE_UNKNOWN:
      default:
        return "Unknown";
    }
  }

  std::string ExtractOSName()
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

    return "Unknown Windows version";
  }

  std::optional<std::string> ExtractComputerName()
  {
    char computerName[CNLEN + 1];
    DWORD size = sizeof(computerName);
    if (!GetComputerNameA(computerName, &size))
    {
      return std::nullopt;
    }

    return computerName;
  }

  std::optional<std::string> ExtractUserName()
  {
    char userName[UNLEN + 1];
    DWORD size = sizeof(userName);
    if (!GetUserNameA(userName, &size))
    {
      return std::nullopt;
    }

    return userName;
  }

  std::string ExtractArchitecture()
  {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    return ProcessorArchitectureToString(systemInfo.wProcessorArchitecture);
  }

  DWORD ExtractProcessorsAmount()
  {
    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    return systemInfo.dwNumberOfProcessors;
  }

  std::optional<RAMInfo> ExtractRAMInfo()
  {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
      return std::nullopt;
    }

    return RAMInfo{
      .totalRAM = statex.ullTotalPhys,
      .freeRAM = statex.ullAvailPhys,
    };
  }

  std::optional<SIZE_T> ExtractVMInfo()
  {
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);

    if (!GetPerformanceInfo(&pi, pi.cb))
    {
      return std::nullopt;
    }

    return pi.CommitLimit * pi.PageSize;
  }

  std::optional<DWORD> ExtractMemoryLoad()
  {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
      return std::nullopt;
    }

    return statex.dwMemoryLoad;
  }

  std::optional<PagefileInfo> ExtractPageFileInfo()
  {
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);
    if (!GetPerformanceInfo(&pi, pi.cb))
    {
      return std::nullopt;
    }

    const auto totalPagefileSize = pi.CommitLimit * pi.PageSize;
    const auto committedPagefileSize = pi.CommitTotal * pi.PageSize;

    return PagefileInfo{
      .commitLimit = totalPagefileSize,
      .commitTotal = committedPagefileSize,
    };
  }

  std::optional<std::vector<std::string>> ExtractDriveNames()
  {
    const DWORD bufferSize = GetLogicalDriveStringsA(0, nullptr);
    if (bufferSize == 0)
    {
      return std::nullopt;
    }

    std::vector<char> buffer(bufferSize);
    if (GetLogicalDriveStringsA(bufferSize, buffer.data()) == 0)
    {
      return std::nullopt;
    }

    std::vector<std::string> driveNames;
    for (const char* pDrive = buffer.data(); *pDrive != '\0'; pDrive += strlen(pDrive) + 1)
    {
      driveNames.emplace_back(pDrive);
    }

    return driveNames;
  }

  std::optional<DiskUsageInfo> ExtractDiskUsage(const std::string& driveName)
  {
    DiskUsageInfo usageInfo{};
    if (!GetDiskFreeSpaceExA(driveName.c_str(),
      &usageInfo.freeBytesAvailableToCaller,
      &usageInfo.totalNumberOfBytes,
      &usageInfo.totalNumberOfFreeBytes))
    {
      return std::nullopt;
    }

    return usageInfo;
  }

  std::vector<DriveState> ExtractDriveStates()
  {
    const auto driveNames = ExtractDriveNames();
    if (!driveNames)
    {
      return {};
    }

    std::vector<DriveState> driveStates;
    driveStates.reserve(driveNames->size());

    for (const auto& driveName : *driveNames)
    {
      if (auto usage = ExtractDiskUsage(driveName))
      {
        driveStates.push_back({.driveName = driveName, .usageInfo = *usage});
      }
    }

    return driveStates;
  }

  std::string DriveStateToString(const DriveState& driveState, const Measurement measurement)
  {
    const auto mv = GetMeasurementValue(measurement);
    const auto ms = MeasurementToString(measurement);

    const auto totalSpace = driveState.usageInfo.totalNumberOfBytes.QuadPart;
    const auto freeSpace = driveState.usageInfo.totalNumberOfFreeBytes.QuadPart;

    return driveState.driveName + ": " + std::to_string(freeSpace / mv) + " " + ms + " free / "
      + std::to_string(totalSpace / mv) + " " + ms + " total";

  }
}


SysInfo::SysInfo(const Measurement measurement)
  : m_measurement(measurement)
  , m_osInfo({
    .osName = ExtractOSName(),
    .computerName = ExtractComputerName(),
    .userName = ExtractUserName(),
    .architecture = ExtractArchitecture()
  })
  , m_hwInfo(HardwareInfo{
    .ramInfo = ExtractRAMInfo(),
    .virtualMemory = ExtractVMInfo(),
    .memoryLoad = ExtractMemoryLoad(),
    .pageFileInfo = ExtractPageFileInfo(),
    .processorsAmount = ExtractProcessorsAmount(),
  })
  , m_driveStates(ExtractDriveStates())
{
}

void SysInfo::PrintInfo() const
{
  std::cout << "OS Name: " << m_osInfo.osName << "\n";
  std::cout << "Computer Name: " << ComputerNameToString() << "\n";
  std::cout << "User Name: " << UserNameToString() << "\n";
  std::cout << "Architecture: " << m_osInfo.architecture << "\n";
  std::cout << "RAM: " << RAMInfoToString() << "\n";
  std::cout << "Virtual Memory: " << VMToString() << "\n";
  std::cout << "Memory Load: " << MemoryLoadToString() << "\n";
  std::cout << "Pagefile: " << PagefileToString() << "\n";
  std::cout << "Processors: " << m_hwInfo.processorsAmount << "\n";
  std::cout << "Drives: \n" + DriveStatesToString();
}

std::string SysInfo::ComputerNameToString() const
{
  if (!m_osInfo.computerName)
  {
    return "unavailable";
  }

  return *m_osInfo.computerName;
}

std::string SysInfo::UserNameToString() const
{
  if (!m_osInfo.userName)
  {
    return "unavailable";
  }

  return *m_osInfo.userName;
}

std::string SysInfo::RAMInfoToString() const
{
  if (!m_hwInfo.ramInfo)
  {
    return "unavailable";
  }

  const auto usedRAM = m_hwInfo.ramInfo->totalRAM - m_hwInfo.ramInfo->freeRAM;
  const auto mv = GetMeasurementValue(m_measurement);
  const auto ms = MeasurementToString(m_measurement);

  return std::to_string(usedRAM / mv) + " " + ms
    + " / " + std::to_string(m_hwInfo.ramInfo->totalRAM / mv) + " " + ms;
}

std::string SysInfo::VMToString() const
{
  if (!m_hwInfo.virtualMemory)
  {
    return "unavailable";
  }

  const auto mv = GetMeasurementValue(m_measurement);
  return std::to_string(*m_hwInfo.virtualMemory / mv) + " " + MeasurementToString(m_measurement);
}

std::string SysInfo::MemoryLoadToString() const
{
  if (!m_hwInfo.memoryLoad)
  {
    return "unavailable";
  }

  return std::to_string(*m_hwInfo.memoryLoad) + "%";
}

std::string SysInfo::PagefileToString() const
{
  if (!m_hwInfo.pageFileInfo)
  {
    return "unavailable";
  }

  const auto mv = GetMeasurementValue(m_measurement);
  const auto ms = MeasurementToString(m_measurement);

  return std::to_string(m_hwInfo.pageFileInfo->commitTotal / mv) + " " + ms
    + " / " + std::to_string(m_hwInfo.pageFileInfo->commitLimit / mv) + " " + ms;
}

std::string SysInfo::DriveStatesToString() const
{
  if (m_driveStates.empty())
  {
    return "  - unavailable\n";
  }
  std::string result;
  for (const auto& driveState : m_driveStates)
  {
    result += "  - " + DriveStateToString(driveState, m_measurement) + "\n";
  }

  return result;
}