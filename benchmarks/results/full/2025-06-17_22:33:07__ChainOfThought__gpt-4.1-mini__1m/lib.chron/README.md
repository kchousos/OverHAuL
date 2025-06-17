# lib.chron

Schedulers, timers, and time-contingent eventing for the C programming language

## Library Features

- Simple, fully programmable interfaces for
  - single-use aka 'one shot' timers
  - periodic timers
  - exponential timers
- rescheduling
- cancellation
- consistent exception-handling
- Hierarchical Timer Wheels

##  Install

Via [clib](https://github.com/clibs/clib/):

```bash
clib install MatthewZito/lib.chron
```

From Source:
```bash
git clone https://github.com/MatthewZito/lib.chron
cd lib.chron && make
```

## Hierarchical Timer Wheel

The Timer Wheel implementation this library offers is implemented as a ring buffer data structure with numbered slots. Each slot contains a pointer to a linked list of elements, each sub-slots for scheduled events.

When scheduling an event *k*, we assign an integer value *r* where *r* is the number of full revolutions that must occur before event *k* is invoked.

The Timer Wheel is further optimized in that the linked list must store events such that *r* is ascending; if on revolution 3 we iterate the linked list, invoking any events whose *r* value is 3 or less, we can stop iterating the moment we scan a list node whose *r* value is greater than 3.

In this way, we never *search* the linked list; in traversing each slot on the wheel's internal ring buffer, we maintain a *time complexity of 0(1)*.

## Dynamic Linking

Linking to `lib.chron`:

```bash
# 1) include and use lib.chron in your project
# 2) generate object file for your project
gcc -I ../path/to/libchron -c main.c -o main.o
# 3) generate shared object file
make
# 4) link your project to lib.chron
gcc -o main main.o -L../path/to/libchron -llibchron
# you may need to add the lib location to your PATH
```

Linking to `lib.chron` on Windows:

```bash
# 1) include and use lib.chron in your project
# 2) generate object file for your project
gcc -I ../path/to/libchron -c main.c -o main.o
# 3) generate shared object file
make win
# 3) link your project to lib.chron
gcc -o main.exe main.o -L /path/to/lib.chron -llib_chron.dll
# you may need to add the lib location to your PATH
```
