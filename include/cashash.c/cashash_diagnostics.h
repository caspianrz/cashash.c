#pragma once

#ifndef CASHASH_DIAGNOSTICS_H
#define CASHASH_DIAGNOSTICS_H

#include <cashash.c/cashash_type.h>

#include <stdio.h>

/**
 * @brief Validation error codes returned by cashash_validate().
 *
 * These values describe the first internal consistency error found while
 * validating a hash table.
 */
typedef enum cashash_validate_error_e {
  /**
   * @brief No validation error was found.
   */
  CASHASH_VALIDATE_OK = 0,

  /**
   * @brief The table pointer passed to cashash_validate() was NULL.
   */
  CASHASH_VALIDATE_NULL_TABLE,

  /**
   * @brief The table has no valid bucket or entry storage.
   *
   * This may mean that the bucket array for a chaining table is NULL, the entry
   * array for an open-addressing table is NULL, or the table capacity is zero.
   */
  CASHASH_VALIDATE_NULL_BUCKETS,

  /**
   * @brief The stored table size does not match the number of entries found.
   *
   * For separate chaining, this means the number of nodes found in all chains
   * differs from the table's stored size.
   *
   * For open addressing, this means the number of occupied entries or deleted
   * entries differs from the stored counters.
   */
  CASHASH_VALIDATE_SIZE_MISMATCH,

  /**
   * @brief An invalid entry was found.
   *
   * This may indicate a NULL key pointer, an invalid open-addressing entry
   * state, or another malformed internal entry.
   */
  CASHASH_VALIDATE_INVALID_ENTRY,

  /**
   * @brief A stored cached hash does not match the recomputed key hash.
   *
   * This error is only reported when the table has enough configuration
   * information to recompute the hash.
   */
  CASHASH_VALIDATE_HASH_MISMATCH,

  /**
   * @brief An entry was found in the wrong bucket.
   *
   * For separate chaining, this means the entry's hash does not map to the
   * bucket chain where the node was found.
   */
  CASHASH_VALIDATE_BUCKET_MISMATCH,

  /**
   * @brief A cycle was detected in a separate-chaining bucket list.
   *
   * This indicates that a linked list inside the table contains a loop and
   * cannot be safely traversed normally.
   */
  CASHASH_VALIDATE_CYCLE_DETECTED
} cashash_validate_error_t;

/**
 * @brief Result object filled by cashash_validate().
 *
 * This structure provides details about the result of a validation operation.
 * If validation fails, `error` describes the reason and the index fields point
 * to the approximate location where the problem was detected.
 */
typedef struct cashash_validate_result_s {
  /**
   * @brief Whether the table passed validation.
   *
   * This is true only when no consistency error was found.
   */
  bool valid;

  /**
   * @brief Validation error code.
   *
   * This is CASHASH_VALIDATE_OK when `valid` is true.
   */
  cashash_validate_error_t error;

  /**
   * @brief Bucket or entry-array index where the error was detected.
   *
   * For separate chaining, this is the bucket index.
   * For open addressing, this is the entry index.
   */
  size_t bucket_index;

  /**
   * @brief Entry position related to the error.
   *
   * For separate chaining, this is usually the node position inside the bucket
   * chain.
   *
   * For open addressing, this may be the number of occupied entries counted
   * before the error was found.
   */
  size_t entry_index;
} cashash_validate_result_t;

/**
 * @brief Statistics collected from a hash table.
 *
 * This structure contains general hash table statistics, plus fields that are
 * mainly useful for either open addressing or separate chaining.
 *
 * The meaning of collision statistics depends on the table kind:
 *
 * - For separate chaining, `collisions` is usually calculated as the sum of
 *   `chain_length - 1` for all non-empty chains.
 * - For open addressing, `collisions` is usually calculated from entries that
 *   are not stored in their ideal hash bucket.
 */
