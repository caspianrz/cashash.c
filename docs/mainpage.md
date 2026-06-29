[![](https://img.shields.io/badge/Download-blue)](@ref download)
[![](https://img.shields.io/badge/API%20Reference-red)](annotated.html)

# Home {#mainpage}
`cashash.c` is a lightweight hash table library designed for C applications
that require a simple API, stable behavior, and multiple collision
resolution strategies. It is suitable for both small utilities and larger
projects where predictable and portable code is important.

## Features

* Lightweight and portable
* Separate chaining (default)
* Generic keys
* Dynamic bucket growth
* Open addressing support
    * Linear probing
    * Quadratic probing
    * Double hashing
* Iterator API
* Diagnostics, including validation and statistics

## **cashash.c** Philosophy
The philosophy behind `cashash.c` is that no single hashing strategy is the
best solution for every application. Instead of committing users to one
implementation, the library provides multiple collision resolution strategies
through a consistent API, making it straightforward to experiment, benchmark,
and compare different approaches. This allows you to choose the strategy that
offers the best balance of performance and memory usage for your particular
workload.

There are many hash table libraries for C, each designed with different
goals in mind. `cashash.c` focuses on providing a clean, consistent API
without sacrificing flexibility or performance.

Rather than exposing a single implementation, `cashash.c` supports multiple
collision resolution strategies, allowing you to choose the approach that
best fits your application's requirements. Whether you prefer separate
chaining or one of several open addressing techniques, the API remains
consistent across all implementations.

The library is written in standard C and, by default, has no external
dependencies. Optional libraries can be enabled to extend its functionality.
It is designed to integrate easily into existing projects. A strong emphasis
is placed on readable code, predictable behavior, and long-term API
stability, making it suitable for both small utilities and larger
applications.

`cashash.c` also includes an iterator API and optional diagnostic utilities,
such as table validation and runtime statistics, making it easier to inspect,
debug, and optimize your hash tables during development.

## Ownership
`cashash.c` copies every key inserted into the hash table, allowing the
original key data to be modified or released after insertion without
affecting the table.

Values are stored as pointers and are **not** copied or managed by the
library. The object pointed to by a value remains owned by the application,
which is responsible for ensuring that it remains valid for as long as it is
stored in the hash table and for releasing any associated resources when they
are no longer needed.

Destroying a hash table releases all memory allocated internally by
`cashash.c`, including copied keys and internal data structures. It does not
free the objects referenced by stored value pointers.


## Quick Example

```c
#include <stdio.h>

#include <cashash.c/cashash.h>

int main(void) {
  cashash_t *table = cashash_create(128);

  cashash_key_datum_t key = {"language", sizeof("language") - 1};
  const char *value = "C";

  /* Insert a key-value pair. */
  cashash_insert(table, key, (void *)value);

  /* Find the value associated with the key. */
  void *result = cashash_find(table, key);

  if (result != NULL) {
    printf("%s\n", (const char *)result);
  }

  /* Remove the entry from the table. */
  cashash_remove(table, key);

  /* Release the hash table. */
  cashash_destroy(table);

  return 0;
}
```
