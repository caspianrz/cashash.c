#include <cashash.c/cashash_oa.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CASHASH_OA_MAX_LOAD_PERCENT 70

typedef struct cashash_oa_slot_result_s {
  bool found;
  size_t index;
} cashash_oa_slot_result_t;

static bool cashash_oa_key_equals(const cashash_t *table,
                                  const cashash_oa_entry_t *entry,
                                  const cashash_key_datum_t key, size_t hash) {
  if (entry->state != CASHASH_OA_ENTRY_OCCUPIED) {
    return false;
  }

  if (entry->hash != hash) {
    return false;
  }

  if (entry->key.length != key.length) {
    return false;
  }

  return table->config.equal(entry->key.data, key.data, key.length);
}

static size_t cashash_oa_default_second_hash(size_t hash, size_t bucket_count) {
  if (bucket_count <= 1) {
    return 1;
  }

  return 1 + (hash % (bucket_count - 1));
}

static size_t cashash_oa_probe_step(const cashash_t *table,
                                    const cashash_key_datum_t key,
                                    size_t hash) {
  const cashash_oa_table_t *oa = &table->storage.oa;
  size_t step = 1;

  if (oa->probe_strategy != CASHASH_OA_PROBE_DOUBLE_HASHING) {
    return 1;
  }

  if (oa->second_hash != NULL) {
    step = oa->second_hash(key.data, key.length, &table->option);
    step %= oa->bucket_count;

    if (step == 0) {
      step = 1;
    }

    return step;
  }

  return cashash_oa_default_second_hash(hash, oa->bucket_count);
}

static size_t cashash_oa_probe_index(const cashash_t *table, size_t hash,
                                     size_t step, size_t attempt) {
  const size_t bucket_count = table->storage.oa.bucket_count;

  switch (table->storage.oa.probe_strategy) {
  case CASHASH_OA_PROBE_LINEAR:
    return (hash + attempt) % bucket_count;

  case CASHASH_OA_PROBE_QUADRATIC:
    return (hash + attempt * attempt) % bucket_count;

  case CASHASH_OA_PROBE_DOUBLE_HASHING:
    return (hash + attempt * step) % bucket_count;
  }

  return (hash + attempt) % bucket_count;
}

static cashash_oa_slot_result_t
cashash_oa_find_slot(const cashash_t *table, const cashash_key_datum_t key,
                     size_t hash, bool for_insert) {
  const cashash_oa_table_t *oa = &table->storage.oa;
  size_t first_deleted = SIZE_MAX;
  size_t step = cashash_oa_probe_step(table, key, hash);

  for (size_t attempt = 0; attempt < oa->bucket_count; attempt++) {
    size_t index = cashash_oa_probe_index(table, hash, step, attempt);
    const cashash_oa_entry_t *entry = &oa->entries[index];

    if (entry->state == CASHASH_OA_ENTRY_EMPTY) {
      if (for_insert && first_deleted != SIZE_MAX) {
        return (cashash_oa_slot_result_t){
            .found = false,
            .index = first_deleted,
        };
      }

      return (cashash_oa_slot_result_t){
          .found = false,
          .index = index,
      };
    }

    if (entry->state == CASHASH_OA_ENTRY_DELETED) {
      if (for_insert && first_deleted == SIZE_MAX) {
        first_deleted = index;
      }

      continue;
    }

    if (cashash_oa_key_equals(table, entry, key, hash)) {
      return (cashash_oa_slot_result_t){
          .found = true,
          .index = index,
      };
    }
  }

  if (for_insert && first_deleted != SIZE_MAX) {
    return (cashash_oa_slot_result_t){
        .found = false,
        .index = first_deleted,
    };
  }

  return (cashash_oa_slot_result_t){
      .found = false,
      .index = SIZE_MAX,
  };
}

static bool cashash_oa_insert_existing_entry(cashash_t *table,
                                             cashash_oa_entry_t old_entry) {
  cashash_oa_table_t *oa = &table->storage.oa;

  cashash_oa_slot_result_t slot =
      cashash_oa_find_slot(table, old_entry.key, old_entry.hash, true);

  if (slot.index == SIZE_MAX) {
    return false;
  }

  oa->entries[slot.index] = old_entry;
  oa->entries[slot.index].state = CASHASH_OA_ENTRY_OCCUPIED;
  oa->size++;

  return true;
}

static bool cashash_oa_resize(cashash_t *table, size_t new_bucket_count) {
  cashash_oa_table_t *oa = &table->storage.oa;

  cashash_oa_entry_t *old_entries = oa->entries;
  size_t old_bucket_count = oa->bucket_count;

  cashash_oa_entry_t *new_entries =
      calloc(new_bucket_count, sizeof *new_entries);

  if (new_entries == NULL) {
    return false;
  }

  oa->entries = new_entries;
  oa->bucket_count = new_bucket_count;
  oa->size = 0;
  oa->deleted_count = 0;

  for (size_t i = 0; i < old_bucket_count; i++) {
    if (old_entries[i].state != CASHASH_OA_ENTRY_OCCUPIED) {
      continue;
    }

    if (!cashash_oa_insert_existing_entry(table, old_entries[i])) {
      free(new_entries);

      oa->entries = old_entries;
      oa->bucket_count = old_bucket_count;

      /*
       * Recompute size/deleted_count conservatively after rollback.
       */
      oa->size = 0;
      oa->deleted_count = 0;

      for (size_t j = 0; j < old_bucket_count; j++) {
        if (old_entries[j].state == CASHASH_OA_ENTRY_OCCUPIED) {
          oa->size++;
        } else if (old_entries[j].state == CASHASH_OA_ENTRY_DELETED) {
          oa->deleted_count++;
        }
      }

      return false;
    }
  }

  free(old_entries);

  return true;
}

