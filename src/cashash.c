#include <cashash.c/cashash.h>
#include <cashash.c/cashash_chain.h>
#include <cashash.c/cashash_oa.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

cashash_t *cashash_create_chain(size_t bucket_count) {
  cashash_hash_strategy_t strategy = CASHASH_HASH_STRATEGY_FNV1A;
  cashash_strategy_option_t options;

  options.used = false;
#if CASHASH_USE_XXHASH
  options.xxh64.seed = 0;
#endif
  return cashash_create_chain_with_strategy(bucket_count, strategy, options);
}

cashash_t *
cashash_create_chain_with_strategy(size_t bucket_count,
                                   cashash_hash_strategy_t strategy,
                                   cashash_strategy_option_t option) {
  cashash_config_t config;
  switch (strategy) {
  case CASHASH_HASH_STRATEGY_NONE:
  case CASHASH_HASH_STRATEGY_FNV1A:
    config.hash = cashash_hash_fnv1a_bytes;
    break;
#ifdef CASHASH_USE_XXHASH
  case CASHASH_HASH_STRATEGY_XXH3:
    config.hash = cashash_hash_xxh3_bytes;
    break;
  case CASHASH_HASH_STRATEGY_XXH64:
    config.hash = cashash_hash_xxh64_bytes;
#endif
    break;
  }
  config.equal = cashash_equal_bytes;
  config.copy_key = cashash_copy_bytes;
  config.destroy_key = cashash_key_destroy_bytes;

  return cashash_create_chain_with_config(bucket_count, config, option);
}

cashash_t *cashash_create_chain_with_config(size_t bucket_count,
                                            cashash_config_t config,
                                            cashash_strategy_option_t option) {
  cashash_t *table;

  if (bucket_count == 0) {
    return NULL;
  }

  if (config.hash == NULL || config.equal == NULL || config.copy_key == NULL ||
      config.destroy_key == NULL) {
    return NULL;
  }

  table = malloc(sizeof(cashash_t));
  if (table == NULL) {
    return NULL;
  }

  table->storage.chain.buckets = calloc(bucket_count, sizeof(cashash_node_t *));
  if (table->storage.chain.buckets == NULL) {
    free(table);
    return NULL;
  }

  table->storage.chain.bucket_count = bucket_count;
  table->storage.chain.size = 0;

  table->config = config;
  table->option = option;

  return table;
}

bool cashash_remove(cashash_t *table, const cashash_key_datum_t key) {
  if (table == NULL) {
    return false;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    return cashash_chain_remove(table, key);

  case CASHASH_TABLE_OPEN_ADDRESSING:
    return cashash_oa_remove(table, key);
  }
  return false;
}

void cashash_clear(cashash_t *table) {
  if (table == NULL) {
    return;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    cashash_chain_clear(table);
    break;

  case CASHASH_TABLE_OPEN_ADDRESSING:
    cashash_oa_clear(table);
    break;
  }
}

void cashash_destroy(cashash_t *table) {
  if (table == NULL) {
    return;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    cashash_chain_destroy(table);
    break;

  case CASHASH_TABLE_OPEN_ADDRESSING:
    cashash_oa_clear(table);
    break;
  }

  free(table);
}

bool cashash_insert(cashash_t *table, const cashash_key_datum_t key,
                    void *value) {
  if (table == NULL) {
    return false;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    return cashash_chain_insert(table, key, value);

  case CASHASH_TABLE_OPEN_ADDRESSING:
    return cashash_oa_insert(table, key, value);
  }

  return false;
}

void *cashash_find(const cashash_t *table, const cashash_key_datum_t key) {
  if (table == NULL) {
    return NULL;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    return cashash_chain_find(table, key);

  case CASHASH_TABLE_OPEN_ADDRESSING:
    return cashash_oa_find(table, key);
  }

  return NULL;
}
