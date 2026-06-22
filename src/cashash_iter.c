#include <cashash.c/cashash_iter.h>

static void cashash_iter_advance_chain(cashash_iter_t *iter) {
  if (iter == NULL || iter->table == NULL) {
    return;
  }

  if (iter->current != NULL && iter->current->next != NULL) {
    iter->current = iter->current->next;
    return;
  }

  iter->current = NULL;

  for (size_t i = iter->bucket_index + 1;
       i < iter->table->storage.chain.bucket_count; i++) {
    if (iter->table->storage.chain.buckets[i] != NULL) {
      iter->bucket_index = i;
      iter->current = iter->table->storage.chain.buckets[i];
      return;
    }
  }

  iter->bucket_index = iter->table->storage.chain.bucket_count;
}

static void cashash_iter_advance_oa(cashash_iter_t *iter) {
  if (iter == NULL || iter->table == NULL) {
    return;
  }

  for (size_t i = iter->bucket_index + 1;
       i < iter->table->storage.oa.bucket_count; i++) {
    if (iter->table->storage.oa.entries[i].state == CASHASH_OA_ENTRY_OCCUPIED) {
      iter->bucket_index = i;
      return;
    }
  }

  iter->bucket_index = iter->table->storage.oa.bucket_count;
}

void cashash_iter_init(cashash_t *table, cashash_iter_t *iter) {
  if (iter == NULL) {
    return;
  }

  iter->table = table;
  iter->bucket_index = 0;
  iter->current = NULL;

  if (table == NULL) {
    return;
  }

  switch (table->kind) {
  case CASHASH_TABLE_CHAINING:
    if (table->storage.chain.buckets == NULL ||
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

    iter->bucket_index = table->storage.chain.bucket_count;
    return;

  case CASHASH_TABLE_OPEN_ADDRESSING:
    if (table->storage.oa.entries == NULL ||
        table->storage.oa.bucket_count == 0) {
      return;
    }

    for (size_t i = 0; i < table->storage.oa.bucket_count; i++) {
      if (table->storage.oa.entries[i].state == CASHASH_OA_ENTRY_OCCUPIED) {
        iter->bucket_index = i;
        return;
      }
    }

    iter->bucket_index = table->storage.oa.bucket_count;
    return;
  }
}

bool cashash_iter_has_next(cashash_iter_t *iter) {
  if (iter == NULL || iter->table == NULL) {
    return false;
  }

  switch (iter->table->kind) {
  case CASHASH_TABLE_CHAINING:
    return iter->current != NULL;

  case CASHASH_TABLE_OPEN_ADDRESSING:
    if (iter->table->storage.oa.entries == NULL) {
      return false;
    }

    if (iter->bucket_index >= iter->table->storage.oa.bucket_count) {
      return false;
    }

    return iter->table->storage.oa.entries[iter->bucket_index].state ==
           CASHASH_OA_ENTRY_OCCUPIED;
  }

  return false;
}

bool cashash_iter_next(cashash_iter_t *iter, cashash_pair_t *pair) {
  if (iter == NULL || pair == NULL) {
    return false;
  }

  if (!cashash_iter_has_next(iter)) {
    return false;
  }

  switch (iter->table->kind) {
  case CASHASH_TABLE_CHAINING: {
    cashash_node_t *node = iter->current;

    pair->key = node->key;
    pair->value = node->value;

    cashash_iter_advance_chain(iter);

    return true;
  }

  case CASHASH_TABLE_OPEN_ADDRESSING: {
    cashash_oa_entry_t *entry =
        &iter->table->storage.oa.entries[iter->bucket_index];

    pair->key = entry->key;
    pair->value = entry->value;

    cashash_iter_advance_oa(iter);

    return true;
  }
  }

  return false;
}

bool cashash_iter_next_key(cashash_iter_t *iter, cashash_key_datum_t *out_key) {
  if (out_key == NULL) {
    return false;
  }

  cashash_pair_t pair;

  if (!cashash_iter_next(iter, &pair)) {
    return false;
  }

  *out_key = pair.key;

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
  if (table == NULL || callback == NULL) {
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
