#include <cashash.c/cashash.h>
#include <cashash.c/cashash_iter.h>

#include <stdio.h>
#include <stdlib.h>

static const int keys[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static int values[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

void populate_map(cashash_t *table) {
  for (int key = 0; key < 10; key++) {
    const cashash_key_datum_t k = {
        .data = keys + key,
        .length = sizeof(int),
    };
    void *v = values + key;
    cashash_insert(table, k, v);
  }
}

/** callback function for foreach. */
static bool add_value_to_even_keys(cashash_pair_t pair, void *value) {
  int *v = value;

  const int *key_value = pair.key.data;
  int *pair_value = pair.value;
  if (*key_value % 2 == 0) {
    *pair_value = (*pair_value) + *v;
  }

  return true;
}

int main(void) {
  cashash_t *map = cashash_create(128);

  if (map == NULL) {
    return 1;
  }

  populate_map(map);

  int callback_data = 3;
  cashash_foreach(map, add_value_to_even_keys, &callback_data);

  cashash_iter_t iterator;
  cashash_iter_init(map, &iterator);
  cashash_pair_t pair;

  while (cashash_iter_has_next(&iterator)) {
    if (!cashash_iter_next(&iterator, &pair)) {
      break;
    }

    int key = *(int *)pair.key.data;
    int value = *(int *)pair.value;
    printf("Key: %d, Value: %d\n", key, value);
  }

  cashash_destroy(map);
  return 0;
}
