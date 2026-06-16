#pragma once

#ifndef CASHASH_ITER_H
#define CASHASH_ITER_H

#include <stdbool.h>
#include <stddef.h>

#include <cashash.c/cashash.h>

/**
 * @file cashash_iter.h
 * @brief Iterator API for cashash hash tables.
 *
 * This header provides functions for iterating over all key-value pairs stored
 * in a hash table.
 */

/**
 * @struct cashash_iter_s
 * @brief Hash table iterator.
 *
 * Tracks the current position while iterating over table entries.
 */
typedef struct cashash_iter_s {
  /**
   * @brief Table being iterated.
   */
  cashash_t *table;

  /**
   * @brief Current bucket index.
   */
  size_t bucket_index;

  /**
   * @brief Current node in the bucket chain.
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

#endif /* CASHASH_ITER_H */
