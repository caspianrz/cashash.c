#include <cashash.c/cashash.h>

#include <stdio.h>

#define CKEY(key) (key), (sizeof(key) - 1)

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

  cashash_insert(map, CKEY("name"), "cashash");
  cashash_insert(map, CKEY("language"), "C");

  printf("size: %zu\n", cashash_size(map));

  char *name = cashash_find(map, CKEY("name"));

  if (name != NULL) {
    printf("name: %s\n", name);
  }

  char *language = cashash_find(map, CKEY("language"));

  if (language != NULL) {
    printf("language: %s\n", language);
  }

  cashash_destroy(map);

  return 0;
}
