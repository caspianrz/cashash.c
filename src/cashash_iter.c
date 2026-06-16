#include <cashash.c/cashash_iter.h>

void cashash_iter_init(cashash_t *table, cashash_iter_t *iter) {
  if (iter == NULL) {
    return;
  }

  iter->table = table;
  iter->bucket_index = 0;
  iter->current = NULL;

  if (table == NULL || table->buckets == NULL || table->bucket_count == 0) {
    return;
  }

  for (size_t i = 0; i < table->bucket_count; i++) {
    if (table->buckets[i] != NULL) {
      iter->bucket_index = i;
      iter->current = table->buckets[i];
      return;
    }
  }
}

bool cashash_iter_has_next(cashash_iter_t *iter) {
  return iter != NULL && iter->current != NULL;
}

bool cashash_iter_next(cashash_iter_t *iter, cashash_pair_t *pair) {
  if (iter == NULL) {
    return false;
  }

  if (iter->table == NULL || iter->current == NULL) {
    return false;
  }

  cashash_node_t *node = iter->current;

  pair->key = node->key;
  pair->value = node->value;

  if (node->next != NULL) {
    iter->current = node->next;
    return true;
  }

  iter->current = NULL;

  for (size_t i = iter->bucket_index + 1; i < iter->table->bucket_count; i++) {
    if (iter->table->buckets[i] != NULL) {
      iter->bucket_index = i;
      iter->current = iter->table->buckets[i];
      break;
    }
  }

  return true;
}
