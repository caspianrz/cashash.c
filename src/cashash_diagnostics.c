#include <cashash.c/cashash_diagnostics.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static void cashash_validate_result_set(cashash_validate_result_t *result,
                                        bool valid,
                                        cashash_validate_error_t error,
                                        size_t bucket_index,
                                        size_t entry_index) {
  if (result == NULL) {
    return;
  }

  result->valid = valid;
  result->error = error;
  result->bucket_index = bucket_index;
  result->entry_index = entry_index;
}

static void cashash_stats_zero(cashash_stats_t *stats) {
  if (stats == NULL) {
    return;
  }

  *stats = (cashash_stats_t){
      .entries = 0,
      .capacity = 0,
      .load_factor = 0.0,
      .collisions = 0,
      .tombstones = 0,
      .max_probe_length = 0,
      .avg_probe_length = 0.0,
      .used_buckets = 0,
      .empty_buckets = 0,
      .longest_chain = 0,
  };
}

static bool cashash_validate_base(const cashash_t *table,
                                  cashash_validate_result_t *result) {
  if (table == NULL) {
    cashash_validate_result_set(result, false, CASHASH_VALIDATE_NULL_TABLE, 0,
                                0);
    return false;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    if (table->storage.chain.bucket_count == 0 ||
        table->storage.chain.buckets == NULL) {
      cashash_validate_result_set(result, false, CASHASH_VALIDATE_NULL_BUCKETS,
                                  0, 0);
      return false;
    }

    return true;

  case CASHASH_TABLE_OPEN_ADDRESSING:
    if (table->storage.oa.bucket_count == 0 ||
        table->storage.oa.entries == NULL) {
      cashash_validate_result_set(result, false, CASHASH_VALIDATE_NULL_BUCKETS,
                                  0, 0);
      return false;
    }

    return true;

  default:
    cashash_validate_result_set(result, false, CASHASH_VALIDATE_INVALID_ENTRY,
                                0, 0);
    return false;
  }
}

static bool cashash_try_recompute_hash(const cashash_t *table,
                                       cashash_key_datum_t key,
                                       size_t *hash_out) {
  if (table == NULL || hash_out == NULL || table->config.hash == NULL) {
    return false;
  }

#ifdef CASHASH_USE_XXHASH
  if (table->config.strategy == CASHASH_HASH_STRATEGY_XXH64 &&
      table->option.used) {
    *hash_out =
        table->config.hash(key.data, key.length, table->option.xxh64.seed);
    return true;
  }
#endif

  *hash_out = table->config.hash(key.data, key.length);
  return true;
}

static bool cashash_validate_chaining(const cashash_t *table,
                                      cashash_validate_result_t *result) {
  const cashash_chain_table_t *chain = &table->storage.chain;

  size_t counted_entries = 0;

  for (size_t i = 0; i < chain->bucket_count; ++i) {
    const cashash_node_t *slow = chain->buckets[i];
    const cashash_node_t *fast = chain->buckets[i];

    while (fast != NULL && fast->next != NULL) {
      slow = slow->next;
      fast = fast->next->next;

      if (slow == fast) {
        cashash_validate_result_set(
            result, false, CASHASH_VALIDATE_CYCLE_DETECTED, i, counted_entries);
        return false;
      }
    }

    size_t chain_index = 0;

    for (const cashash_node_t *node = chain->buckets[i]; node != NULL;
         node = node->next) {
      if (node->key.data == NULL) {
        cashash_validate_result_set(
            result, false, CASHASH_VALIDATE_INVALID_ENTRY, i, chain_index);
        return false;
      }

      size_t trusted_hash = node->hash;
      size_t recomputed_hash = 0;

      if (cashash_try_recompute_hash(table, node->key, &recomputed_hash)) {
        if (recomputed_hash != node->hash) {
          cashash_validate_result_set(
              result, false, CASHASH_VALIDATE_HASH_MISMATCH, i, chain_index);
          return false;
        }

        trusted_hash = recomputed_hash;
      }

      const size_t expected_bucket = trusted_hash % chain->bucket_count;

      if (expected_bucket != i) {
        cashash_validate_result_set(
            result, false, CASHASH_VALIDATE_BUCKET_MISMATCH, i, chain_index);
        return false;
      }

      ++chain_index;
      ++counted_entries;
    }
  }

  if (counted_entries != chain->size) {
    cashash_validate_result_set(result, false, CASHASH_VALIDATE_SIZE_MISMATCH,
                                0, counted_entries);
    return false;
  }

  cashash_validate_result_set(result, true, CASHASH_VALIDATE_OK, 0, 0);
  return true;
}

