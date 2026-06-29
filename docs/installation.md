# Installation {#installation}

`cashash.c` uses CMake as its build system and can be integrated into your project in several ways.

## Requirements

To build `cashash.c`, you will need:

* A C99 (or newer) compatible compiler
* CMake 3.16 or newer

---

## Vendoring the Repository

The recommended way to use `cashash.c` is to vendor the repository into your project.

Clone the repository into a `vendor` directory:

```sh
git clone https://github.com/caspianrz/cashash.c.git vendor/cashash.c
```

or if your project is already a git project:

```sh
git submodule add https://github.com/caspianrz/cashash.c.git vendor/cashash.c
```

Your project may look like this:

```text
your_project/
├── CMakeLists.txt
├── src/
└── vendor/
    └── cashash.c/
```

Then add the library to your `CMakeLists.txt`:

```cmake
add_subdirectory(vendor/cashash.c)

target_link_libraries(your_application PRIVATE cashash::cashash)
```

---

## Using CMake FetchContent

`cashash.c` can also be downloaded automatically during configuration using CMake's `FetchContent` module.

```cmake
include(FetchContent)

FetchContent_Declare(
    cashash
    GIT_REPOSITORY https://github.com/caspianrz/cashash.c.git
    GIT_TAG v1.0.0
)

FetchContent_MakeAvailable(cashash)

target_link_libraries(your_application PRIVATE cashash::cashash)
```

Replace `v1.0.0` with the desired release tag.

---

## Verifying the Installation

After linking against `cashash`, the following program should compile successfully.

```c
#include <cashash.c/cashash.h>

int main(void) {
  cashash_t *table = cashash_create(1);

  cashash_destroy(table);

  return 0;
}
```

If the program builds successfully, `cashash.c` has been installed correctly.

