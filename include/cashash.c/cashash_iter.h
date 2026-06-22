
#pragma once

#ifndef CASHASH_ITER_H
#define CASHASH_ITER_H

#include <stdbool.h>
#include <stddef.h>

#include <cashash.c/cashash.h>
#include <cashash.c/cashash_type.h>

/**
 * @file cashash_iter.h
 * @brief Iterator API for cashash hash tables.
 *
 * This header provides functions for iterating over all key-value pairs stored
 * in both separate-chaining and open-addressing hash tables.
 */

/**
 * @struct cashash_iter_s
 * @brief Hash table iterator.
 *
 * Tracks the current position while iterating over table entries.
 *
 * For separate-chaining tables:
 * - bucket_index is the current bucket.
 * - current points to the current node in the bucket chain.
 *
 * For open-addressing tables:
 * - bucket_index is the current occupied entry slot.
 * - current is unused and remains NULL.
 */
typedef struct cashash_iter_s {
  /**
   * @brief Table being iterated.
   */
  cashash_t *table;

  /**
   * @brief Current bucket/slot index.
   */
  size_t bucket_index;

  /**
   * @brief Current node in a separate-chaining bucket.
   *
   * This field is only used by separate-chaining tables.
   * It is unused for open-addressing tables.
   */
  cashash_node_t *current;
} cashash_iter_t;

/**
 * @brief Initialize an iterator for a hash table.
 *
 * After initialization, the iterator points to the first available entry in the
 * table, if one exists.
 *
 * @param table Hash table to iterate over.
 * @param iter Iterator to initialize.
 */
void cashash_iter_init(cashash_t *table, cashash_iter_t *iter);

/**
 * @brief Check whether an iterator has another entry.
 *
 * This function does not advance the iterator.
 *
 * @param iter Iterator to check.
 *
 * @return `true` if another entry is available.
 * @return `false` if the iterator is exhausted or invalid.
 */
bool cashash_iter_has_next(cashash_iter_t *iter);

/**
 * @brief Get the next key-value pair and advance the iterator.
 *
 * The returned pair contains borrowed references to data stored in the table.
 * The caller must not free the returned key or value data unless it owns that
 * memory.
 *
 * @param iter Iterator to advance.
 * @param pair Output pointer that receives the current key-value pair.
 *
 * @return `true` if a pair was returned.
 * @return `false` if the iterator is exhausted or invalid.
 */
bool cashash_iter_next(cashash_iter_t *iter, cashash_pair_t *pair);

/**
 * @brief Get the next key from an iterator and advance it.
 *
 * Returns only the key of the current entry. The iterator is advanced to the
 * next entry after a key is returned.
 *
 * @param iter Iterator to advance.
 * @param out_key Output pointer that receives the current key.
 *
 * @return `true` if a key was returned.
 * @return `false` if the iterator is exhausted or invalid.
 */
bool cashash_iter_next_key(cashash_iter_t *iter, cashash_key_datum_t *out_key);

/**
 * @typedef cashash_foreach_fn
 * @brief Callback function used by cashash_foreach().
 *
 * This callback is called once for each key-value pair in the hash table.
 *
 * @param pair Current key-value pair.
 * @param user_data User-provided context pointer.
 *
 * @return `true` to continue iteration.
 * @return `false` to stop iteration early.
 */
typedef bool (*cashash_foreach_fn)(cashash_pair_t pair, void *user_data);

/**
 * @brief Iterate over all key-value pairs in a hash table.
 *
 * Calls the given callback once for each entry in the table.
 * Iteration stops early if the callback returns `false`.
 *
 * @param table Hash table to iterate over.
 * @param callback Function to call for each key-value pair.
 * @param user_data Optional user-provided context passed to the callback.
 *
 * @return `true` if all entries were visited.
 * @return `false` if the table or callback is invalid, or if the callback stops
 * iteration.
 */
bool cashash_foreach(cashash_t *table, cashash_foreach_fn callback,
                     void *user_data);

/**
 * @typedef cashash_key_foreach_fn
 * @brief Callback function used by cashash_foreach_key().
 *
 * This callback is called once for each key in the hash table.
 *
 * @param key Current key datum.
 * @param user_data User-provided context pointer.
 *
 * @return `true` to continue iteration.
 * @return `false` to stop iteration early.
 */
typedef bool (*cashash_key_foreach_fn)(cashash_key_datum_t key,
                                       void *user_data);

/**
 * @brief Iterate over all keys in a hash table.
 *
 * Calls the given callback once for each key in the table.
 * Iteration stops early if the callback returns `false`.
 *
 * @param table Hash table to iterate over.
 * @param callback Function to call for each key.
 * @param user_data Optional user-provided context passed to the callback.
 *
 * @return `true` if all keys were visited.
 * @return `false` if the table or callback is invalid, or if the callback stops
 * iteration.
 */
bool cashash_foreach_key(cashash_t *table, cashash_key_foreach_fn callback,
                         void *user_data);

#endif /* CASHASH_ITER_H */
