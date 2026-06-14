#pragma once

#ifndef CASHASH_C_H
#define CASHASH_C_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file cashash.c/cashash.h
 * @brief Public API for the cashash string-key hash table.
 *
 * cashash is a small C hash table library using:
 *
 * - string keys
 * - generic `void *` values
 * - separate chaining
 * - FNV-1a hashing
 * - automatic dynamic growth
 *
 * The table stores generic `void *` values. It owns and frees copied keys,
 * but it does not own or free inserted values.
 *
 * The table starts with the bucket count passed to cashash_create(). As new
 * key-value pairs are inserted, the table may allocate a larger bucket array
 * and rehash existing entries to keep collision chains shorter.
 */

/**
 * @defgroup cashash Cashash
 * @brief String-key hash table API.
 *
 * @{
 */

/**
 * @brief Opaque hash table type.
 *
 * The internal representation is hidden from users of the library.
 * Create a table with cashash_create() and release it with cashash_destroy().
 */
typedef struct cashash_s cashash_t;

typedef enum {
  CASHASH_HASH_STRATEGY_NONE,
  CASHASH_HASH_STRATEGY_FNV1A,
#ifdef CASHASH_USE_XXHASH
  CASHASH_HASH_STRATEGY_XXH3,
  CASHASH_HASH_STRATEGY_XXH64,
#endif
} cashash_hash_strategy_t;

typedef size_t (*cashash_hash_fn)(const void *key, const size_t len, ...);
typedef bool (*cashash_equal_fn)(const void *a, const void *b, size_t len);
typedef void *(*cashash_key_copy_fn)(const void *key, size_t len);
typedef void (*cashash_key_destroy_fn)(const void *key);

typedef struct {
  cashash_hash_strategy_t strategy;
  cashash_hash_fn hash;
  cashash_equal_fn equal;
  cashash_key_copy_fn copy_key;
  cashash_key_destroy_fn destroy_key;
} cashash_config_t;

typedef union {
  bool used;
#ifdef CASHASH_USE_XXHASH
  struct {
    uint64_t seed;
  } xxh64;
#endif
} cashash_strategy_option_t;

/**
 * @brief Create a new hash table.
 *
 * Creates a hash table with a fixed number of buckets.
 * Since V1 does not support resizing, the bucket count affects performance.
 *
 * @param bucket_count Number of buckets to allocate.
 *
 * @return A pointer to a new hash table on success.
 * @return NULL if allocation fails or if `bucket_count` is 0.
 *
 * @note A larger bucket count usually reduces collisions but uses more memory.
 * @note A bucket count of 1 is valid, but all keys will collide into one chain.
 */
cashash_t *cashash_create(size_t bucket_count);

cashash_t *cashash_create_with_strategy(size_t bucket_count,
                                        cashash_hash_strategy_t strategy,
                                        cashash_strategy_option_t option);

cashash_t *cashash_create_with_config(size_t bucket_count,
                                      cashash_config_t config,
                                      cashash_strategy_option_t option);

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
 * @brief Insert or update a key-value pair.
 *
 * Inserts `key` with the given `value`. If the key already exists, its value
 * is replaced and the table size does not increase.
 *
 * The key string is copied internally, so the caller may free or modify the
 * original key after insertion.
 *
 * @param table Hash table.
 * @param key Null-terminated string key.
 * @param value Value pointer to associate with the key.
 *
 * @return true if insertion or update succeeds.
 * @return false if `table` is NULL, `key` is NULL, or allocation fails.
 *
 * @note The table stores the value pointer as-is. It does not copy the value.
 */
bool cashash_insert(cashash_t *table, const void *key, const size_t key_len,
                    void *value);

/**
 * @brief Find a value by key.
 *
 * Searches the table for `key`.
 *
 * @param table Hash table.
 * @param key Null-terminated string key.
 *
 * @return The value associated with `key`.
 * @return NULL if the key is not found, or if `table` or `key` is NULL.
 *
 * @warning Since NULL is also a valid `void *` value, this function cannot
 * distinguish between "key not found" and "key found with NULL value".
 */
void *cashash_find(const cashash_t *table, const void *key,
                   const size_t key_len);

/**
 * @brief Get the number of stored key-value pairs.
 *
 * @param table Hash table.
 *
 * @return Number of key-value pairs in the table.
 * @return 0 if `table` is NULL.
 */
size_t cashash_size(const cashash_t *table);

/**
 * @brief Get the current bucket count.
 *
 * @param table Hash table.
 *
 * @return Current number of buckets.
 * @return 0 if `table` is NULL.
 *
 * @note The bucket count may increase automatically when dynamic growth occurs.
 */
size_t cashash_bucket_count(const cashash_t *table);

/**
 * @brief Remove a key-value pair from the hash table.
 *
 * Searches for `key` and removes the matching entry if it exists.
 *
 * This function frees the internally copied key and the internal table node,
 * but it does not free the stored value pointer.
 *
 * @param table Hash table.
 * @param key Null-terminated string key to remove.
 *
 * @return true if the key was found and removed.
 * @return false if the key was not found, or if `table` or `key` is NULL.
 *
 * @note The table does not own inserted values, so the caller is responsible
 * for freeing or otherwise managing the value if needed.
 * @note Removing a key decreases cashash_size() by 1.
 */
bool cashash_remove(cashash_t *table, const void *key, const size_t key_len);

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
 * @}
 */

#endif
