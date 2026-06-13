#include <cashash.c/cashash.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CASHASH_MAX_LOAD_NUMERATOR 3
#define CASHASH_MAX_LOAD_DENOMINATOR 4
#define CASHASH_GROWTH_FACTOR 2

typedef struct cashash_node {
  char *key;
  void *value;
  struct cashash_node *next;
} cashash_node_t;

struct cashash_s {
  cashash_node_t **buckets;
  size_t bucket_count;
  size_t size;
};

static char *cashash_strdup(const char *s) {
  size_t len = strlen(s) + 1;

  char *copy = malloc(len);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, s, len);
  return copy;
}

static uint64_t cashash_fnv1a(const char *key) {
  uint64_t hash = 14695981039346656037ull;

  while (*key != '\0') {
    hash ^= (unsigned char)*key;
    hash *= 1099511628211ull;
    key++;
  }

  return hash;
}

static size_t cashash_bucket_index(const char *key, size_t bucket_count) {
  return (size_t)(cashash_fnv1a(key) % bucket_count);
}

static bool cashash_should_grow(const cashash_t *table) {
  size_t next_size;

  if (table == NULL || table->bucket_count == 0) {
    return false;
  }

  next_size = table->size + 1;

  if (next_size < table->size) {
    return true;
  }

  return next_size > ((table->bucket_count * CASHASH_MAX_LOAD_NUMERATOR) /
                      CASHASH_MAX_LOAD_DENOMINATOR);
}

static bool cashash_resize(cashash_t *table, size_t new_bucket_count) {
  cashash_node_t **new_buckets;

  if (table == NULL || new_bucket_count == 0) {
    return false;
  }

  new_buckets = calloc(new_bucket_count, sizeof(cashash_node_t *));
  if (new_buckets == NULL) {
    return false;
  }

  for (size_t i = 0; i < table->bucket_count; i++) {
    cashash_node_t *node = table->buckets[i];

    while (node != NULL) {
      cashash_node_t *next = node->next;
      size_t new_index = cashash_bucket_index(node->key, new_bucket_count);

      node->next = new_buckets[new_index];
      new_buckets[new_index] = node;

      node = next;
    }
  }

  free(table->buckets);

  table->buckets = new_buckets;
  table->bucket_count = new_bucket_count;

  return true;
}

static bool cashash_grow(cashash_t *table) {
  size_t new_bucket_count;

  if (table == NULL) {
    return false;
  }

  if (table->bucket_count > ((size_t)-1) / CASHASH_GROWTH_FACTOR) {
    return false;
  }

  new_bucket_count = table->bucket_count * CASHASH_GROWTH_FACTOR;

  return cashash_resize(table, new_bucket_count);
}

cashash_t *cashash_create(size_t bucket_count) {
  cashash_t *table;

  if (bucket_count == 0) {
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

  return table;
}

void cashash_destroy(cashash_t *table) {
  if (table == NULL) {
    return;
  }

  for (size_t i = 0; i < table->bucket_count; i++) {
    cashash_node_t *node = table->buckets[i];

    while (node != NULL) {
      cashash_node_t *next = node->next;

      free(node->key);
      free(node);

      node = next;
    }
  }

  free(table->buckets);
  free(table);
}

bool cashash_insert(cashash_t *table, const char *key, void *value) {
  size_t index;
  cashash_node_t *node;
  cashash_node_t *new_node;

  if (table == NULL || key == NULL) {
    return false;
  }

  // First check whether the key already exists.
  // Updating an existing key should not change size and should not trigger
  // resizing.
  index = cashash_bucket_index(key, table->bucket_count);
  node = table->buckets[index];

  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      node->value = value;
      return true;
    }

    node = node->next;
  }

  // This is a new key. Grow before inserting if the next insertion would
  // exceed the max load factor.
  // If growth fails, the table is left unchanged.

  if (cashash_should_grow(table)) {
    if (!cashash_grow(table)) {
      return false;
    }

    index = cashash_bucket_index(key, table->bucket_count);
  }

  new_node = malloc(sizeof(cashash_node_t));
  if (new_node == NULL) {
    return false;
  }

  new_node->key = cashash_strdup(key);
  if (new_node->key == NULL) {
    free(new_node);
    return false;
  }

  new_node->value = value;
  new_node->next = table->buckets[index];

  table->buckets[index] = new_node;
  table->size++;

  return true;
}

void *cashash_find(const cashash_t *table, const char *key) {
  if (table == NULL || key == NULL) {
    return NULL;
  }

  uint64_t hash = cashash_fnv1a(key);
  size_t index = hash % table->bucket_count;

  cashash_node_t *node = table->buckets[index];

  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
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
