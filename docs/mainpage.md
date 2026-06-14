# cashash.c

`cashash.c` is a small C hash table library.

## Features

- String keys
- Generic `void *` values
- Separate chaining
- FNV-1a hash
- Dynamic bucket count

## Example

```c
#include <cashash.c/cashash.h>

#include <stdio.h>

int main(void) {
    cashash_t *map = cashash_create(128);

    cashash_insert(map, "hello", "world");

    printf("hello: %s\n", (char *)cashash_find(map, "hello"));

    cashash_destroy(map);

    return 0;
}
```

## Ownership
The table copies inserted keys.
The table does not copy or free inserted values.
