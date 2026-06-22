#pragma once

#ifndef CASHASH_HASH_H
#define CASHASH_HASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @file cashash.c/cashash_hash.h
 * @brief Hash, equality, copy, and destroy helpers for cashash.c.
 *
 * This header provides the default helper functions used by the cashash
 * generic byte-key hash table.
 *
 * The byte-based helpers operate on explicit pointer-plus-length keys. This
 * makes them suitable for strings, binary buffers, integers, structs, and other
 * fixed-size key types.
 *
 * These functions do not take ownership of input key pointers unless otherwise
 * documented. Hash and equality functions never modify input data.
 *
 * @warning These hash functions are intended for hash table distribution only.
 * They are not cryptographic hash functions.
 */

/**
 * @defgroup cashash_hash Cashash Hash Helpers
 * @brief Hashing and key utility functions for cashash.
 *
 * @{
 */

/**
 * @brief Hash function callback type.
 *
 * Computes a hash value for `key_len` bytes starting at `key`.
 *
 * The variadic arguments are reserved for strategy-specific options, such as
 * an xxHash seed.
 *
 * @param key Pointer to key bytes.
 * @param len Number of bytes in the key.
 *
 * @return Hash value for the provided key bytes.
 */
typedef size_t (*cashash_hash_fn)(const void *key, const size_t len, ...);

/**
 * @brief Key equality callback type.
 *
 * Compares exactly `len` bytes from `a` and `b`.
 *
 * Implementations should be binary-safe and should not depend on
 * null-terminated strings.
 *
 * @param a First key pointer.
 * @param b Second key pointer.
 * @param len Number of bytes to compare.
 *
 * @return true if the keys are equal.
 * @return false otherwise.
 */
typedef bool (*cashash_equal_fn)(const void *a, const void *b, size_t len);

/**
 * @brief Key copy callback type.
 *
 * Creates an owned copy of `len` bytes from `key`.
 *
 * The returned pointer is stored internally by the hash table and later passed
 * to the configured cashash_key_destroy_fn.
 *
 * @param key Pointer to key bytes to copy.
 * @param len Number of bytes to copy.
 *
 * @return Pointer to copied key memory on success.
 * @return NULL on allocation failure.
 */
typedef void *(*cashash_key_copy_fn)(const void *key, size_t len);

/**
 * @brief Key destroy callback type.
 *
 * Destroys a key previously returned by cashash_key_copy_fn.
 *
 * @param key Copied key pointer to destroy.
 */
typedef void (*cashash_key_destroy_fn)(const void *key);

/**
 * @brief Hash a null-terminated string using FNV-1a.
 *
 * This function hashes bytes from `key` until the first `'\0'` byte.
 *
 * For generic byte keys or strings with explicit lengths, use
 * cashash_hash_fnv1a_bytes() instead.
 *
 * @param key Null-terminated string to hash.
 *
 * @return FNV-1a hash value.
 *
 * @warning This function is string-based and stops at the first `'\0'`.
 * It is not suitable for binary keys containing embedded null bytes.
 */
size_t cashash_hash_fnv1a_string(const char *key);

/**
 * @brief Hash raw bytes using FNV-1a.
 *
 * Computes an FNV-1a hash over exactly `len` bytes starting at `data`.
 *
 * This function is binary-safe. The input may contain embedded `'\0'` bytes.
 *
 * @param data Pointer to the bytes to hash.
 * @param len Number of bytes to hash.
 * @param ... Unused. Present for compatibility with cashash_hash_fn.
 *
 * @return FNV-1a hash value.
 *
 * @note The returned value is a hash, not a bucket index. The hash table maps
 * it to a bucket using the current bucket count.
 */
size_t cashash_hash_fnv1a_bytes(const void *data, const size_t len, ...);

/**
 * @brief Compare two byte keys for equality.
 *
 * Compares exactly `len` bytes from `a` and `b`.
 *
 * This function is binary-safe and does not stop at embedded `'\0'` bytes.
 *
 * @param a First byte sequence.
 * @param b Second byte sequence.
 * @param len Number of bytes to compare.
 *
 * @return true if the byte sequences are equal.
 * @return false if they differ, or if either pointer is NULL.
 *
 * @note This is intended to be used as the default cashash_equal_fn callback.
 */
bool cashash_equal_bytes(const void *a, const void *b, const size_t len);

/**
 * @brief Copy a byte key.
 *
 * Allocates a new buffer and copies exactly `len` bytes from `key`.
 *
 * The returned pointer is owned by the caller. When used by cashash.c, the
 * table owns this copied key and later destroys it using
 * cashash_key_destroy_fnv1a_bytes().
 *
 * @param key Pointer to the key bytes to copy.
 * @param len Number of bytes to copy.
 *
 * @return Pointer to the copied key bytes on success.
 * @return NULL if `key` is NULL or allocation fails.
 *
 * @warning The copied key is not guaranteed to be null-terminated. It should be
 * treated as raw bytes.
 */
void *cashash_copy_bytes(const void *key, const size_t len);

/**
 * @brief Destroy a copied byte key.
 *
 * Frees a key previously returned by cashash_copy_fnv1a_bytes().
 *
 * @param key Copied key pointer to destroy. Passing NULL is allowed.
 */
void cashash_key_destroy_bytes(const void *key);

#ifdef CASHASH_USE_XXHASH

/**
 * @brief Hash raw bytes using XXH3.
 *
 * Computes an XXH3 hash over exactly `len` bytes starting at `data`.
 *
 * This function is binary-safe. The input may contain embedded `'\0'` bytes.
 *
 * @param data Pointer to the bytes to hash.
 * @param len Number of bytes to hash.
 * @param ... Unused. Present for compatibility with cashash_hash_fn.
 *
 * @return XXH3 hash value cast to size_t.
 *
 * @note Available only when CASHASH_USE_XXHASH is enabled.
 * @warning The returned value may be truncated if `size_t` is smaller than the
 * native XXH3 hash width on the target platform.
 */
size_t cashash_hash_xxh3_bytes(const void *data, size_t len, ...);

/**
 * @brief Hash raw bytes using XXH64.
 *
 * Computes an XXH64 hash over exactly `len` bytes starting at `data`.
 *
 * This function expects a `uint64_t` seed as its variadic argument.
 *
 * Example:
 *
 * @code
 * uint64_t seed = 0;
 * size_t hash = cashash_hash_xxh64_bytes(data, len, seed);
 * @endcode
 *
 * @param data Pointer to the bytes to hash.
 * @param len Number of bytes to hash.
 * @param ... A `uint64_t` seed.
 *
 * @return XXH64 hash value cast to size_t.
 *
 * @note Available only when CASHASH_USE_XXHASH is enabled.
 * @warning The returned value may be truncated if `size_t` is smaller than
 * 64 bits on the target platform.
 */
size_t cashash_hash_xxh64_bytes(const void *data, size_t len, ...);

#endif

/**
 * @}
 */

#endif
