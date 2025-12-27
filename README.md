# agenda.h
**A GENeric Dynamic Array** implementation in C

This is a header only,
[stb](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt) style,
library that provides an implementation of generic dynamic array in C.  It
provides a set of functions and macros to create, manage and destroy the dynamic
array.

The creation of the dynamic array, via the `agenda_init()` function, returns the
pointer `items` (as show below), and this lets the user access the items the
allocated array just as they were in a static array, by the subscript operator
`[]`.  The necessary metadata is stored inside the `Agenda_Header`, hidden
behind the first element of the array, like this:

```C
// +---------------+---------+---------+---------+
// | Agenda_Header |   [0]   |   [1]   |   ...   |
// +---------------+---------+---------+---------+
//                 ^
//                 items
```

## Small example
Here i provide a small example of creation, simple usage and destroy of the
dynamic array for quick reference.  A more detailed example is provided inside
the source code, under the `example` directory.
```C
#include <stdio.h>
#define AGENDA_IMPLEMENTATION
#define AGENDA_HARDEN_WITH_MAGIC
#include <agenda.h>

int main(void) {
    int *items = agenda_init(int);

    for (int i = 0; i < 100; i++) {
        int x = i * 2;
        agenda_push_back(&items, &x);
    }

    for (int i = 0; i < agenda_items_count(items); i++) {
        printf("%d\n", agenda[i]);
    }

    agenda_deinit(&items);
    return 0;
}
```

## Controlling the behavior with macros
The following macros will change the behavior of the library if defined before
the inclusion of the `agenda.h` file:
- `AGENDA_IMPLEMENTATION`: this flag is used to decide in which translation unit
  will be placed the implementation of the functions that the library relies on.
  Not more than one implementation should be present in one project, as this
  will induce a compilation error of duplicate implementation

- `AGENDA_SHORT_PREFIX`: this flag is used to unlock short names for the
  functions.  This can be done in different translation units.

- `AGENDA_HARDEN_WITH_MAGIC`: this will harden the implementation by adding an
  additional member to the header that will contain a magic number that will be
  checked when interacting with the functions

## Platform Support
The library works on the following platforms:
- Linux, via wsl, compiled with `gcc` and checked with `valgrind`
- Windows, compiled with `gcc` via [msys2](https://www.msys2.org/), works with
  both **ucrt64** and **mingw64** environments

## Inspirations
This library takes inspiration from the following implementations of dynamic
array:
- [tsoding/nob.h](https://github.com/tsoding/nob.h)
- [gallexi/darray](https://github.com/gallexi/darray)

## Possible future plans
- `agenda_reserve_back(items, number_of_items)`
- Agenda submodules:
  - stack
  - string builder
    - `agenda_sb_push_back_str(da, item)`
