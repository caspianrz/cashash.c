#include <cashash.c/cashash_oa.h>

bool cashash_oa_insert(cashash_t *table, const cashash_key_datum_t key,
                       void *data) {
  (void)table;
  (void)key;
  (void)data;

  return false;
}

bool cashash_oa_remove(cashash_t *table, const cashash_key_datum_t key) {
  (void)table;
  (void)key;
  return false;
}

void *cashash_oa_find(const cashash_t *table, const cashash_key_datum_t key) {
  (void)table;
  (void)key;
  return NULL;
}

void cashash_oa_clear(cashash_t *table) { (void)table; }