static bool cashash_stats_chaining(const cashash_t *table,
                                   cashash_stats_t *stats) {
  const cashash_chain_table_t *chain = &table->storage.chain;

  for (size_t i = 0; i < chain->bucket_count; ++i) {
    size_t chain_length = 0;

    for (const cashash_node_t *node = chain->buckets[i]; node != NULL;
         node = node->next) {
      ++chain_length;
    }

    if (chain_length == 0) {
      ++stats->empty_buckets;
      continue;
    }

    ++stats->used_buckets;

    if (chain_length > stats->longest_chain) {
      stats->longest_chain = chain_length;
    }

    if (chain_length > 1) {
      stats->collisions += chain_length - 1;
    }
  }

  return true;
}

static size_t cashash_oa_linear_distance(size_t from, size_t to,
                                         size_t capacity) {
  if (capacity == 0) {
    return 0;
  }

  if (to >= from) {
    return to - from;
  }

  return capacity - from + to;
}

static bool
cashash_validate_open_addressing(const cashash_t *table,
                                 cashash_validate_result_t *result) {
  const cashash_oa_table_t *oa = &table->storage.oa;

  size_t counted_entries = 0;
  size_t counted_deleted = 0;

  for (size_t i = 0; i < oa->bucket_count; ++i) {
    const cashash_oa_entry_t *entry = &oa->entries[i];

    switch (entry->state) {
    case CASHASH_OA_ENTRY_EMPTY:
      break;

    case CASHASH_OA_ENTRY_DELETED:
      ++counted_deleted;
      break;

    case CASHASH_OA_ENTRY_OCCUPIED: {
      if (entry->key.data == NULL) {
        cashash_validate_result_set(
            result, false, CASHASH_VALIDATE_INVALID_ENTRY, i, counted_entries);
        return false;
      }

      const size_t ideal_bucket = entry->hash % oa->bucket_count;

      if (ideal_bucket >= oa->bucket_count) {
        cashash_validate_result_set(result, false,
                                    CASHASH_VALIDATE_BUCKET_MISMATCH, i,
                                    counted_entries);
        return false;
      }

      size_t recomputed_hash = 0;

      if (cashash_try_recompute_hash(table, entry->key, &recomputed_hash) &&
          recomputed_hash != entry->hash) {
        cashash_validate_result_set(
            result, false, CASHASH_VALIDATE_HASH_MISMATCH, i, counted_entries);
        return false;
      }

      ++counted_entries;
      break;
    }

    default:
      cashash_validate_result_set(result, false, CASHASH_VALIDATE_INVALID_ENTRY,
                                  i, counted_entries);
      return false;
    }
  }

  if (counted_entries != oa->size || counted_deleted != oa->deleted_count) {
    cashash_validate_result_set(result, false, CASHASH_VALIDATE_SIZE_MISMATCH,
                                0, counted_entries);
    return false;
  }

  cashash_validate_result_set(result, true, CASHASH_VALIDATE_OK, 0, 0);
  return true;
}

