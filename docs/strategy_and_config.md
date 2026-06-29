# Hash Strategies {#hash_strategies}

`cashash.c` provides several built-in hashing algorithms that can be selected
when creating a hash table. This makes it easy to compare different hashing
strategies without writing custom hash functions.

## Creating a Table

Choose the desired hash strategy when creating the table.

```c
cashash_t *table = cashash_create_chain_with_strategy(
    1024,
    CASHASH_HASH_STRATEGY_XXH64,
    CASHASH_STRATEGY_OPTION_DEFAULT);
```

The first argument specifies the number of buckets, the second selects the
hashing algorithm, and the third provides strategy-specific options.

## Available Strategies

| Strategy | Description |
|----------|-------------|
| `CASHASH_HASH_STRATEGY_FNV1A` | Built-in FNV-1a hash function. |
| `CASHASH_HASH_STRATEGY_XXH3` | Built-in XXH3 hash function (requires `CASHASH_USE_XXHASH`). |
| `CASHASH_HASH_STRATEGY_XXH64` | Built-in XXH64 hash function (requires `CASHASH_USE_XXHASH`). |
| `CASHASH_HASH_STRATEGY_NONE` | No built-in hash function. Intended for custom configurations. |

## Example

```c
#define DATUM(value)                                                           \
  ((cashash_key_datum_t){.data = (value), .length = sizeof(value) - 1})

int main() {
  cashash_t *table = cashash_create_chain_with_strategy(
      1024, CASHASH_HASH_STRATEGY_XXH3,
      (cashash_strategy_option_t){.used = false});

  cashash_insert(table, DATUM("apple"), &(int){42});

  int *value = cashash_find(table, DATUM("apple"));
  printf("Value is %d\n", *value);

  cashash_stats_t stats;

  cashash_stats(table, &stats);
  cashash_stats_print(stdout, &stats);

  cashash_destroy(table);
}
```

`CASHASH_HASH_STRATEGY_XXH3` is a good general-purpose choice when
`CAS_USE_XXHASH` is enabled, offering excellent speed and high-quality
hash distribution.

# Custom Strategy With Config {#custom_functions}

One of the design goals of **cashash.c** is flexibility. Nearly every
internal operation can be customized by providing user-defined functions.
This allows the library to integrate with existing codebases, memory
allocators, hashing algorithms, and comparison routines without modifying
the library itself.

## Configuration

Custom functions are supplied through the `cashash_config_t` structure before
creating a hash table.

```c
/* Assign custom callbacks */
cashash_config_t config = {
    .strategy = CASHASH_HASH_STRATEGY_NONE,
    .hash = my_hash,
    .equal = my_equal,
    .copy_key = my_copy,
    .destroy_key = my_destroy
};

size_t bucket_count = 64;
cashash_strategy_option_t option = {
    .used = false,
};
cashash_t *table = cashash_create_chain_with_config(bucket_count, config, option);
```

<!--TODO: not so sure about this but implement if it doesn't exist in source -->
Only the callbacks you replace are affected. Any callback left as `NULL`
uses the library's default implementation.

## Hash Function

The hash function determines the bucket used for a key.

```c
size_t my_hash(const void * key, const size_t len, ...)
{
    /* Your hashing algorithm */
}
```

A good hash function should:

- Produce consistent output for identical keys.
- Distribute values uniformly.
- Execute quickly.

## Key Comparison

The comparison function determines whether two keys are equal.

```c
bool my_equal(cashash_key_datum_t a, cashash_key_datum_t b)
{
    return a.length == b.length &&
           memcmp(a.data, b.data, a.length) == 0;
}
```

The comparison function must agree with the hash function. If two keys compare
equal, they must always produce the same hash value.

## Memory Allocation

Applications that already use a custom allocator can replace the default
memory functions.

```c
void * my_copy(const void * key, size_t len)
{
    void * copy = malloc(len);
    memcpy(copy, key, len);
    return copy;
}
```

```c
void my_destroy(void * key)
{
    free(key);
}
```

This is useful when integrating with:

- Arena allocators
- Memory pools
- Debug allocators
- Game engines
- Embedded systems

## Recommendations

- Keep hash functions deterministic.
- Ensure the comparison function matches the hash function.
- Allocation and deallocation functions must always be compatible.
- Avoid expensive comparison functions whenever possible.
- Test custom hash functions with realistic data sets to verify good key
  distribution.
