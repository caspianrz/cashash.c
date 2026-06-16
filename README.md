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

`cashash.c` is a small generic-key hash table library for C.

It stores keys as raw byte slices using `cashash_key_datum_t`, and stores values as generic `void *` pointers. This makes it usable with string keys, binary keys, integer keys, struct keys, and other fixed-size key types.

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
* Iteration using `cashash_iter_next()`
* Iteration using `cashash_foreach()`
* Static library builds
* Cross-platform support for Linux, macOS, and Windows

## Basic Usage

```c
#include <cashash.c/cashash.h>

#include <stdio.h>

#define CASHASH_KEY(data_, length_) \
  ((cashash_key_datum_t){.data = (data_), .length = (length_)})

#define CKEY(key_) CASHASH_KEY((key_), sizeof(key_) - 1)

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

## Public API

The main API uses `cashash_key_datum_t` for keys and `void *` for values:

```c
bool cashash_insert(
    cashash_t *table,
    const cashash_key_datum_t key,
    void *data
);

void *cashash_find(
    const cashash_t *table,
    const cashash_key_datum_t key
);

bool cashash_remove(
    cashash_t *table,
    const cashash_key_datum_t key
);
```

## Generic Keys

Unlike string-only hash tables, `cashash.c` does not depend on null-terminated keys.

Every key is passed as a datum:

```c
typedef struct cashash_key_datum_s {
  const void *data;
  size_t length;
} cashash_key_datum_t;
```

This means the key may contain any bytes, including `'\0'`.

## Binary Key Example

```c
char key[] = {'i', 'd', '\0', '1'};

cashash_key_datum_t datum = {
  .data = key,
  .length = sizeof key,
};

cashash_insert(map, datum, "value");

char *value = cashash_find(map, datum);
```

The full `sizeof(key)` bytes are used when hashing and comparing the key.

## Integer Key Example

```c
int id = 42;

cashash_key_datum_t key = {
  .data = &id,
  .length = sizeof id,
};

cashash_insert(map, key, "user-42");

char *value = cashash_find(map, key);
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

user_key_t user_key = {0};

user_key.id = 100;
user_key.kind = 2;
user_key.flags = 1;

cashash_key_datum_t key = {
  .data = &user_key,
  .length = sizeof user_key,
};

cashash_insert(map, key, "user-data");

char *value = cashash_find(map, key);
```

When using structs as raw byte keys, zero-initialize them first. Padding bytes are part of the raw memory representation, so uninitialized padding may cause two logically identical structs to compare differently.

## Mutable Values

Values are stored as `void *`, so the table stores the pointer you pass to `cashash_insert()`.

Example with mutable integer values:

```c
int value = 10;

cashash_insert(map, CKEY("count"), &value);

int *stored = cashash_find(map, CKEY("count"));

if (stored != NULL) {
  *stored += 5;
}

printf("%d\n", value); /* prints 15 */
```

## Iteration with `next`

Include the iterator header:

```c
#include <cashash.c/cashash_iter.h>
```

Then use `cashash_iter_init()`, `cashash_iter_has_next()`, and `cashash_iter_next()`:

```c
cashash_iter_t iter;
cashash_pair_t pair;

cashash_iter_init(map, &iter);

while (cashash_iter_has_next(&iter)) {
  if (!cashash_iter_next(&iter, &pair)) {
    break;
  }

  printf("key length: %zu\n", pair.key.length);
  printf("value pointer: %p\n", pair.value);
}
```

For integer keys and integer values:

```c
const int keys[] = {1, 2, 3};
int values[] = {10, 20, 30};

for (size_t i = 0; i < 3; i++) {
  cashash_key_datum_t key = {
    .data = &keys[i],
    .length = sizeof keys[i],
  };

  cashash_insert(map, key, &values[i]);
}

cashash_iter_t iter;
cashash_pair_t pair;

cashash_iter_init(map, &iter);

while (cashash_iter_has_next(&iter)) {
  cashash_iter_next(&iter, &pair);

  int key = *(const int *)pair.key.data;
  int value = *(int *)pair.value;

  printf("%d => %d\n", key, value);
}
```

## Iteration with `foreach`

`cashash_foreach()` calls a callback once for each key-value pair.

```c
typedef bool (*cashash_foreach_fn)(
    cashash_pair_t pair,
    void *user_data
);

bool cashash_foreach(
    cashash_t *table,
    cashash_foreach_fn callback,
    void *user_data
);
```

Example:

```c
static bool print_pair(cashash_pair_t pair, void *user_data) {
  (void)user_data;

  int key = *(const int *)pair.key.data;
  int value = *(int *)pair.value;

  printf("%d => %d\n", key, value);

  return true;
}
```

Usage:

```c
cashash_foreach(map, print_pair, NULL);
```

The callback should return `true` to continue iteration and `false` to stop early.

Example that mutates stored integer values:

```c
static bool add_ten(cashash_pair_t pair, void *user_data) {
  (void)user_data;

  int *value = pair.value;
  *value += 10;

  return true;
}

cashash_foreach(map, add_ten, NULL);
```

## Ownership Rules

`cashash.c` copies keys internally.

Values are not copied. The hash table only stores the `void *` value pointer passed to `cashash_insert()`.

When `cashash_clear()` or `cashash_destroy()` is called:

* copied keys are destroyed internally
* values are not freed
* the caller remains responsible for managing value memory

Example:

```c
char *value = malloc(128);

if (value == NULL) {
  return 1;
}

cashash_insert(map, CKEY("buffer"), value);

cashash_destroy(map);

/* value must still be freed by the caller */
free(value);
```

If you store stack values, they must remain alive for as long as the entry is used:

```c
int value = 42;

cashash_insert(map, CKEY("answer"), &value);

/* safe while value is still in scope */
int *stored = cashash_find(map, CKEY("answer"));
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

Values are stored as pointers and are preserved during growth.

## Documentation

API documentation is generated with Doxygen and published through GitHub Pages.

The documentation should cover:

* public hash table API
* iterator API
* foreach API
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
* iterator traversal
* foreach traversal
* mutable pointer values
* hash helper functions

## Links

* [Documentation](https://caspianrz.github.io/cashash.c/)
* [Issues](https://github.com/caspianrz/cashash.c/issues)
* [Releases](https://github.com/caspianrz/cashash.c/releases)
