// #include "SysInfoWin.h"
//
// #include <iomanip>
// #include <iostream>
// #include <ostream>
// #include <vector>
//
// #include <windows.h>
// #include <lmcons.h>
// #include <versionhelpers.h>
// #include <psapi.h>
//
// namespace
// {
//   std::string ProcessorArchitectureToString(const WORD architecture)
//   {
//     switch (architecture)
//     {
//       case PROCESSOR_ARCHITECTURE_AMD64:
//         return "x64 (AMD or Intel)";
//       case PROCESSOR_ARCHITECTURE_ARM:
//         return "ARM";
//       case PROCESSOR_ARCHITECTURE_ARM64:
//         return "ARM64";
//       case PROCESSOR_ARCHITECTURE_IA64:
//         return "Intel Itanium-based";
//       case PROCESSOR_ARCHITECTURE_INTEL:
//         return "x86";
//       case PROCESSOR_ARCHITECTURE_UNKNOWN:
//       default:
//         return "Unknown";
//     }
//   }
//
//   DWORDLONG GetPrecisionValue(const Measurement p)
//   {
//     return static_cast<DWORDLONG>(p);
//   }
//
//   std::string PrecisionToString(const Measurement precision)
//   {
//     switch (precision)
//     {
//       case Measurement::KB: return "KB";
//       case Measurement::MB: return "MB";
//       case Measurement::GB: return "GB";
//     }
//
//     return "";
//   }
//
//   DWORD GetBufferSize()
//   {
//     const DWORD bufferSize = GetLogicalDriveStringsW(0, nullptr);
//     if (bufferSize == 0)
//     {
//       throw std::runtime_error("Failed to get drive strings size. Error: " + std::to_string(GetLastError()));
//     }
//
//     return bufferSize;
//   }
//
//   struct DiskUsageInfo
//   {
//     ULARGE_INTEGER freeBytesAvailableToCaller;
//     ULARGE_INTEGER totalNumberOfBytes;
//     ULARGE_INTEGER totalNumberOfFreeBytes;
//   };
//
//   DiskUsageInfo GetDiskUsageInfo(const char* diskName)
//   {
//     DiskUsageInfo dui{};
//     const bool result = GetDiskFreeSpaceExA(
//       diskName,
//       &dui.freeBytesAvailableToCaller,
//       &dui.totalNumberOfBytes,
//       &dui.totalNumberOfFreeBytes);
//
//     if (!result)
//     {
//       throw std::runtime_error("Cannot retrieve disk info");
//     }
//
//     return dui;
//   }
// }
//
// void PrintSystemVersion()
// {
//   std::cout << "OS: ";
//   if (IsWindowsServer())
//   {
//     std::cout << "Windows Server\n";
//   }
//   else if (IsWindows10OrGreater())
//   {
//     std::cout << "Windows 10 or Greater\n";
//   }
//   else if (IsWindows8OrGreater() || IsWindows8Point1OrGreater())
//   {
//     std::cout << "Windows 8 or Greater\n";
//   }
//   else if (IsWindows7OrGreater() || IsWindows7SP1OrGreater())
//   {
//     std::cout << "Windows 7 or Greater\n";
//   }
//   else if (IsWindowsVistaOrGreater() || IsWindowsVistaSP1OrGreater() || IsWindowsVistaSP2OrGreater())
//   {
//     std::cout << "Windows Vista or Greater\n";
//   }
//   else if (IsWindowsXPOrGreater()
//         || IsWindowsXPSP1OrGreater()
//         || IsWindowsXPSP2OrGreater()
//         || IsWindowsXPSP3OrGreater())
//   {
//     std::cout << "Windows XP or Greater\n";
//   }
//   else
//   {
//     std::cout << "Unknown Windows version\n";
//   }
// }
//
// void PrintComputerName()
// {
//   char computerName[CNLEN + 1];
//   DWORD size = sizeof(computerName);
//   if (GetComputerNameA(computerName, &size))
//   {
//     std::cout << "Computer Name: " << computerName << std::endl;
//   } else
//   {
//     std::cout << "GetComputerName failed" << std::endl;
//   }
// }
//
// void PrintUserName()
// {
//   char userName[UNLEN + 1];
//   DWORD size = sizeof(userName);
//   if (GetUserNameA(userName, &size))
//   {
//     std::cout << "User Name: " << userName << std::endl;
//   } else
//   {
//     std::cout << "GetUserName failed" << std::endl;
//   }
// }
//
// void PrintArchitecture()
// {
//   SYSTEM_INFO info;
//   GetSystemInfo(&info);
//
//   std::cout << "Architecture: " << ProcessorArchitectureToString(info.wProcessorArchitecture) << "\n";
// }
//
// void PrintRAM(const Measurement measurement)
// {
//   MEMORYSTATUSEX statex;
//   statex.dwLength = sizeof(statex);
//   GlobalMemoryStatusEx(&statex);
//
//   const auto div = GetPrecisionValue(measurement);
//   const double totalRam = static_cast<double>(statex.ullTotalPhys) / static_cast<double>(div);
//   const double freeRam = static_cast<double>(statex.ullAvailPhys) / static_cast<double>(div);
//
//   std::cout << std::fixed << std::setprecision(2);
//   std::cout << "RAM: " << totalRam - freeRam << " " << PrecisionToString(measurement) << " / "
//             << totalRam << " " << PrecisionToString(measurement) << "\n";
// }
//
// void PrintVirtualMemory(const Measurement measurement)
// {
//   PERFORMANCE_INFORMATION pi;
//   pi.cb = sizeof(pi);
//   std::cout << "Virtual Memory: ";
//
//   if (!GetPerformanceInfo(&pi, sizeof(pi)))
//   {
//     std::cout << "unavailable\n";
//   }
//
//   const double totalVRAM = static_cast<double>(pi.CommitLimit * pi.PageSize) /
//     static_cast<double>(GetPrecisionValue(measurement));
//
//   std::cout << std::fixed << std::setprecision(2);
//   std::cout << totalVRAM << " " << PrecisionToString(measurement) << "\n";
// }
//
// void PrintMemoryLoad()
// {
//   MEMORYSTATUSEX statex;
//   statex.dwLength = sizeof(statex);
//   std::cout << "Memory Load: ";
//   if (!GlobalMemoryStatusEx(&statex))
//   {
//     std::cout << "unavailable\n";
//     return;
//   }
//   std::cout << statex.dwMemoryLoad << "%\n";
// }
//
// void PrintPageFile(const Measurement measurement)
// {
//   PERFORMANCE_INFORMATION pi;
//   pi.cb = sizeof(pi);
//   std::cout << "Pagefile: ";
//   if (!GetPerformanceInfo(&pi, sizeof(pi)))
//   {
//     std::cout << "unavailable\n";
//   }
//
//   const auto div = GetPrecisionValue(measurement);
//   const auto totalPagefileSize = pi.CommitLimit * pi.PageSize;
//   const auto commitedPagefileSize = pi.CommitTotal * pi.PageSize;
//
//   std::cout << commitedPagefileSize / div << PrecisionToString(measurement) << " / "
//     << totalPagefileSize / div << PrecisionToString(measurement) << "\n";
// }
//
//
// void PrintProcessorsAmount()
// {
//   SYSTEM_INFO si;
//   GetSystemInfo(&si);
//
//   std::cout << "Processors: " << si.dwNumberOfProcessors << "\n";
// }
//
// void PrintDrives(const Measurement measurement)
// {
//   std::cout << "Drives: \n";
//
//   const auto bufferSize = GetBufferSize();
//   std::vector<char> buffer(bufferSize);
//
//   if (const auto result = GetLogicalDriveStringsA(bufferSize, buffer.data()); result == 0)
//   {
//     throw std::runtime_error("Failed to get drive strings size. Error: " + std::to_string(GetLastError()));
//   }
//
//   const char* driveName = buffer.data();
//   const auto div = GetPrecisionValue(measurement);
//
//   while (*driveName != '\0')
//   {
//     std::cout << "  - " << driveName << ": ";
//     const auto dui = GetDiskUsageInfo(driveName);
//
//     const auto totalDriveSize = dui.totalNumberOfBytes.QuadPart;
//     const auto freeDrivePart = dui.freeBytesAvailableToCaller.QuadPart;
//
//     std::cout << std::fixed << std::setprecision(2);
//     std::cout << (totalDriveSize - freeDrivePart) / div << PrecisionToString(measurement) << " free / "
//               << totalDriveSize / div << PrecisionToString(measurement) << " total \n";
//
//     driveName += strlen(driveName) + 1;
//   }
// }