static bool cashash_stats_open_addressing(const cashash_t *table,
                                          cashash_stats_t *stats) {
  const cashash_oa_table_t *oa = &table->storage.oa;

  size_t total_probe_length = 0;

  for (size_t i = 0; i < oa->bucket_count; ++i) {
    const cashash_oa_entry_t *entry = &oa->entries[i];

    switch (entry->state) {
    case CASHASH_OA_ENTRY_EMPTY:
      ++stats->empty_buckets;
      break;

    case CASHASH_OA_ENTRY_DELETED:
      ++stats->tombstones;
      break;

    case CASHASH_OA_ENTRY_OCCUPIED: {
      ++stats->used_buckets;

      const size_t ideal_bucket = entry->hash % oa->bucket_count;

      /*
       * This is exact for linear probing.
       *
       * For quadratic probing and double hashing, this is a useful physical
       * displacement metric, but not necessarily the exact probe count unless
       * your probe sequence is recomputed here.
       */
      const size_t probe_length =
          cashash_oa_linear_distance(ideal_bucket, i, oa->bucket_count);

      if (probe_length > 0) {
        ++stats->collisions;
      }

      if (probe_length > stats->max_probe_length) {
        stats->max_probe_length = probe_length;
      }

      total_probe_length += probe_length;
      break;
    }

    default:
      break;
    }
  }

  if (stats->entries > 0) {
    stats->avg_probe_length =
        (double)total_probe_length / (double)stats->entries;
  }

  return true;
}

bool cashash_validate(const cashash_t *table,
                      cashash_validate_result_t *result) {
  cashash_validate_result_set(result, false, CASHASH_VALIDATE_OK, 0, 0);

  if (!cashash_validate_base(table, result)) {
    return false;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    return cashash_validate_chaining(table, result);

  case CASHASH_TABLE_OPEN_ADDRESSING:
    return cashash_validate_open_addressing(table, result);

  default:
    cashash_validate_result_set(result, false, CASHASH_VALIDATE_INVALID_ENTRY,
                                0, 0);
    return false;
  }
}

bool cashash_stats(const cashash_t *table, cashash_stats_t *stats) {
  if (table == NULL || stats == NULL) {
    return false;
  }

  cashash_stats_zero(stats);

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    if (table->storage.chain.bucket_count == 0 ||
        table->storage.chain.buckets == NULL) {
      return false;
    }

    stats->entries = table->storage.chain.size;
    stats->capacity = table->storage.chain.bucket_count;
    stats->load_factor = (double)stats->entries / (double)stats->capacity;

    return cashash_stats_chaining(table, stats);

  case CASHASH_TABLE_OPEN_ADDRESSING:
    if (table->storage.oa.bucket_count == 0 ||
        table->storage.oa.entries == NULL) {
      return false;
    }

    stats->entries = table->storage.oa.size;
    stats->capacity = table->storage.oa.bucket_count;
    stats->load_factor = (double)stats->entries / (double)stats->capacity;

    return cashash_stats_open_addressing(table, stats);

  default:
    return false;
  }
}

void cashash_stats_print(FILE *stream, const cashash_stats_t *stats) {
  if (stream == NULL || stats == NULL) {
    return;
  }

  fprintf(stream, "Entries: %zu\n", stats->entries);
  fprintf(stream, "Capacity: %zu\n", stats->capacity);
  fprintf(stream, "Load Factor: %.2f\n", stats->load_factor);
  fprintf(stream, "Collisions: %zu\n", stats->collisions);

  if (stats->tombstones > 0) {
    fprintf(stream, "Tombstones: %zu\n", stats->tombstones);
  }

  if (stats->max_probe_length > 0 || stats->avg_probe_length > 0.0) {
    fprintf(stream, "Max Probe Length: %zu\n", stats->max_probe_length);
    fprintf(stream, "Avg Probe Length: %.2f\n", stats->avg_probe_length);
  }

  if (stats->used_buckets > 0 || stats->empty_buckets > 0) {
    fprintf(stream, "Used Buckets: %zu\n", stats->used_buckets);
    fprintf(stream, "Empty Buckets: %zu\n", stats->empty_buckets);
  }

  if (stats->longest_chain > 0) {
    fprintf(stream, "Longest Chain: %zu\n", stats->longest_chain);
  }
}
