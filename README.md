# БАЗОВЫЙ УРОВЕНЬ

`strace` - показывает границу между программой и ядром: все системные вызовы (syscalls), которые делает процесс, и все сигналы, которые он получает.

`ltrace` - показывает вызовы библиотечных функций (malloc, free, strcpy, printf, pthread_create, SSL_read и т.д.) — то, что происходит внутри программы, до того как она обратится к ядру.

1. `strace`

## Примеры решения задач

1. Есть программа, которая открывает 3 файла разными способами.
Мы хотим посмотреть какие методы она выполняет после вызова функций в программе

```C
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void) {
    int fd = open("/dsds", O_RDONLY);
    if (fd >= 0) close(fd);

    fd = openat(AT_FDCWD, "/etc/os-release", O_RDONLY);
    if (fd >= 0) close(fd);

    FILE *fp = fopen("/etc/hosts", "r");
    if (fp) fclose(fp);

    return 0;
}
```

И в консоли введем команду

```bash
strace -e trace=openat ./main
//Вывод
openat(AT_FDCWD, "/etc/ld.so.cache", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/lib64/libc.so.6", O_RDONLY|O_CLOEXEC) = 3
openat(AT_FDCWD, "/dsds", O_RDONLY)     = -1 ENOENT (Нет такого файла или каталога)
openat(AT_FDCWD, "/etc/os-release", O_RDONLY) = 3
openat(AT_FDCWD, "/etc/hosts", O_RDONLY) = 3
```

Что вывела команда:
1. Первые две строки - это библиотеки смотрят КЭШ
2. Каждому успешному файлу передается файловый дескриптор 3 (первые три - это stdin, stdout, stderr)
3. Если файл не удалось открыть, он напишет причину и выдаст код -1

2. Наша программа зависает и мы хотим узнать почему

```C
#include <stdio.h>
#include <unistd.h>

int main(void) {
    printf("Введи что-нибудь и нажми Enter...\n");
    fflush(stdout);

    char buf[256];
    ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Получено: %s", buf);
    }
    return 0;
}
```
Откроем две консоли. В 1 запустим программу, а во 2 выполним команду

```bash
sudo strace -p $(pidof main) -tt -T
```
Во 2 консоли вывод будет следующим:

```bash
strace: Process 4356 attached
20:10:33.887403 read(0,
```

После ввода в 1 консоль, например, "Hey", во 2 консоли сразу появится следующая информация

```bash
strace: Process 4356 attached
20:10:33.887403 read(0, "Hey\n", 255)   = 4 <70.097927>
20:11:43.985613 write(1, "\320\237\320\276\320\273\321\203\321\207\320\265\320\275\320\276: Hey\n", 22) = 22 <0.000017>
20:11:43.986132 write(1, "\320\223\320\276\321\202\320\276\320\262\320\276.\n", 14) = 14 <0.000035>
20:11:43.986529 exit_group(0)           = ?
20:11:43.986799 +++ exited with 0 +++
```

Мы увидели не только то, что записали, но и что была записано через printf (цифры через \ - это русские слова)

3. Наша программа подвисает и мы хотим узнать сколько и какие методы занимают наше время

```C
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(void) {
    for (int i = 0; i < 100; i++) {
        printf("line %d\n", i);
    }
    for (int i = 0; i < 50; i++) {
        access("/etc/hosts", F_OK);
    }
    for (int i = 0; i < 5; i++) {
        usleep(100000); // 100 мс
    }
    return 0;
}
```
Выполним команду и увидем следующий результат

```bash
strace -c ./main
% time     seconds  usecs/call     calls    errors syscall
------ ----------- ----------- --------- --------- ----------------
 41,32    0,002979          29       101           write
 18,88    0,001361         272         5           clock_nanosleep
 17,81    0,001284         160         8           mmap
  4,73    0,000341         113         3           brk
  3,54    0,000255          85         3           fstat
  3,07    0,000221          55         4           mprotect
  2,98    0,000215           4        51         1 access
  2,30    0,000166          83         2           close
  2,26    0,000163         163         1           arch_prctl
  1,96    0,000141         141         1           read
  0,75    0,000054          27         2           openat
  0,42    0,000030          30         1           munmap
  0,00    0,000000           0         1           execve
------ ----------- ----------- --------- --------- ----------------
100,00    0,007210          39       183         1 total
```

