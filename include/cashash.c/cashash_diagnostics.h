#pragma once

#ifndef CASHASH_DIAGNOSTICS_H
#define CASHASH_DIAGNOSTICS_H

#include <cashash.c/cashash_type.h>

#include <stdio.h>

typedef enum cashash_validate_error_e {
  CASHASH_VALIDATE_OK = 0,
  CASHASH_VALIDATE_NULL_TABLE,
  CASHASH_VALIDATE_NULL_BUCKETS,
  CASHASH_VALIDATE_SIZE_MISMATCH,
  CASHASH_VALIDATE_INVALID_ENTRY,
  CASHASH_VALIDATE_HASH_MISMATCH,
  CASHASH_VALIDATE_BUCKET_MISMATCH,
  CASHASH_VALIDATE_CYCLE_DETECTED
} cashash_validate_error_t;

typedef struct cashash_validate_result_s {
  bool valid;
  cashash_validate_error_t error;
  size_t bucket_index;
  size_t entry_index;
} cashash_validate_result_t;

typedef struct cashash_stats_s {
  size_t entries;
  size_t capacity;

  double load_factor;

  size_t collisions;

  /**
   * Useful mostly for open addressing.
   */
  size_t tombstones;
  size_t max_probe_length;
  double avg_probe_length;

  /**
   * Useful mostly for separate chaining.
   */
  size_t used_buckets;
  size_t empty_buckets;
  size_t longest_chain;
} cashash_stats_t;

bool cashash_validate(const cashash_t *table,
                      cashash_validate_result_t *result);

bool cashash_stats(const cashash_t *table, cashash_stats_t *stats);

void cashash_stats_print(FILE *stream, const cashash_stats_t *stats);

#endif
