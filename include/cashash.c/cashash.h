#pragma once

#ifndef CASHASH_C_H
#define CASHASH_C_H

#include <cashash.c/cashash.h>
#include <cashash.c/cashash_type.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file cashash.c/cashash.h
 * @brief Public API for the cashash generic-key hash table.
 *
 * cashash is a small C hash table library using:
 *
 * - generic byte keys
 * - generic `void *` values
 * - separate-chaining and open-addressing backends
 * - FNV-1a hashing by default
 * - optional xxHash support
 * - automatic dynamic bucket growth
 *
 * Keys are passed as a pointer plus an explicit length using
 * cashash_key_datum_t. This makes the table usable with strings, binary
 * buffers, integers, structs, and other fixed-size key types.
 *
 * The table copies keys internally. The caller may free or modify the original
 * key after insertion.
 *
 * The table stores generic `void *` values as-is. It does not copy, destroy,
 * or otherwise manage inserted values.
 */

/**
 * @brief Create a new hash table using the default backend.
 *
 * This function is kept for backward compatibility with older versions of
 * cashash.c. It creates a separate-chaining hash table using the default
 * configuration.
 *
 * New code may prefer calling cashash_create_chain() or
 * cashash_create_open_addressing() explicitly to make the selected backend
 * clear.
 *
 * @param bucket_count Initial number of buckets to allocate.
 *
 * @return A pointer to a new hash table on success.
 * @return NULL if allocation fails or if `bucket_count` is 0.
 *
 * @note This function currently defaults to the separate-chaining backend.
 * @note This is equivalent to calling cashash_create_chain(bucket_count).
 */
cashash_t *cashash_create(size_t bucket_count);

/* -------------------------------------------------------------------------- */
/* Open-addressing creation API */
/* -------------------------------------------------------------------------- */

/**
 * @brief Create an open-addressing hash table using the default strategy.
 *
 * Uses FNV-1a hashing and linear probing by default.
 *
 * @param bucket_count Initial number of buckets.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_open_addressing(size_t bucket_count);

/**
 * @brief Create an open-addressing hash table using a hash and probing
 * strategy.
 *
 * @param bucket_count Initial number of buckets.
 * @param strategy Hash strategy to use.
 * @param option Strategy-specific options.
 * @param probe_strategy Open-addressing probing strategy.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_open_addressing_with_strategy(
    size_t bucket_count, cashash_hash_strategy_t strategy,
    cashash_strategy_option_t option,
    cashash_oa_probe_strategy_t probe_strategy);

/**
 * @brief Create an open-addressing hash table using a custom config.
 *
 * @param bucket_count Initial number of buckets.
 * @param config Hash table behavior configuration.
 * @param option Strategy-specific options.
 * @param probe_strategy Open-addressing probing strategy.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_open_addressing_with_config(
    size_t bucket_count, cashash_config_t config,
    cashash_strategy_option_t option,
    cashash_oa_probe_strategy_t probe_strategy);

/**
 * @brief Create an open-addressing hash table using custom double hashing.
 *
 * Creates a table using CASHASH_OA_PROBE_DOUBLE_HASHING and stores the given
 * second hash function.
 *
 * @param bucket_count Initial number of buckets.
 * @param strategy Hash strategy to use.
 * @param option Strategy-specific options.
 * @param second_hash Second hash function.
 * @param second_hash_user_data User data passed to the second hash function.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_open_addressing_with_double_hash(
    size_t bucket_count, cashash_hash_strategy_t strategy,
    cashash_strategy_option_t option, cashash_hash_fn second_hash,
    void *second_hash_user_data);

/**
 * @brief Create an open-addressing hash table using custom config and double
 * hashing.
 *
 * Creates a table using CASHASH_OA_PROBE_DOUBLE_HASHING and stores the given
 * second hash function.
 *
 * @param bucket_count Initial number of buckets.
 * @param config Hash table behavior configuration.
 * @param option Strategy-specific options.
 * @param second_hash Second hash function.
 * @param second_hash_user_data User data passed to the second hash function.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_open_addressing_with_config_and_double_hash(
    size_t bucket_count, cashash_config_t config,
    cashash_strategy_option_t option, cashash_hash_fn second_hash,
    void *second_hash_user_data);

/* -------------------------------------------------------------------------- */
/* Separate-chaining creation API */
/* -------------------------------------------------------------------------- */

