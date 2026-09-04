### Задание 1 — sys-info-win — 50 баллов

Научиться работать с Windows API для получения системной информации, реализовать структурированный и надёжный C++-код, который:

- Использует RAII для управления ресурсами,
- Обрабатывает все возможные ошибки корректно,
- Даёт студенту понимание, как устроена система "под капотом".

Напишите программу sys-info-win для ОС Windows, которая бы выводила в консоль информацию о компьютере на котором она запущена:

- Версия операционной системы.
    - Используйте IsWindows10OrGreater() и аналоги из VersionHelpers.h
- Размер виртуальной и физической памяти, а также использование памяти в процентах.
- Количество ядер процессора
- Имя компьютера и имя пользователя
- Архитектура процессора (x86, x64, ARM)
- Размер файла подкачки (функция `GetPerformanceInfo`)
- Список логических дисков + их объёмы

#### Пример выводимой информации

```txt
OS: Windows 10 or Greater
Computer Name: DESKTOP-12345
User: Ivan Ivanov
Architecture: x64 (AMD64)
RAM: 6417MB / 7796MB
Virtual Memory: 16000MB
Memory Load: 47%
Pagefile: 20480MB / 32000MB

Processors: 16
Drives:
  - C:\  (NTFS): 114 GB free / 237 GB total
  - D:\  (NTFS): 80 GB free / 100 GB total
```

#### Подсказки

Начиная с Windows 8.1 Microsoft не рекомендует приложениям привязываться к версии операционной системы,
поэтому функции вроде [`GetVersionEx`](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getversionexw)
могут возвращать недостоверную информацию о версии операционной системы.

Используйте [вспомогательные функции Windows](https://learn.microsoft.com/en-us/windows/win32/sysinfo/version-helper-apis),
чтобы узнать информации о версии ОС.
Для получения актуальной информации используйте функцию [RtlGetVersion](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlgetversion?redirectedfrom=MSDN).

Узнать количество процессоров можно функцией [`GetSystemInfo`](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsysteminfo)
и [`GetNativeSystemInfo`](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getnativesysteminfo).

Узнать информацию о памяти компьютера можно функций [`GlobalMemoryStatusEx`](https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-globalmemorystatusex).

Размер файла подкачки можно получить функцией [GetPerformanceInfo](https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getperformanceinfo).

Информацию о дисках можно получить функциями `GetLogicalDriveStrings` и `GetDiskFreeSpaceEx`.