static bool cashash_oa_should_grow(const cashash_t *table) {
  const cashash_oa_table_t *oa = &table->storage.oa;

  if (oa->bucket_count == 0) {
    return true;
  }

  return ((oa->size + oa->deleted_count + 1) * 100 >=
          oa->bucket_count * CASHASH_OA_MAX_LOAD_PERCENT);
}

bool cashash_oa_insert(cashash_t *table, const cashash_key_datum_t key,
                       void *value) {
  if (table == NULL || table->kind != CASHASH_TABLE_OPEN_ADDRESSING) {
    return false;
  }

  if (key.data == NULL || key.length == 0) {
    return false;
  }

  cashash_oa_table_t *oa = &table->storage.oa;

  if (cashash_oa_should_grow(table)) {
    size_t new_bucket_count = oa->bucket_count * 2;

    if (new_bucket_count == 0) {
      new_bucket_count = 8;
    }

    if (!cashash_oa_resize(table, new_bucket_count)) {
      return false;
    }
  }

  size_t hash = table->config.hash(key.data, key.length, &table->option);

  cashash_oa_slot_result_t slot = cashash_oa_find_slot(table, key, hash, true);

  if (slot.index == SIZE_MAX) {
    return false;
  }

  cashash_oa_entry_t *entry = &oa->entries[slot.index];

  if (slot.found) {
    entry->value = value;
    return true;
  }

  void *copied_key = table->config.copy_key(key.data, key.length);

  if (copied_key == NULL) {
    return false;
  }

  if (entry->state == CASHASH_OA_ENTRY_DELETED) {
    oa->deleted_count--;
  }

  entry->state = CASHASH_OA_ENTRY_OCCUPIED;
  entry->hash = hash;
  entry->key = (cashash_key_datum_t){
      .data = copied_key,
      .length = key.length,
  };
  entry->value = value;

  oa->size++;

  return true;
}

void *cashash_oa_find(const cashash_t *table, const cashash_key_datum_t key) {
  if (table == NULL || table->kind != CASHASH_TABLE_OPEN_ADDRESSING) {
    return NULL;
  }

  if (key.data == NULL || key.length == 0) {
    return NULL;
  }

  const cashash_oa_table_t *oa = &table->storage.oa;

  if (oa->entries == NULL || oa->bucket_count == 0) {
    return NULL;
  }

  size_t hash = table->config.hash(key.data, key.length, &table->option);

  cashash_oa_slot_result_t slot = cashash_oa_find_slot(table, key, hash, false);

  if (!slot.found) {
    return NULL;
  }

  return oa->entries[slot.index].value;
}

bool cashash_oa_remove(cashash_t *table, const cashash_key_datum_t key) {
  if (table == NULL || table->kind != CASHASH_TABLE_OPEN_ADDRESSING) {
    return false;
  }

  if (key.data == NULL || key.length == 0) {
    return false;
  }

  cashash_oa_table_t *oa = &table->storage.oa;

  if (oa->entries == NULL || oa->bucket_count == 0) {
    return false;
  }

  size_t hash = table->config.hash(key.data, key.length, &table->option);

  cashash_oa_slot_result_t slot = cashash_oa_find_slot(table, key, hash, false);

  if (!slot.found) {
    return false;
  }

  cashash_oa_entry_t *entry = &oa->entries[slot.index];

  table->config.destroy_key(entry->key.data);

  entry->state = CASHASH_OA_ENTRY_DELETED;
  entry->hash = 0;
  entry->key = (cashash_key_datum_t){0};
  entry->value = NULL;

  oa->size--;
  oa->deleted_count++;

  return true;
}

void cashash_oa_clear(cashash_t *table) {
  if (table == NULL || table->kind != CASHASH_TABLE_OPEN_ADDRESSING) {
    return;
  }

  cashash_oa_table_t *oa = &table->storage.oa;

  if (oa->entries == NULL) {
    return;
  }

  for (size_t i = 0; i < oa->bucket_count; i++) {
    cashash_oa_entry_t *entry = &oa->entries[i];

    if (entry->state == CASHASH_OA_ENTRY_OCCUPIED) {
      table->config.destroy_key(entry->key.data);
    }

    entry->state = CASHASH_OA_ENTRY_EMPTY;
    entry->hash = 0;
    entry->key = (cashash_key_datum_t){0};
    entry->value = NULL;
  }

  oa->size = 0;
  oa->deleted_count = 0;
}

void cashash_oa_destroy(cashash_t *table) {
  if (table == NULL) {
    return;
  }

  if (table->kind != CASHASH_TABLE_OPEN_ADDRESSING) {
    return;
  }

  cashash_oa_clear(table);

  free(table->storage.oa.entries);
  table->storage.oa.entries = NULL;
  table->storage.oa.bucket_count = 0;

  free(table);
}