/**
 * @brief Create a separate-chaining hash table using the default strategy.
 *
 * Creates a hash table using the default FNV-1a byte-key configuration.
 *
 * @param bucket_count Initial number of buckets to allocate.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_chain(size_t bucket_count);

/**
 * @brief Create a separate-chaining hash table using a built-in hash strategy.
 *
 * @param bucket_count Initial number of buckets to allocate.
 * @param strategy Built-in hash strategy to use.
 * @param option Strategy-specific options.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_chain_with_strategy(size_t bucket_count,
                                              cashash_hash_strategy_t strategy,
                                              cashash_strategy_option_t option);

/**
 * @brief Create a separate-chaining hash table using a custom configuration.
 *
 * Creates a table with caller-provided hash, equality, key copy, and key
 * destroy functions.
 *
 * @param bucket_count Initial number of buckets to allocate.
 * @param config Hash table configuration.
 * @param option Strategy-specific options.
 *
 * @return New hash table, or `NULL` on failure.
 */
cashash_t *cashash_create_chain_with_config(size_t bucket_count,
                                            cashash_config_t config,
                                            cashash_strategy_option_t option);

/* -------------------------------------------------------------------------- */
/* Common table API */
/* -------------------------------------------------------------------------- */

/**
 * @brief Insert or update a key-value pair.
 *
 * Inserts `key` with the given value pointer. If the key already exists, its
 * value pointer is replaced and the table size does not increase.
 *
 * The key is copied internally using the configured key copy function.
 *
 * @param table Hash table.
 * @param key Key datum to insert.
 * @param data Value pointer to store.
 *
 * @return `true` if insertion or update succeeds.
 * @return `false` if `table` is NULL, `key.data` is NULL, `key.length` is 0,
 * or allocation fails.
 *
 * @note The table stores the value pointer as-is. It does not copy the value.
 * @note Keys may contain embedded `'\0'` bytes.
 */
bool cashash_insert(cashash_t *table, const cashash_key_datum_t key,
                    void *data);

/**
 * @brief Find a value by key.
 *
 * Searches the table for a key that has the same bytes and length as `key`.
 *
 * @param table Hash table.
 * @param key Key datum to search for.
 *
 * @return Stored value pointer if the key is found.
 * @return `NULL` if the key is not found, or if `table` or `key.data` is NULL.
 *
 * @warning Since `NULL` is also a valid `void *` value, this function cannot
 * distinguish between "key not found" and "key found with NULL value".
 */
void *cashash_find(const cashash_t *table, const cashash_key_datum_t key);

/**
 * @brief Remove a key-value pair from the hash table.
 *
 * Searches for `key` and removes the matching entry if it exists.
 *
 * This function frees the internally copied key, but it does not free the
 * stored value pointer.
 *
 * @param table Hash table.
 * @param key Key datum to remove.
 *
 * @return `true` if the key was found and removed.
 * @return `false` if the key was not found, or if `table` or `key.data` is
 * NULL.
 *
 * @note The table does not own inserted values.
 * @note Removing a key decreases cashash_size() by 1.
 */
bool cashash_remove(cashash_t *table, const cashash_key_datum_t key);

/**
 * @brief Remove all entries from the hash table.
 *
 * Clears every key-value pair from the table while keeping the table itself
 * allocated and reusable.
 *
 * This function frees all internally copied keys, but it does not free stored
 * value pointers.
 *
 * @param table Hash table to clear. Passing NULL is allowed.
 *
 * @note The current bucket count is unchanged.
 * @note After clearing, cashash_size() returns 0.
 */
void cashash_clear(cashash_t *table);

/**
 * @brief Destroy a hash table.
 *
 * Frees all copied keys, internal storage, and the table itself.
 *
 * @param table Hash table to destroy. Passing NULL is allowed.
 *
 * @warning This function does not free stored values. The caller is responsible
 * for managing the lifetime of values inserted into the table.
 */
void cashash_destroy(cashash_t *table);

/**
 * @brief Get the number of key-value pairs stored in the hash table.
 *
 * @param table Hash table.
 *
 * @return Number of entries in the table.
 * @return 0 if `table` is NULL.
 */
size_t cashash_size(const cashash_t *table);

/**
 * @brief Get the current number of buckets/slots in the hash table.
 *
 * For separate-chaining tables, this returns the number of bucket chains.
 * For open-addressing tables, this returns the number of entry slots.
 *
 * @param table Hash table.
 *
 * @return Current bucket/slot count.
 * @return 0 if `table` is NULL.
 */
size_t cashash_bucket_count(const cashash_t *table);

#endif /* CASHASH_C_H */
