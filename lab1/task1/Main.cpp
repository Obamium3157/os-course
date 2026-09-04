#include <iostream>
#include <versionhelpers.h>

int main()
{
  if (IsWindows7OrGreater())
  {
    std::cout << "Windows 7 or Greater" << std::endl;
  }
  else
  {
    std::cout << "ngwut" << std::endl;
  }

  std::cout << "Hello World!" << std::endl;
}