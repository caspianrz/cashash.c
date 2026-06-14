#pragma once

#ifndef CASHASH_C_H
#define CASHASH_C_H

#include <stdbool.h>
#include <stddef.h>

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
bool cashash_insert(cashash_t *table, const char *key, void *value);

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
void *cashash_find(const cashash_t *table, const char *key);

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

bool cashash_remove(cashash_t *table, const char *key);

void cashash_clear(cashash_t *table);

/**
 * @}
 */

#endif
