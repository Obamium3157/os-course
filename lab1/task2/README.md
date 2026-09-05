### Задание 2 — sys-info-linux — 50 баллов

Напишите программу sys-info-linux для ОС Linux, которая бы выводила в консоль информацию о компьютере, на котором запущена:

- Версия ядра и дистрибутива
    - Используйте `uname()` для ядра
    - Используйте `lsb_release()` или чтение `/etc/os-release`
- Количество свободной и имеющейся оперативной памяти (в мегабайтах)
    - Используйте `sysinfo` или парсинг `/proc/meminfo`.
- Количество логических процессоров (`get_nprocs()`)
- Архитектура процессора
    - Используйте `uname().machine`
- Загрузка процессора (из sysinfo.loads) или `/proc/loadavg`.
- Список подключенных логических дисков
    - прочитать `/proc/mounts` или вызывать `getmntent`.
    - Получить статистику через `statvfs()`
- Информация о текущем пользователе и hostname
    - `getlogin()`, `gethostname()` или `getpwuid(getuid())`.
- Объём доступной виртуальной памяти
    - Через `/proc/meminfo`, поле `VmallocTotal` (если доступно)

#### Пример выводимой информации

```txt
OS: Ubuntu 22.04.1 LTS
Kernel: Linux 5.15.0-86-generic
Architecture: x86_64
Hostname: dev-machine-01
User: student
RAM: 5983MB free / 7796MB total
Swap: 2047MB total / 512MB free
Virtual memory: 134217 MB
Processors: 16
Load average: 0.12, 0.45, 0.91
Drives:
  /          ext4     40GB free / 100GB total
  /mnt/c     fuse     12GB free / 237GB total
```