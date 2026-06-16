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
    const cashash_value_datum_t v = {
        .data = values + key,
        .length = sizeof(int),
    };
    cashash_insert(table, k, v);
  }
}

int main(void) {
  cashash_t *map = cashash_create(128);

  if (map == NULL) {
    return 1;
  }

  populate_map(map);

  cashash_iter_t iterator;

  cashash_iter_init(map, &iterator);
  cashash_pair_t pair;

  while (cashash_iter_has_next(&iterator)) {
    if (!cashash_iter_next(&iterator, &pair)) {
      break;
    }

    int key = *(int *)pair.key.data;
    int value = *(int *)pair.value.data;
    printf("Key: %d, Value: %d\n", key, value);
  }

  cashash_destroy(map);
  return 0;
}
