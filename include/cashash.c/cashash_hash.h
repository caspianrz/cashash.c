#pragma once

#ifndef CASHASH_HASH_H
#define CASHASH_HASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file cashash.c/cashash_hash.h
 * @brief Hashing function declarations for cashash.c.
 *
 * This header provides hash functions that can be used with `cashash.c`.
 *
 * The functions in this file are intended to support different key types as
 * the library moves toward generic-key hash maps. Each hash function accepts a
 * key pointer and returns a `size_t` hash value that can be mapped into the
 * table's bucket array.
 *
 * These functions do not allocate memory, do not take ownership of keys, and
 * do not modify the input data.
 *
 * @note The returned hash value is not a bucket index by itself. The hash table
 * maps it to a bucket using the current bucket count.
 *
 * @warning These hash functions are intended for hash table distribution, not
 * for cryptographic use.
 */

size_t cashash_hash_fnv1a_string(const char *key);

size_t cashash_hash_fnv1a_bytes(const void *data, const size_t len, ...);
bool cashash_equal_fnv1a_bytes(const void *a, const void *b, const size_t len);
void *cashash_copy_fnv1a_bytes(const void *key, const size_t len);
void cashash_key_destroy_fnv1a_bytes(const void *key);

#ifdef CASHASH_USE_XXHASH
size_t cashash_hash_xxh3_bytes(const void *data, size_t len, ...);
size_t cashash_hash_xxh64_bytes(const void *data, size_t len, ...);
#endif

#endif
