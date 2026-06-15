#include <cashash.c/cashash.h>

#include <stdio.h>

#define DATUM(kind, value)                                                     \
  ((cashash_##kind##_datum_t){.data = (value), .length = sizeof(value) - 1})

/** We use xxHash in this. **/
int main(void) {
  cashash_t *map = cashash_create(128);
  cashash_strategy_option_t option = {
      .used = false,
  };
  cashash_create_with_strategy(128, CASHASH_HASH_STRATEGY_XXH3, option);

  if (map == NULL) {
    return 1;
  }

  cashash_insert(map, DATUM(key, "name"), DATUM(value, "cashash"));
  cashash_insert(map, DATUM(key, "language"), DATUM(value, "C"));

  printf("size: %zu\n", cashash_size(map));

  cashash_value_datum_t result;
  bool name_found = cashash_find(map, DATUM(key, "name"), &result);

  if (name_found) {
    printf("name: %s\n", (char *)result.data);
  }

  bool language_found = cashash_find(map, DATUM(key, "language"), &result);

  if (language_found) {
    printf("language: %s\n", (char *)result.data);
  }

  cashash_destroy(map);

  return 0;
}
