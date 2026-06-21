#pragma once

#ifndef CASHASH_C_H
#define CASHASH_C_H

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
 * - separate chaining
 * - FNV-1a hashing by default
 * - optional xxHash support
 * - automatic dynamic bucket growth
 *
 * Keys are passed as a pointer plus an explicit length. This makes the table
 * usable with strings, binary buffers, integers, structs, and other fixed-size
 * key types.
 *
 * The table copies keys internally. The caller may free or modify the original
 * key after insertion.
 *
 * The table stores generic `void *` values as-is. It does not copy, destroy,
 * or otherwise manage inserted values.
 *
 * As new key-value pairs are inserted, the table may allocate a larger bucket
 * array and rehash existing entries to keep collision chains shorter.
 */

/**
 * @brief Create a new hash table using the default strategy.
 *
 * Creates a hash table using the default FNV-1a byte-key configuration.
 *
 * @param bucket_count Initial number of buckets to allocate.
 *
 * @return A pointer to a new hash table on success.
 * @return NULL if allocation fails or if `bucket_count` is 0.
 *
 * @note The table may grow dynamically as entries are inserted.
 * @note A larger initial bucket count usually reduces early collisions but
 * uses more memory.
 * @note A bucket count of 1 is valid, but all initial keys collide into one
 * chain until growth occurs.
 */
cashash_t *cashash_create_chain(size_t bucket_count);

/**
 * @brief Create a new hash table using a built-in hash strategy.
 *
 * Creates a hash table using one of the built-in hash strategies.
 *
 * @param bucket_count Initial number of buckets to allocate.
 * @param strategy Built-in hash strategy to use.
 * @param option Strategy-specific options.
 *
 * @return A pointer to a new hash table on success.
 * @return NULL if allocation fails, if `bucket_count` is 0, or if `strategy`
 * is invalid.
 */
cashash_t *cashash_create_chain_with_strategy(size_t bucket_count,
                                              cashash_hash_strategy_t strategy,
                                              cashash_strategy_option_t option);

/**
 * @brief Create a new hash table using a custom configuration.
 *
 * Creates a hash table with caller-provided hash, equality, key copy, and key
 * destroy functions.
 *
 * @param bucket_count Initial number of buckets to allocate.
 * @param config Hash table configuration.
 * @param option Strategy-specific options.
 *
 * @return A pointer to a new hash table on success.
 * @return NULL if allocation fails, if `bucket_count` is 0, or if any required
 * callback in `config` is NULL.
 *
 * @warning Custom callbacks must be compatible with each other. The hash and
 * equality functions should operate on the same key representation.
 */
cashash_t *cashash_create_chain_with_config(size_t bucket_count,
                                            cashash_config_t config,
                                            cashash_strategy_option_t option);

/**
 * @brief Insert or update a key-value pair.
 *
 * Inserts `key` with the given `value`. If the key already exists, its value
 * is replaced and the table size does not increase.
 *
 * The key is copied internally using the configured key copy function.
 *
 * @param table Hash table.
 * @param key datum to insert.
 * @param value pointer to insert.
 *
 * @return true if insertion or update succeeds.
 * @return false if `table` is NULL, `key` is NULL, or allocation fails.
 *
 * @note The table stores the value pointer as-is. It does not copy the value.
 * @note Keys may contain embedded `'\0'` bytes.
 */
bool cashash_insert(cashash_t *table, const cashash_key_datum_t key,
                    void *data);

/**
 * @brief Find a value by key.
 *
 * Searches the table for a key matching exactly `key_len` bytes from `key`.
 *
 * @param table Hash table.
 * @param key datum contains key and it's size.
 * @param value pointer which fills datum with value and it's size if found.
 *
 * @return value pointer if key is found.
 * @return NULL if key is not found, or if `table` or `key` is NULL.
 *
 * @warning Since NULL is also a valid `void *` value, this function cannot
 * distinguish between "key not found" and "key found with NULL value".
 */
void *cashash_find(const cashash_t *table, const cashash_key_datum_t key);

/**
 * @brief Remove a key-value pair from the hash table.
 *
 * Searches for `key` and removes the matching entry if it exists.
 *
 * This function frees the internally copied key and the internal table node,
 * but it does not free the stored value pointer.
 *
 * @param table Hash table.
 * @param key datum for removal.
 *
 * @return true if the key was found and removed.
 * @return false if the key was not found, or if `table` or `key` is NULL.
 *
 * @note The table does not own inserted values, so the caller is responsible
 * for freeing or otherwise managing the value if needed.
 * @note Removing a key decreases cashash_size() by 1.
 */
bool cashash_remove(cashash_t *table, const cashash_key_datum_t key);

/**
 * @brief Remove all entries from the hash table.
 *
 * Clears every key-value pair from the table while keeping the table itself
 * allocated and reusable.
 *
 * This function frees all internally copied keys and internal table nodes,
 * but it does not free any stored value pointers.
 *
 * @param table Hash table to clear. Passing NULL is allowed.
 *
 * @note The bucket array remains allocated.
 * @note The current bucket count is unchanged.
 * @note After clearing, cashash_size() returns 0.
 * @note New entries may be inserted into the table after calling this function.
 */
void cashash_clear(cashash_t *table);

/**
 * @brief Destroy a hash table.
 *
 * Frees all internal nodes, copied keys, bucket storage, and the table itself.
 *
 * @param table Hash table to destroy. Passing NULL is allowed.
 *
 * @warning This function does not free stored values. The caller is responsible
 * for managing the lifetime of values inserted into the table.
 */
void cashash_destroy(cashash_t *table);

/**
 * @}
 */

#endif
