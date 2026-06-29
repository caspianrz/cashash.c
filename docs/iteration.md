# Iteration {#iteration}

`cashash.c` provides two ways to iterate over the contents of a hash table:

- An explicit iterator API.
- A callback-based `foreach` API.

Both work with separate-chaining and open-addressing hash tables.

## Iterator API

The iterator API gives complete control over iteration and is useful when
entries need to be processed manually.

Begin by initializing an iterator.

```c
cashash_iter_t iter;
cashash_iter_init(table, &iter);
```

Then repeatedly retrieve entries until the iterator is exhausted.

```c
cashash_iter_t iter;
cashash_pair_t pair;

cashash_iter_init(table, &iter);

while (cashash_iter_next(&iter, &pair)) {
    printf("%.*s\n",
           (int)pair.key.size,
           (const char *)pair.key.data);
}
```

The returned key and value are borrowed references to data stored inside the
hash table. They remain valid until the corresponding entry is removed or the
table is destroyed.

## Iterating Over Keys

If only the keys are needed, use `cashash_iter_next_key()`.

```c
cashash_iter_t iter;
cashash_key_datum_t key;

cashash_iter_init(table, &iter);

while (cashash_iter_next_key(&iter, &key)) {
    printf("%.*s\n",
           (int)key.size,
           (const char *)key.data);
}
```

This avoids constructing a full key-value pair when only keys are required.

## Foreach API

For simple traversal, the callback-based API is often more convenient.

```c
static bool print_pair(cashash_pair_t pair, void *user_data)
{
    (void)user_data;

    printf("%.*s\n",
           (int)pair.key.size,
           (const char *)pair.key.data);

    return true;
}

cashash_foreach(table, print_pair, NULL);
```

The callback is called once for every entry in the table.

Returning `true` continues iteration, while returning `false` stops iteration
immediately.

## Foreach Keys

To iterate over only the keys, use `cashash_foreach_key()`.

```c
static bool print_key(cashash_key_datum_t key, void *user_data)
{
    (void)user_data;

    printf("%.*s\n",
           (int)key.size,
           (const char *)key.data);

    return true;
}

cashash_foreach_key(table, print_key, NULL);
```

## Choosing an API

Use the iterator API when:

- You need explicit control over iteration.
- Iteration is part of a larger loop or algorithm.
- You want to retrieve entries one at a time.

Use the `foreach` API when:

- You simply want to visit every entry.
- A callback produces cleaner code.
- Early termination is sufficient for your use case.

## Notes

- Iteration order is unspecified and should not be relied upon.
- Every entry present in the table is visited exactly once.
- Keys and values returned during iteration are borrowed references and are not
  copied by the iterator.
