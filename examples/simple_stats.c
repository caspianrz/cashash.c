#include <cashash.c/cashash.h>
#include <cashash.c/cashash_diagnostics.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATUM(value)                                                           \
  ((cashash_key_datum_t){.data = (value), .length = sizeof(value) - 1})

int main(void) {
  cashash_t *map = cashash_create(10);

  char **keys = malloc(1000 * sizeof(*keys));
  int **values = malloc(1000 * sizeof(*values));

  for (int i = 0; i < 1000; ++i) {
    keys[i] = malloc(16);
    snprintf(keys[i], 16, "%d", i);

    values[i] = malloc(sizeof(int));
    *values[i] = i;

    cashash_insert(
        map, (cashash_key_datum_t){.data = keys[i], .length = strlen(keys[i])},
        values[i]);
  }

  cashash_stats_t stats;
  cashash_stats(map, &stats);

  cashash_stats_print(stdout, &stats);

  cashash_destroy(map);

  printf("--------------------\n");
  map = cashash_create_open_addressing(10);

  for (int i = 0; i < 1000; ++i) {
    cashash_insert(
        map, (cashash_key_datum_t){.data = keys[i], .length = strlen(keys[i])},
        values[i]);
  }

  cashash_stats(map, &stats);
  cashash_stats_print(stdout, &stats);

  for (int i = 0; i < 1000; ++i) {
    free(values[i]);
    free(keys[i]);
  }

  free(values);
  free(keys);

  cashash_destroy(map);

  return 0;
}