typedef struct cashash_stats_s {
  /**
   * @brief Number of active key-value pairs stored in the table.
   */
  size_t entries;

  /**
   * @brief Number of buckets or entry slots in the table.
   *
   * For separate chaining, this is the number of bucket chains.
   * For open addressing, this is the number of entry slots.
   */
  size_t capacity;

  /**
   * @brief Current load factor of the table.
   *
   * Calculated as:
   *
   * @code
   * entries / capacity
   * @endcode
   */
  double load_factor;

  /**
   * @brief Number of detected collisions.
   *
   * For separate chaining, this is typically the total number of entries after
   * the first entry in each bucket chain.
   *
   * For open addressing, this is typically the number of occupied entries whose
   * current position differs from their ideal hash bucket.
   */
  size_t collisions;

  /**
   * @brief Number of deleted entries in an open-addressing table.
   *
   * Also known as tombstones. This value is mostly useful for open addressing.
   * For separate chaining, this is usually zero.
   */
  size_t tombstones;

  /**
   * @brief Maximum observed probe length.
   *
   * This is mostly useful for open-addressing tables.
   *
   * For linear probing, this can represent the distance between an entry's
   * ideal bucket and its actual slot.
   *
   * For quadratic probing and double hashing, this value is only exact if the
   * implementation recomputes the original probe sequence or stores the probe
   * length during insertion.
   */
  size_t max_probe_length;

  /**
   * @brief Average observed probe length.
   *
   * This is mostly useful for open-addressing tables. For separate chaining,
   * this is usually zero.
   */
  double avg_probe_length;

  /**
   * @brief Number of non-empty buckets or occupied slots.
   *
   * For separate chaining, this is the number of bucket chains containing at
   * least one node.
   *
   * For open addressing, this is usually the number of occupied slots.
   */
  size_t used_buckets;

  /**
   * @brief Number of empty buckets or empty slots.
   *
   * For separate chaining, this is the number of bucket chains containing no
   * nodes.
   *
   * For open addressing, this is the number of slots in the empty state.
   */
  size_t empty_buckets;

  /**
   * @brief Length of the longest bucket chain.
   *
   * This is mostly useful for separate chaining. For open addressing, this is
   * usually zero.
   */
  size_t longest_chain;
} cashash_stats_t;

/**
 * @brief Validate the internal consistency of a hash table.
 *
 * This function checks whether the table's internal structure is valid.
 *
 * For separate chaining, validation may check:
 *
 * - whether the bucket array exists
 * - whether bucket chains contain cycles
 * - whether all nodes contain valid keys
 * - whether cached hashes match recomputed hashes
 * - whether nodes are stored in the correct bucket
 * - whether the counted node total matches the stored table size
 *
 * For open addressing, validation may check:
 *
 * - whether the entry array exists
 * - whether entry states are valid
 * - whether occupied entries contain valid keys
 * - whether cached hashes match recomputed hashes
 * - whether occupied and deleted counters are correct
 *
 * @param table Hash table to validate.
 * @param result Optional result object to fill with validation details.
 *               May be NULL if only the boolean result is needed.
 *
 * @return true if the table is valid.
 * @return false if the table is invalid or NULL.
 */
bool cashash_validate(const cashash_t *table,
                      cashash_validate_result_t *result);

/**
 * @brief Collect statistics from a hash table.
 *
 * This function fills a cashash_stats_t structure with information about the
 * current state of the table, such as entry count, capacity, load factor,
 * collision count, tombstone count, and chain/probe metrics.
 *
 * The exact meaning of some fields depends on whether the table uses separate
 * chaining or open addressing.
 *
 * @param table Hash table to inspect.
 * @param stats Output statistics object.
 *
 * @return true if statistics were collected successfully.
 * @return false if `table` or `stats` is NULL, or if the table has invalid
 *         internal storage.
 */
bool cashash_stats(const cashash_t *table, cashash_stats_t *stats);

/**
 * @brief Print hash table statistics to a stream.
 *
 * This function prints a human-readable summary of a cashash_stats_t object.
 *
 * Example output:
 *
 * @code
 * Entries: 1204
 * Capacity: 2048
 * Load Factor: 0.58
 * Collisions: 331
 * @endcode
 *
 * @param stream Output stream.
 *               For example, stdout, stderr, or a file stream.
 * @param stats Statistics object to print.
 *
 * @note If `stream` or `stats` is NULL, this function does nothing.
 */
void cashash_stats_print(FILE *stream, const cashash_stats_t *stats);

#endif