Теперь у нас есть таблица-статиска, где видно, что больше всего времени занимает write (наш printf)

# АРХИВ

## 1 слой - Add-on tools and viewers (Дополнительные инструменты и средства просмотра)

Это инструменты, которые работают поверх базовых компонентов трассировки и решают две ключевые задачи: упрощают управление трассировкой и предоставляют наглядные способы визуализации собранных данных

## Краткий разбор утилит

### 1. **trace-cmd** - это основной инструмент командной строки для работы с ftrace. Он незаменим, когда нужно записать события ядра, а затем проанализировать их текстово.

#### Пример

Какие функции ядра вызываются при выполнении команды ls.
``` 
sudo trace-cmd record -p function_graph -e sched_switch ls > /dev/null
```

Эта команда запускает ls, записывая граф вызовов функций (function_graph) и события переключения контекста (sched_switch). Результат сохраняется в файл trace.dat

```
sudo trace-cmd report
```
Вы получите текстовый лог с временными метками, именами процессов и последовательностью вызовов функций, что позволяет детально изучить поведение ядра во время выполнения команды

<img width="943" height="273" alt="trace-cmd-1" src="https://github.com/user-attachments/assets/079fa37c-b73f-4d97-9d76-06190600fee0" />

<img width="1299" height="323" alt="trace-cmd-2" src="https://github.com/user-attachments/assets/7bb4f344-45c3-489c-9b87-ffe3ad4b5b1e" />


### 2. **KernelShark** - это графический интерфейс для данных, собранных trace-cmd. Он превращает огромный текстовый лог в интерактивную временную диаграмму.

#### Пример

После сбора данных командой trace-cmd record ..., вы можете открыть файл trace.dat в KernelShark:
```
kernelshark trace.dat
```
Вы увидите графическое представление, где по вертикали расположены ядра CPU, а по горизонтали — время. 

<img width="1156" height="546" alt="kernelshark" src="https://github.com/user-attachments/assets/7b5a0fb1-07a1-4349-b5be-81978c182f5d" />

### 3. **LTTng** - это мощный фреймворк для трассировки, который может отслеживать как ядро, так и пользовательские приложения, написанные на C/C++, Java, Python. Его главная особенность — возможность создавать единую временную шкалу событий из разных источников.

#### Пример - системная команда

Какие файлы открывает команда ls.

Сначала создаем сессию и включаем системные вызовы 

```
lttng create my-session
lttng list
lttng set-session my-session
lttng enable-channel chan0 -k
lttng enable-event -k sched_switch -c chan0
lttng status
```
<img width="595" height="590" alt="lttng-1" src="https://github.com/user-attachments/assets/834b9689-2648-4753-b0e1-e692913c5634" />

Запускаем трассировку

```
lttng start
```

После выполняем sleep останавливаем трассировку и смотрим данные, после чего удаляем ее.

```
lttng stop
lttng view | grep sleep
lttng destroy
```

<img width="1305" height="139" alt="lttng-2" src="https://github.com/user-attachments/assets/000a3f4b-d58c-456d-97d7-2024c0fc5824" />

#### Пример - пользовательская программа

У нас есть простая программа на C:
```
#include <lttng/tracef.h>
int main(void) {
    tracef("Привет из моего приложения!");
    return 0;
}
```
`tracef()` - это аналог для `printf()`, но как раз для LTTng

Компилируем код
```
gcc -o hello hello.c -llttng-ust
```
И далее как из "Пример - системная команда

```
lttng create user-session
lttng enable-event --userspace 'lttng_ust_tracef:*'
lttng start
./hello
lttng stop
lttng view
lttng destroy
```

<<Здесь будет пример в виде изображения>>

### 4. perf-tools - это коллекция скриптов, созданная Бренданом Григгом (Brendan Gregg) для упрощения анализа производительности

! Для работы необходимо:
4.1. установить `perf` 

```
sudo apt-get install linux-tools-common linux-tools-$(uname -r)
```
4.2. склонировать `perf-tools`

```
git clone --depth 1 https://github.com/brendangregg/perf-tools.git
```

4.3. сделать их исполняемыми

```
cd perf-tools
chmod +x ./*
```

Теперь можно приступать к работе


