# Diagnostics and Validation {#diagnostics}

`cashash.c` includes a small diagnostics API that can be used to verify the
internal consistency of a hash table and collect runtime statistics.

These functions are primarily intended for debugging, testing, benchmarking,
and performance analysis.

## Validating a Table

Use `cashash_validate()` to verify that a hash table is internally consistent.

```c
cashash_validate_result_t result;

if (cashash_validate(table, &result)) {
    printf("Table is valid.\n");
} else {
    printf("Validation failed.\n");
    printf("Error: %d\n", result.error);
    printf("Bucket: %zu\n", result.bucket_index);
    printf("Entry: %zu\n", result.entry_index);
}
```

Validation checks the internal structure of the table and reports the first
consistency error that is encountered.

Depending on the table implementation, validation may detect:

- Missing or invalid internal storage.
- Incorrect entry counts.
- Invalid entries.
- Cached hash mismatches.
- Entries stored in incorrect buckets.
- Cycles in bucket chains.

## Validation Result

When a validation error is found, `cashash_validate_result_t` provides
additional information.

- `valid` indicates whether validation succeeded.
- `error` identifies the type of validation failure.
- `bucket_index` identifies the bucket or slot where the error occurred.
- `entry_index` provides additional location information within that bucket or
  table.

Passing `NULL` as the result parameter is allowed if only a boolean success or
failure is needed.

```c
if (!cashash_validate(table, NULL)) {
    fprintf(stderr, "Hash table is corrupted.\n");
}
```

## Collecting Statistics

Use `cashash_stats()` to collect information about the current state of a hash
table.

```c
cashash_stats_t stats;

if (cashash_stats(table, &stats)) {
    printf("Entries: %zu\n", stats.entries);
    printf("Capacity: %zu\n", stats.capacity);
    printf("Load Factor: %.2f\n", stats.load_factor);
}
```

The statistics include information such as:

- Number of entries.
- Table capacity.
- Load factor.
- Collision count.
- Used and empty buckets.
- Longest chain (separate chaining).
- Tombstones (open addressing).
- Probe length metrics (open addressing).

Some fields are specific to separate chaining or open addressing. Fields that
are not applicable are typically zero.

## Printing Statistics

To print a human-readable summary, use `cashash_stats_print()`.

```c
cashash_stats_t stats;

if (cashash_stats(table, &stats)) {
    cashash_stats_print(stdout, &stats);
}
```

Typical output looks like:

```text
Entries: 1204
Capacity: 2048
Load Factor: 0.59
Collisions: 331
Used Buckets: 914
Longest Chain: 5
```

## Common Uses

The diagnostics API is useful for:

- Verifying correctness during development.
- Detecting internal corruption while debugging.
- Measuring the effectiveness of different hash strategies.
- Comparing separate chaining and open addressing implementations.
- Monitoring collision rates and load factors during benchmarks.

## Notes

- Validation is intended as a debugging tool and is not typically required in
  production code.
- Statistics represent a snapshot of the table at the time they are collected.
- Validation stops after the first detected error, allowing problems to be
  identified quickly.
