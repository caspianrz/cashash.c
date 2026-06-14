<div align="center">

# cashash.c

**A small, embeddable generic-key hash table library for C.**

![Language](https://img.shields.io/badge/language-C-blue.svg)
![Build](https://img.shields.io/github/actions/workflow/status/caspianrz/cashash.c/ci.yml?branch=main\&label=build)
![Tests](https://img.shields.io/badge/tests-Check-success.svg)
![Docs](https://img.shields.io/badge/docs-Doxygen-informational.svg)
![License](https://img.shields.io/github/license/caspianrz/cashash.c)
![Version](https://img.shields.io/github/v/release/caspianrz/cashash.c?include_prereleases)

</div>

It provides a minimal generic-key hash table with `void *` values. Keys are treated as raw bytes using an explicit key length, so the table can be used with strings, binary buffers, integers, structs, and other fixed-size key types.

The library is designed to be portable, easy to embed, and simple to use in C projects.

## Features

* Generic byte keys
* Generic `void *` values
* String keys with explicit lengths
* Binary keys with embedded `'\0'` bytes
* Integer and struct key support
* Separate chaining for collision handling
* FNV-1a hashing
* Optional xxHash support
* Dynamic bucket growth
* Static library builds
* Cross-platform support for Linux, macOS, and Windows

## Basic Usage

```c
#include <cashash.c/cashash.h>

#include <stdio.h>

#define CKEY(key) (key), (sizeof(key) - 1)

int main(void) {
  cashash_t *map = cashash_create(128);

  if (map == NULL) {
    return 1;
  }

  cashash_insert(map, CKEY("name"), "cashash");
  cashash_insert(map, CKEY("language"), "C");

  printf("%s\n", (char *)cashash_find(map, CKEY("name")));
  printf("%s\n", (char *)cashash_find(map, CKEY("language")));

  cashash_destroy(map);

  return 0;
}
```

## Generic Keys

Unlike string-only hash tables, `cashash.c` does not depend on null-terminated keys. Every key is passed with an explicit length:

```c
bool cashash_insert(cashash_t *table, const char *key, size_t key_len, void *value);
void *cashash_find(cashash_t *table, const char *key, size_t key_len);
bool cashash_remove(cashash_t *table, void *key, size_t key_len);
```

This means the key may contain any bytes, including `'\0'`.

## Binary Key Example

```c
char key[] = {'i', 'd', '\0', '1'};

cashash_insert(map, key, sizeof(key), "value");

char *value = cashash_find(map, key, sizeof(key));
```

The full `sizeof(key)` bytes are used when hashing and comparing the key.

## Integer Key Example

```c
int id = 42;

cashash_insert(map, (const char *)&id, sizeof(id), "user-42");

char *value = cashash_find(map, (const char *)&id, sizeof(id));
```

Integer keys are copied internally as raw bytes.

## Struct Key Example

```c
typedef struct {
  unsigned int id;
  unsigned short kind;
  unsigned char flags;
  char tag[8];
} user_key_t;

user_key_t key = {0};

key.id = 100;
key.kind = 2;
key.flags = 1;

cashash_insert(map, (const char *)&key, sizeof(key), "user-data");

char *value = cashash_find(map, (const char *)&key, sizeof(key));
```

When using structs as raw byte keys, zero-initialize them first. Padding bytes are part of the raw memory representation, so uninitialized padding may cause two logically identical structs to compare differently.

## Ownership Rules

`cashash.c` copies keys internally.

Values are not copied. The hashmap only stores the `void *` value pointer passed to `cashash_insert()`.

When `cashash_clear()` or `cashash_destroy()` is called:

* copied keys are destroyed internally
* values are not freed
* the caller remains responsible for managing value memory

Example:

```c
char *value = malloc(128);

cashash_insert(map, CKEY("buffer"), value);

cashash_destroy(map);

/* value must still be freed by the caller */
free(value);
```

## Hashing

By default, `cashash.c` uses FNV-1a hashing.

The hash helper API includes:

```c
size_t cashash_hash_fnv1a_string(const char *key);
size_t cashash_hash_fnv1a_bytes(const void *data, size_t len, ...);

bool cashash_equal_fnv1a_bytes(const void *a, const void *b, size_t len);
void *cashash_copy_fnv1a_bytes(const void *key, size_t len);
void cashash_key_destroy_fnv1a_bytes(const void *key);
```

If `CASHASH_USE_XXHASH` is enabled, xxHash-based hashing functions are also available:

```c
size_t cashash_hash_xxh3_bytes(const void *data, size_t len, ...);
size_t cashash_hash_xxh64_bytes(const void *data, size_t len, ...);
```

## Dynamic Growth

The table grows dynamically when the load factor becomes high.

During growth, existing keys are rehashed using their stored key lengths, so binary keys and generic keys remain valid after resizing.

## Documentation

API documentation is generated with Doxygen and published through GitHub Pages.

The documentation should cover:

* public hashmap API
* hash helper API
* ownership rules
* generic key behavior
* examples for string, binary, integer, and struct keys

## Testing

The project uses the Check unit testing framework and is tested through GitHub Actions across multiple platforms.

The test suite covers:

* creation and destruction
* insertion and lookup
* key updates
* removal
* clearing
* collision handling
* dynamic bucket growth
* binary keys
* integer keys
* long keys
* struct keys
* hash helper functions

## Links

* [Documentation](https://caspianrz.github.io/cashash.c/)
* [Issues](https://github.com/caspianrz/cashash.c/issues)
* [Releases](https://github.com/caspianrz/cashash.c/releases)
