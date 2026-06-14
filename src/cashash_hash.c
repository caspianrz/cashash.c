#include "cashash.c/cashash_hash.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#ifdef CASHASH_USE_XXHASH
#include <xxhash.h>
#endif

size_t cashash_hash_fnv1a_string(const char *key) {
  uint64_t hash = 14695981039346656037ull;

  while (*key != '\0') {
    hash ^= (unsigned char)*key;
    hash *= 1099511628211ull;
    key++;
  }

  return (size_t)hash;
}

size_t cashash_hash_fnv1a_bytes(const void *data, size_t len, ...) {
  const unsigned char *bytes = data;
  uint64_t hash = 14695981039346656037ull;

  for (size_t i = 0; i < len; i++) {
    hash ^= bytes[i];
    hash *= 1099511628211ull;
  }

  return (size_t)hash;
}

bool cashash_equal_fnv1a_bytes(const void *a, const void *b, const size_t len) {
  return strncmp((const char *)a, (const char *)b, len) == 0;
}

void *cashash_copy_fnv1a_bytes(const void *key, const size_t len) {
  void *copy;

  if (key == NULL) {
    return NULL;
  }

  copy = malloc(len);
  if (copy == NULL) {
    return NULL;
  }

  memcpy(copy, key, len);

  return copy;
}

void cashash_key_destroy_fnv1a_bytes(const void *key) { free((void *)key); }

#ifdef CASHASH_USE_XXHASH
size_t cashash_hash_xxh3_bytes(const void *data, size_t len, ...) {
  return (size_t)XXH3_64bits(data, len);
}

size_t cashash_hash_xxh64_bytes(const void *data, size_t len, ...) {
  va_list args;
  uint64_t seed;

  va_start(args, len);
  seed = va_arg(args, uint64_t);
  va_end(args);

  return (size_t)XXH64(data, len, seed);
}
#endif
