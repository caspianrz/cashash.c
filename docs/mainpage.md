# cashash.c {#mainpage}

Small C string-key hash table library.

[Download](@ref download) · [API Reference](annotated.html)

## Why cashash.c?

`cashash.c` is a small C hash table library focused on simple usage,
stable APIs, and multiple collision strategies.

## Features

- Separate chaining by default
- Open addressing support
- Linear probing
- Quadratic probing
- Double hashing
- Iterator API
- Diagnostics: validation and statistics

## Quick example

```c
#include <cashash.c/cashash.h>

int main(void) {
    cashash_t *table = cashash_create(128);

    const char *key = "hello";
    const char *value = "world";

    cashash_insert(
        table,
        CASHASH_KEY_STR(key),
        CASHASH_VALUE_PTR((void *)value)
    );

    cashash_destroy(table);
}
```
