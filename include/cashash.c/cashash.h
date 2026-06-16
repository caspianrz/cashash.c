#pragma once

#ifndef CASHASH_C_H
#define CASHASH_C_H

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
 * @defgroup cashash Cashash
 * @brief Generic byte-key hash table API.
 *
 * @{
 */

/**
 * @typedef cashash_node_t
 * @brief Alias for the internal hash table node type.
 */
typedef struct cashash_node_s cashash_node_t;

/**
 * @brief Opaque hash table type.
 *
 * The internal representation is hidden from users of the library.
 *
 * Create a table with cashash_create(), cashash_create_with_strategy(), or
 * cashash_create_with_config().
 *
 * Release a table with cashash_destroy().
 */
typedef struct cashash_s cashash_t;

/**
 * @brief Built-in hash strategy selector.
 *
 * This enum is used by cashash_create_with_strategy() and stored in
 * cashash_config_t so internal operations know which hash strategy is active.
 */
typedef enum {
  /**
   * @brief No built-in strategy.
   *
   * Intended for fully custom configurations passed to
   * cashash_create_with_config().
   */
  CASHASH_HASH_STRATEGY_NONE,

  /**
   * @brief Use the built-in FNV-1a byte hash.
   */
  CASHASH_HASH_STRATEGY_FNV1A,

#ifdef CASHASH_USE_XXHASH
  /**
   * @brief Use the built-in XXH3 byte hash.
   *
   * Available only when CASHASH_USE_XXHASH is enabled.
   */
  CASHASH_HASH_STRATEGY_XXH3,

  /**
   * @brief Use the built-in XXH64 byte hash.
   *
   * Available only when CASHASH_USE_XXHASH is enabled.
   */
  CASHASH_HASH_STRATEGY_XXH64,
#endif
} cashash_hash_strategy_t;

/**
 * @brief Represents a generic key datum used by a cashash table.
 *
 * A key datum is a pointer-length pair used to describe arbitrary key data.
 * The `data` pointer is `const` because hash table operations should not modify
 * key bytes supplied by the caller.
 *
 * @note This struct does not imply ownership. Whether the hash table copies,
 * borrows, or frees the pointed-to key data depends on the table configuration
 * and API used.
 */
typedef struct cashash_key_datum_s {
  /**
   * @brief Pointer to the key data.
   *
   * May point to any binary or user-defined key data. The pointed-to data
   * should remain valid for the duration required by the API or table
   * configuration.
   */
  const void *data;

  /**
   * @brief Length of the key data in bytes.
   *
   * For string keys, this should usually exclude the terminating `'\0'`.
   */
  size_t length;
} cashash_key_datum_t;

/**
 * @brief Represents a key-value pair in a cashash table.
 *
 * This type groups a key datum and a value datum together. It is useful for
 * iterator APIs, foreach callbacks, insertion helpers, and APIs that return or
 * operate on whole hash table entries.
 *
 * @note The pair itself does not imply ownership of either `key.data` or
 * `value.data`.
 */
typedef struct cashash_pair_s {
  /**
   * @brief Key component of the pair.
   */
  cashash_key_datum_t key;

  /**
   * @brief Value component of the pair.
   */
  void *value;
} cashash_pair_t;

/**
 * @brief Hash function callback type.
 *
 * Computes a hash value for `key_len` bytes starting at `key`.
 *
 * The variadic arguments are reserved for strategy-specific options, such as
 * an xxHash seed.
 *
 * @param key Pointer to key bytes.
 * @param len Number of bytes in the key.
 *
 * @return Hash value for the provided key bytes.
 */
typedef size_t (*cashash_hash_fn)(const void *key, const size_t len, ...);

/**
 * @brief Key equality callback type.
 *
 * Compares exactly `len` bytes from `a` and `b`.
 *
 * Implementations should be binary-safe and should not depend on
 * null-terminated strings.
 *
 * @param a First key pointer.
 * @param b Second key pointer.
 * @param len Number of bytes to compare.
 *
 * @return true if the keys are equal.
 * @return false otherwise.
 */
typedef bool (*cashash_equal_fn)(const void *a, const void *b, size_t len);

/**
 * @brief Key copy callback type.
 *
 * Creates an owned copy of `len` bytes from `key`.
 *
 * The returned pointer is stored internally by the hash table and later passed
 * to the configured cashash_key_destroy_fn.
 *
 * @param key Pointer to key bytes to copy.
 * @param len Number of bytes to copy.
 *
 * @return Pointer to copied key memory on success.
 * @return NULL on allocation failure.
 */
typedef void *(*cashash_key_copy_fn)(const void *key, size_t len);

/**
 * @brief Key destroy callback type.
 *
 * Destroys a key previously returned by cashash_key_copy_fn.
 *
 * @param key Copied key pointer to destroy.
 */
typedef void (*cashash_key_destroy_fn)(const void *key);

/**
 * @brief Hash table configuration.
 *
 * This structure allows callers to provide custom hashing, equality, key copy,
 * and key destroy behavior.
 *
 * For the default byte-key behavior, use cashash_create().
 */
typedef struct {
  /**
   * @brief Hash strategy identifier.
   *
   * Use CASHASH_HASH_STRATEGY_NONE for fully custom callbacks.
   */
  cashash_hash_strategy_t strategy;

  /**
   * @brief Function used to hash keys.
   */
  cashash_hash_fn hash;

  /**
   * @brief Function used to compare keys.
   */
  cashash_equal_fn equal;

  /**
   * @brief Function used to copy keys before storing them.
   */
  cashash_key_copy_fn copy_key;

  /**
   * @brief Function used to destroy copied keys.
   */
  cashash_key_destroy_fn destroy_key;
} cashash_config_t;

/**
 * @brief Strategy-specific hash options.
 *
 * This structure stores optional settings for built-in hash strategies.
 *
 * `used` indicates whether strategy-specific options should be used.
 */
typedef struct {
  /**
   * @brief Whether strategy-specific options are enabled.
   */
  bool used;

#ifdef CASHASH_USE_XXHASH
  /**
   * @brief Options for the XXH64 strategy.
   */
  struct {
    /**
     * @brief Seed passed to XXH64.
     */
    uint64_t seed;
  } xxh64;
#endif
} cashash_strategy_option_t;

/**
 * @struct cashash_node_s
 * @brief Internal hash table node.
 *
 * Stores one key-value pair inside a bucket chain.
 */
struct cashash_node_s {
  /**
   * @brief Next node in the bucket chain.
   */
  struct cashash_node_s *next;

  /**
   * @brief Cached hash of the key.
   */
  size_t hash;

  /**
   * @brief Key datum for this entry.
   */
  cashash_key_datum_t key;

  /**
   * @brief Value pointer for this entry.
   */
  void *value;
};

/**
 * @struct cashash_s
 * @brief Hash table instance.
 *
 * Stores the bucket array, entry count, and configuration used by the table.
 */
struct cashash_s {
  /**
   * @brief Array of bucket chains.
   */
  cashash_node_t **buckets;

  /**
   * @brief Number of buckets in the table.
   */
  size_t bucket_count;

  /**
   * @brief Number of entries stored in the table.
   */
  size_t size;

  /**
   * @brief Hash table behavior configuration.
   */
  cashash_config_t config;

  /**
   * @brief Strategy-specific options.
   */
  cashash_strategy_option_t option;
};

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
cashash_t *cashash_create(size_t bucket_count);

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
cashash_t *cashash_create_with_strategy(size_t bucket_count,
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
cashash_t *cashash_create_with_config(size_t bucket_count,
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
 * @}
 */

#endif
