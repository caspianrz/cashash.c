#include <cashash.c/cashash_iter.h>

void cashash_iter_init(cashash_t *table, cashash_iter_t *iter) {
  if (iter == NULL) {
    return;
  }

  iter->table = table;
  iter->bucket_index = 0;
  iter->current = NULL;

  if (iter->table->kind == CASHASH_TABLE_CHAINING) {

    if (table == NULL || table->storage.chain.buckets == NULL ||
        table->storage.chain.bucket_count == 0) {
      return;
    }

    for (size_t i = 0; i < table->storage.chain.bucket_count; i++) {
      if (table->storage.chain.buckets[i] != NULL) {
        iter->bucket_index = i;
        iter->current = table->storage.chain.buckets[i];
        return;
      }
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

  if (iter->table == NULL) {
    return false;
  }

  if (iter->table->kind == CASHASH_TABLE_CHAINING) {
    if (iter->current == NULL) {
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

    for (size_t i = iter->bucket_index + 1;
         i < iter->table->storage.chain.bucket_count; i++) {
      if (iter->table->storage.chain.buckets[i] != NULL) {
        iter->bucket_index = i;
        iter->current = iter->table->storage.chain.buckets[i];
        break;
      }
    }

    return true;
  } else {
    return false;
  }
}

bool cashash_iter_next_key(cashash_iter_t *iter, cashash_key_datum_t *out_key) {
  if (iter == NULL) {
    return false;
  }

  if (iter->table == NULL || iter->current == NULL) {
    return false;
  }

  cashash_node_t *node = iter->current;

  out_key->data = node->key.data;
  out_key->length = node->key.length;

  if (node->next != NULL) {
    iter->current = node->next;
    return true;
  }

  iter->current = NULL;

  for (size_t i = iter->bucket_index + 1;
       i < iter->table->storage.chain.bucket_count; i++) {
    if (iter->table->storage.chain.buckets[i] != NULL) {
      iter->bucket_index = i;
      iter->current = iter->table->storage.chain.buckets[i];
      break;
    }
  }

  return true;
}

bool cashash_foreach(cashash_t *table, cashash_foreach_fn callback,
                     void *user_data) {
  if (table == NULL || callback == NULL) {
    return false;
  }

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(table, &iter);

  while (cashash_iter_has_next(&iter)) {
    if (!cashash_iter_next(&iter, &pair)) {
      return false;
    }

    if (!callback(pair, user_data)) {
      return false;
    }
  }

  return true;
}

bool cashash_foreach_key(cashash_t *table, cashash_key_foreach_fn callback,
                         void *user_data) {
  if (table == NULL | callback == NULL) {
    return false;
  }

  cashash_iter_t iter;
  cashash_key_datum_t key;

  cashash_iter_init(table, &iter);

  while (cashash_iter_has_next(&iter)) {
    if (!cashash_iter_next_key(&iter, &key)) {
      return false;
    }

    if (!callback(key, user_data)) {
      return false;
    }
  }

  return true;
}
