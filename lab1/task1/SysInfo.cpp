#include "SysInfo.h"

#include <iostream>
#include <lmcons.h>
#include <versionhelpers.h>
#include <psapi.h>
#include <stdexcept>

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

  std::string ExtractComputerName()
  {
    char computerName[CNLEN + 1];
    DWORD size = sizeof(computerName);
    if (!GetComputerNameA(computerName, &size))
    {
      throw std::runtime_error("GetComputerName failed");
    }

    return computerName;
  }

  std::string ExtractUserName()
  {
    char userName[UNLEN + 1];
    DWORD size = sizeof(userName);
    if (!GetUserNameA(userName, &size))
    {
      throw std::runtime_error("GetUserName failed");
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

  RAMInfo ExtractRAMInfo()
  {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);

    return {
      .totalRAM = statex.ullTotalPhys,
      .freeRAM = statex.ullAvailPhys,
    };
  }

  SIZE_T ExtractVMInfo()
  {
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);

    if (!GetPerformanceInfo(&pi, pi.cb))
    {
      throw std::runtime_error("Virtual Memory unavailable");
    }

    return pi.CommitLimit * pi.PageSize;
  }

  DWORD ExtractMemoryLoad()
  {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex))
    {
      throw std::runtime_error("Memory Load unavailable");
    }

    return statex.dwMemoryLoad;
  }

  PagefileInfo ExtractPageFileInfo()
  {
    PERFORMANCE_INFORMATION pi;
    pi.cb = sizeof(pi);
    if (!GetPerformanceInfo(&pi, pi.cb))
    {
      throw std::runtime_error("Performance Info unavailable");
    }

    const auto totalPagefileSize = pi.CommitLimit * pi.PageSize;
    const auto commitedPagefileSize = pi.CommitTotal * pi.PageSize;

    return {
      .commitLimit = totalPagefileSize,
      .commitTotal = commitedPagefileSize,
      .pageSize = pi.PageSize,
    };
  }

  std::vector<std::string> ExtractDriveNames()
  {
    const DWORD bufferSize = GetLogicalDriveStringsA(0, nullptr);
    if (bufferSize == 0)
    {
      throw std::runtime_error("Failed to get drive strings size");
    }

    std::vector<char> buffer(bufferSize);
    if (GetLogicalDriveStringsA(bufferSize, buffer.data()) == 0)
    {
      throw std::runtime_error("Failed to get drive strings");
    }

    std::vector<std::string> driveNames;
    for (const char* pDrive = buffer.data(); *pDrive != '\0'; pDrive += strlen(pDrive) + 1)
    {
      driveNames.emplace_back(pDrive);
    }

    return driveNames;
  }

  std::optional<DriveState> ExtractDriveState(const std::string& driveName)
  {
    ULARGE_INTEGER freeBytesAvailableToCaller{};
    ULARGE_INTEGER totalNumberOfBytes{};
    ULARGE_INTEGER totalNumberOfFreeBytes{};
    if (!GetDiskFreeSpaceExA(
      driveName.c_str(),
      &freeBytesAvailableToCaller,
      &totalNumberOfBytes,
      &totalNumberOfFreeBytes))
    {
      return std::nullopt;
    }

    return DriveState{
      .driveName = driveName,
      .usageInfo = {
        .freeBytesAvailableToCaller = freeBytesAvailableToCaller,
        .totalNumberOfBytes = totalNumberOfBytes,
        .totalNumberOfFreeBytes = totalNumberOfFreeBytes,
      },
    };
  }

  std::vector<DriveState> ExtractDriveStates()
  {
    std::vector<DriveState> driveStates;
    for (const auto& driveName : ExtractDriveNames())
    {
      if (auto driveState = ExtractDriveState(driveName))
      {
        driveStates.push_back(*driveState);
      }
    }

    return driveStates;
  }

  std::string DriveStateToString(const DriveState& driveState, const Measurement measurement)
  {
    const auto mv = GetMeasurementValue(measurement);
    const auto ms = MeasurementToString(measurement);

    const auto totalSpace = driveState.usageInfo.totalNumberOfBytes.QuadPart;
    const auto availableSpace = totalSpace
      - driveState.usageInfo.totalNumberOfFreeBytes.QuadPart;

    return driveState.driveName + ": " + std::to_string(availableSpace / mv) + " " + ms + " available "
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
  std::cout << "Computer Name: " << m_osInfo.computerName << "\n";
  std::cout << "User Name: " << m_osInfo.userName << "\n";
  std::cout << "Architecture: " << m_osInfo.architecture << "\n";
  std::cout << "RAM: " << RAMInfoToString() << "\n";
  std::cout << "Virtual Memory: " << VMToString() << "\n";
  std::cout << "Memory Load: " << MemoryLoadToString() << "\n";
  std::cout << "Pagefile: " << PagefileToString() << "\n";
  std::cout << "Processors: " << m_hwInfo.processorsAmount << "\n";
  std::cout << "Drives: \n" + DriveStatesToString();
}

std::string SysInfo::RAMInfoToString() const
{
  const auto availableRAM = m_hwInfo.ramInfo.totalRAM - m_hwInfo.ramInfo.freeRAM;
  const auto mv = GetMeasurementValue(m_measurement);
  const auto ms = MeasurementToString(m_measurement);

  return std::to_string(availableRAM / mv) + " " + ms
    + " / " + std::to_string(m_hwInfo.ramInfo.totalRAM / mv) + " " + ms;
}

std::string SysInfo::VMToString() const
{
  return std::to_string(m_hwInfo.virtualMemory) + " " + MeasurementToString(m_measurement);
}

std::string SysInfo::MemoryLoadToString() const
{
  return std::to_string(m_hwInfo.memoryLoad) + "%";
}

std::string SysInfo::PagefileToString() const
{
  const auto mv = GetMeasurementValue(m_measurement);
  const auto ms = MeasurementToString(m_measurement);

  return std::to_string(m_hwInfo.pageFileInfo.commitTotal / mv) + " " + ms
    + " / " + std::to_string(m_hwInfo.pageFileInfo.commitLimit / mv) + " " + ms;
}

std::string SysInfo::DriveStatesToString() const
{
  std::string result;
  for (const auto& driveState : m_driveStates)
  {
    result += "  - " + DriveStateToString(driveState, m_measurement) + "\n";
  }

  return result;
}