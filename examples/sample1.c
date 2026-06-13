#include <cashash.c/cashash.h>

#include <stdio.h>

int main(void) {
  cashash_t *map = cashash_create(128);

  if (map == NULL) {
    return 1;
  }

  cashash_insert(map, "name", "cashash");
  cashash_insert(map, "language", "C");

  printf("size: %zu\n", cashash_size(map));

  char *name = cashash_find(map, "name");

  if (name != NULL) {
    printf("name: %s\n", name);
  }

  char *language = cashash_find(map, "language");

  if (language != NULL) {
    printf("language: %s\n", language);
  }

  cashash_destroy(map);

  return 0;
}
