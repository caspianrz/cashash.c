#include <cashash.c/cashash.h>

#include <stdio.h>

#define DATUM(kind, value)                                                     \
  ((cashash_##kind##_datum_t){.data = (value), .length = sizeof(value) - 1})

int main(void) {
  cashash_t *map = cashash_create(128);

  if (map == NULL) {
    return 1;
  }

  cashash_insert(map, DATUM(key, "name"), (void *)"CasHash");
  cashash_insert(map, DATUM(key, "language"), (void *)"C");

  printf("size: %zu\n", cashash_size(map));

  char *name = cashash_find(map, DATUM(key, "name"));
  if (name != NULL) {
    printf("name: %s\n", name);
  }

  char *language = cashash_find(map, DATUM(key, "language"));

  if (language != NULL) {
    printf("language: %s\n", language);
  }

  cashash_destroy(map);

  return 0;
}
