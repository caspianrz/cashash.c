#include <cashash.c/cashash.h>
#include <cashash.c/cashash_hash.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CASHASH_MAX_LOAD_NUMERATOR 3
#define CASHASH_MAX_LOAD_DENOMINATOR 4
#define CASHASH_GROWTH_FACTOR 2

cashash_t *cashash_create(size_t bucket_count) {
  cashash_hash_strategy_t strategy = CASHASH_HASH_STRATEGY_FNV1A;
  cashash_strategy_option_t options;

  options.used = false;
#if CASHASH_USE_XXHASH
  options.xxh64.seed = 0;
#endif

  return cashash_create_with_strategy(bucket_count, strategy, options);
}

cashash_t *cashash_create_with_strategy(size_t bucket_count,
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
  config.equal = cashash_equal_fnv1a_bytes;
  config.copy_key = cashash_copy_fnv1a_bytes;
  config.destroy_key = cashash_key_destroy_fnv1a_bytes;

  return cashash_create_with_config(bucket_count, config, option);
}

cashash_t *cashash_create_with_config(size_t bucket_count,
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

  table->buckets = calloc(bucket_count, sizeof(cashash_node_t *));
  if (table->buckets == NULL) {
    free(table);
    return NULL;
  }

  table->bucket_count = bucket_count;
  table->size = 0;

  table->config = config;
  table->option = option;

  return table;
}

bool cashash_remove(cashash_t *table, const cashash_key_datum_t key) {
  size_t hash;
  size_t index;
  cashash_node_t *node;
  cashash_node_t *prev;

  if (table == NULL || key.data == NULL || table->config.hash == NULL ||
      table->config.equal == NULL) {
    return false;
  }

#ifdef CASHASH_USE_XXHASH
  if (table->option.used &&
      table->config.strategy == CASHASH_HASH_STRATEGY_XXH64) {
    hash = table->config.hash(key.data, key.length, table->option.xxh64.seed);
  } else {
    hash = table->config.hash(key.data, key.length);
  }
#else
  hash = table->config.hash(key.data, key.length);
#endif

  index = hash % table->bucket_count;

  prev = NULL;
  node = table->buckets[index];

  while (node != NULL) {
    if (node->key.length == key.length &&
        table->config.equal(node->key.data, key.data, key.length)) {
      if (prev == NULL) {
        table->buckets[index] = node->next;
      } else {
        prev->next = node->next;
      }

      if (table->config.destroy_key != NULL) {
        table->config.destroy_key(node->key.data);
      }

      free(node);
      table->size--;

      return true;
    }

    prev = node;
    node = node->next;
  }

  return false;
}

void cashash_clear(cashash_t *table) {
  size_t i;
  cashash_node_t *node;
  cashash_node_t *next;

  if (table == NULL || table->buckets == NULL) {
    return;
  }

  for (i = 0; i < table->bucket_count; i++) {
    node = table->buckets[i];

    while (node != NULL) {
      next = node->next;

      if (table->config.destroy_key != NULL) {
        table->config.destroy_key(node->key.data);
      }

      free(node);
      node = next;
    }

    table->buckets[i] = NULL;
  }

  table->size = 0;
}

void cashash_destroy(cashash_t *table) {
  if (table == NULL) {
    return;
  }

  cashash_clear(table);
  free(table->buckets);
  free(table);
}

bool cashash_insert(cashash_t *table, const cashash_key_datum_t key,
                    void *value) {
  size_t hash;
  size_t index;
  cashash_node_t *node;
  cashash_node_t *new_node;
  void *key_copy;

  if (table == NULL || key.data == NULL || table->buckets == NULL ||
      table->config.hash == NULL || table->config.equal == NULL ||
      table->config.copy_key == NULL) {
    return false;
  }

#ifdef CASHASH_USE_XXHASH
  if (table->option.used &&
      table->config.strategy == CASHASH_HASH_STRATEGY_XXH64) {
    hash = table->config.hash(key.data, key.length, table->option.xxh64.seed);
  } else {
    hash = table->config.hash(key.data, key.length);
  }
#else
  hash = table->config.hash(key.data, key.length);
#endif

  index = hash % table->bucket_count;

  node = table->buckets[index];
  while (node != NULL) {
    if (node->key.length == key.length &&
        table->config.equal(node->key.data, key.data, key.length)) {
      node->value = value;
      return true;
    }

    node = node->next;
  }

  if ((table->size + 1) * CASHASH_MAX_LOAD_DENOMINATOR >
      table->bucket_count * CASHASH_MAX_LOAD_NUMERATOR) {
    size_t new_bucket_count;
    cashash_node_t **new_buckets;
    size_t i;

    new_bucket_count = table->bucket_count * CASHASH_GROWTH_FACTOR;
    if (new_bucket_count <= table->bucket_count) {
      return false;
    }

    new_buckets = calloc(new_bucket_count, sizeof(cashash_node_t *));
    if (new_buckets == NULL) {
      return false;
    }

    for (i = 0; i < table->bucket_count; i++) {
      node = table->buckets[i];

      while (node != NULL) {
        cashash_node_t *next;
        size_t node_hash;
        size_t node_index;

        next = node->next;

#ifdef CASHASH_USE_XXHASH
        if (table->option.used &&
            table->config.strategy == CASHASH_HASH_STRATEGY_XXH64) {
          node_hash = table->config.hash(node->key.data, node->key.length,
                                         table->option.xxh64.seed);
        } else {
          node_hash = table->config.hash(node->key.data, node->key.length);
        }
#else
        node_hash = table->config.hash(node->key.data, node->key.length);
#endif

        node_index = node_hash % new_bucket_count;
        node->next = new_buckets[node_index];
        new_buckets[node_index] = node;

        node = next;
      }
    }

    free(table->buckets);

    table->buckets = new_buckets;
    table->bucket_count = new_bucket_count;

    index = hash % table->bucket_count;
  }

  key_copy = table->config.copy_key(key.data, key.length);
  if (key_copy == NULL) {
    return false;
  }

  new_node = malloc(sizeof(cashash_node_t));
  if (new_node == NULL) {
    if (table->config.destroy_key != NULL) {
      table->config.destroy_key(key_copy);
    }

    return false;
  }

  new_node->key.data = key_copy;
  new_node->key.length = key.length;
  new_node->value = value;
  new_node->next = table->buckets[index];

  table->buckets[index] = new_node;
  table->size++;

  return true;
}

void *cashash_find(const cashash_t *table, const cashash_key_datum_t key) {
  size_t hash;
  size_t index;
  cashash_node_t *node;

  if (table == NULL || key.data == NULL || table->buckets == NULL ||
      table->config.hash == NULL || table->config.equal == NULL) {
    return NULL;
  }

#ifdef CASHASH_USE_XXHASH
  if (table->option.used &&
      table->config.strategy == CASHASH_HASH_STRATEGY_XXH64) {
    hash = table->config.hash(key.data, key.length, table->option.xxh64.seed);
  } else {
    hash = table->config.hash(key.data, key.length);
  }
#else
  hash = table->config.hash(key.data, key.length);
#endif

  index = hash % table->bucket_count;

  node = table->buckets[index];
  while (node != NULL) {
    if (node->key.length == key.length &&
        table->config.equal(node->key.data, key.data, key.length)) {
      return node->value;
    }

    node = node->next;
  }

  return NULL;
}

size_t cashash_size(const cashash_t *table) {
  if (table == NULL) {
    return 0;
  }

  return table->size;
}

size_t cashash_bucket_count(const cashash_t *table) {
  if (table == NULL) {
    return 0;
  }

  return table->bucket_count;
}
