#include <cashash.c/cashash_chain.h>
#include <stdlib.h>

#define CASHASH_MAX_LOAD_NUMERATOR 3
#define CASHASH_MAX_LOAD_DENOMINATOR 4
#define CASHASH_GROWTH_FACTOR 2

bool cashash_chain_insert(cashash_t *table, const cashash_key_datum_t key,
                          void *data) {
  size_t hash;
  size_t index;
  cashash_node_t *node;
  cashash_node_t *new_node;
  void *key_copy;

  if (table == NULL || key.data == NULL ||
      table->storage.chain.buckets == NULL || table->config.hash == NULL ||
      table->config.equal == NULL || table->config.copy_key == NULL) {
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

  index = hash % table->storage.chain.bucket_count;

  node = table->storage.chain.buckets[index];
  while (node != NULL) {
    if (node->key.length == key.length &&
        table->config.equal(node->key.data, key.data, key.length)) {
      node->value = data;
      return true;
    }

    node = node->next;
  }

  if ((table->storage.chain.size + 1) * CASHASH_MAX_LOAD_DENOMINATOR >
      table->storage.chain.bucket_count * CASHASH_MAX_LOAD_NUMERATOR) {
    size_t new_bucket_count;
    cashash_node_t **new_buckets;
    size_t i;

    new_bucket_count =
        table->storage.chain.bucket_count * CASHASH_GROWTH_FACTOR;
    if (new_bucket_count <= table->storage.chain.bucket_count) {
      return false;
    }

    new_buckets = calloc(new_bucket_count, sizeof(cashash_node_t *));
    if (new_buckets == NULL) {
      return false;
    }

    for (i = 0; i < table->storage.chain.bucket_count; i++) {
      node = table->storage.chain.buckets[i];

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

    free(table->storage.chain.buckets);

    table->storage.chain.buckets = new_buckets;
    table->storage.chain.bucket_count = new_bucket_count;

    index = hash % table->storage.chain.bucket_count;
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
  new_node->value = data;
  new_node->next = table->storage.chain.buckets[index];

  table->storage.chain.buckets[index] = new_node;
  table->storage.chain.size++;

  return true;
}

bool cashash_chain_remove(cashash_t *table, const cashash_key_datum_t key) {
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

  index = hash % table->storage.chain.bucket_count;

  prev = NULL;
  node = table->storage.chain.buckets[index];

  while (node != NULL) {
    if (node->key.length == key.length &&
        table->config.equal(node->key.data, key.data, key.length)) {
      if (prev == NULL) {
        table->storage.chain.buckets[index] = node->next;
      } else {
        prev->next = node->next;
      }

      if (table->config.destroy_key != NULL) {
        table->config.destroy_key(node->key.data);
      }

      free(node);
      table->storage.chain.size--;

      return true;
    }

    prev = node;
    node = node->next;
  }

  return false;
}

void *cashash_chain_find(const cashash_t *table,
                         const cashash_key_datum_t key) {
  size_t hash;
  size_t index;
  cashash_node_t *node;

  if (table == NULL || key.data == NULL ||
      table->storage.chain.buckets == NULL || table->config.hash == NULL ||
      table->config.equal == NULL) {
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

  index = hash % table->storage.chain.bucket_count;

  node = table->storage.chain.buckets[index];
  while (node != NULL) {
    if (node->key.length == key.length &&
        table->config.equal(node->key.data, key.data, key.length)) {
      return node->value;
    }

    node = node->next;
  }

  return NULL;
}

void cashash_chain_clear(cashash_t *table) {
  size_t i;
  cashash_node_t *node;
  cashash_node_t *next;

  if (table == NULL || table->storage.chain.buckets == NULL) {
    return;
  }

  for (i = 0; i < table->storage.chain.bucket_count; i++) {
    node = table->storage.chain.buckets[i];

    while (node != NULL) {
      next = node->next;

      if (table->config.destroy_key != NULL) {
        table->config.destroy_key(node->key.data);
      }

      free(node);
      node = next;
    }

    table->storage.chain.buckets[i] = NULL;
  }

  table->storage.chain.size = 0;
}

void cashash_chain_destroy(cashash_t *table) {
  if (table == NULL)
    return;

  cashash_chain_clear(table);
  free(table->storage.chain.buckets);
  free(table);
}
