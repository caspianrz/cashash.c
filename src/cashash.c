#include <cashash.c/cashash.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

cashash_t *cashash_create(size_t bucket_count) {
  if (bucket_count == 0) {
    return NULL;
  }

  cashash_t *table = malloc(sizeof(cashash_t));
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
  if (table == NULL || key == NULL) {
    return false;
  }

  uint64_t hash = cashash_fnv1a(key);
  size_t index = hash % table->bucket_count;

  cashash_node_t *node = table->buckets[index];

  while (node != NULL) {
    if (strcmp(node->key, key) == 0) {
      node->value = value;
      return true;
    }

    node = node->next;
  }

  cashash_node_t *new_node = malloc(sizeof(cashash_node_t));
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
