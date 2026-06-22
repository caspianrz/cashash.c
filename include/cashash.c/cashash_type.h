#pragma once

#ifndef CASHASH_DATA_H
#define CASHASH_DATA_H

#include <cashash.c/cashash_hash.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
 * @defgroup cashash Cashash
 * @brief Generic byte-key hash table API.
 *
 * @{
 */

typedef enum cashash_table_kind_e {
  CASHASH_TABLE_CHAINING,
  CASHASH_TABLE_OPEN_ADDRESSING
} cashash_table_kind_t;

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
 * @typedef cashash_node_t
 * @brief Alias for the internal hash table node type.
 */
typedef struct cashash_node_s {
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
} cashash_node_t;

/**
 * @struct cashash_s
 * @brief Hash table instance.
 *
 * Stores the bucket array, entry count, and configuration used by the table.
 */
typedef struct cashash_chain_table_s {
  /**
   * @brief Array of bucket chains.
   *
   * Each bucket points to the first node in a linked list.
   */
  cashash_node_t **buckets;

  /**
   * @brief Number of buckets in the table.
   */
  size_t bucket_count;

  /**
   * @brief Number of key-value pairs stored in the table.
   */
  size_t size;
} cashash_chain_table_t;

typedef enum cashash_oa_entry_state_e {
  CASHASH_OA_ENTRY_EMPTY,
  CASHASH_OA_ENTRY_OCCUPIED,
  CASHASH_OA_ENTRY_DELETED,
} cashash_oa_entry_state_t;

typedef enum cashash_oa_probe_strategy_e {
  CASHASH_OA_PROBE_LINEAR,
  CASHASH_OA_PROBE_QUADRATIC,
  CASHASH_OA_PROBE_DOUBLE_HASHING,
} cashash_oa_probe_strategy_t;

typedef struct cashash_oa_entry_s {
  /**
   * @brief Current state of this entry.
   */
  cashash_oa_entry_state_t state;

  /**
   * @brief Cached hash of the key.
   */
  size_t hash;

  /**
   * @brief Key datum stored in this entry.
   */
  cashash_key_datum_t key;

  /**
   * @brief Value pointer stored in this entry.
   */
  void *value;
} cashash_oa_entry_t;

typedef struct cashash_oa_table_s {
  /**
   * @brief Array of open-addressing entries.
   */
  cashash_oa_entry_t *entries;

  /**
   * @brief Number of entries available in the table.
   */
  size_t bucket_count;

  /**
   * @brief Number of occupied entries.
   */
  size_t size;

  /**
   * @brief Number of deleted/tombstone entries.
   */
  size_t deleted_count;

  /**
   * @brief Probing strategy used for collision resolution.
   */
  cashash_oa_probe_strategy_t probe_strategy;

  /**
   * @brief Optional second hash function for double hashing.
   */
  cashash_hash_fn second_hash;

  /**
   * @brief User data passed to the second hash function.
   */
  void *second_hash_user_data;
} cashash_oa_table_t;

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
typedef struct cashash_s {
  cashash_table_kind_t kind;

  /**
   * @brief Hash table behavior configuration.
   */
  cashash_config_t config;
  /**
   * @brief Strategy-specific options.
   */
  cashash_strategy_option_t option;

  union {
    cashash_chain_table_t chain;
    cashash_oa_table_t oa;
  } storage;
} cashash_t;

#endif
