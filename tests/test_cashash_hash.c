#include <cashash.c/cashash_hash.h>

#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

START_TEST(test_fnv1a_string_matches_bytes) {
  const char *key = "cashash";

  size_t string_hash = cashash_hash_fnv1a_string(key);
  size_t bytes_hash = cashash_hash_fnv1a_bytes(key, strlen(key));

  ck_assert(string_hash == bytes_hash);
}
END_TEST

START_TEST(test_fnv1a_empty_string_matches_empty_bytes) {
  const char *key = "";

  size_t string_hash = cashash_hash_fnv1a_string(key);
  size_t bytes_hash = cashash_hash_fnv1a_bytes(key, 0);

  ck_assert(string_hash == bytes_hash);
}
END_TEST

START_TEST(test_fnv1a_hash_is_deterministic) {
  const char data[] = {'a', 'b', 'c', '\0', 'd', 'e', 'f'};

  size_t hash_a = cashash_hash_fnv1a_bytes(data, sizeof(data));
  size_t hash_b = cashash_hash_fnv1a_bytes(data, sizeof(data));

  ck_assert(hash_a == hash_b);
}
END_TEST

START_TEST(test_fnv1a_binary_data_uses_full_length) {
  const char key_a[] = {'a', '\0', 'x'};
  const char key_b[] = {'a', '\0', 'y'};

  size_t hash_a = cashash_hash_fnv1a_bytes(key_a, sizeof(key_a));
  size_t hash_b = cashash_hash_fnv1a_bytes(key_b, sizeof(key_b));

  ck_assert(hash_a != hash_b);
}
END_TEST

START_TEST(test_fnv1a_string_stops_at_null_byte) {
  const char key[] = {'a', '\0', 'x'};

  size_t string_hash = cashash_hash_fnv1a_string(key);
  size_t prefix_hash = cashash_hash_fnv1a_bytes("a", 1);
  size_t full_hash = cashash_hash_fnv1a_bytes(key, sizeof(key));

  ck_assert(string_hash == prefix_hash);
  ck_assert(string_hash != full_hash);
}
END_TEST

START_TEST(test_fnv1a_long_data_is_deterministic) {
  unsigned char data[4096];
  size_t i;

  for (i = 0; i < sizeof(data); i++) {
    data[i] = (unsigned char)(i % 251);
  }

  ck_assert(cashash_hash_fnv1a_bytes(data, sizeof(data)) ==
            cashash_hash_fnv1a_bytes(data, sizeof(data)));
}
END_TEST

START_TEST(test_fnv1a_long_data_changes_when_byte_changes) {
  unsigned char data_a[4096];
  unsigned char data_b[4096];
  size_t i;

  for (i = 0; i < sizeof(data_a); i++) {
    data_a[i] = (unsigned char)(i % 251);
    data_b[i] = (unsigned char)(i % 251);
  }

  data_b[sizeof(data_b) - 1] ^= 0x7F;

  ck_assert(cashash_hash_fnv1a_bytes(data_a, sizeof(data_a)) !=
            cashash_hash_fnv1a_bytes(data_b, sizeof(data_b)));
}
END_TEST

START_TEST(test_equal_fnv1a_bytes_same_data) {
  const char a[] = {'k', 'e', 'y', '\0', 'x'};
  const char b[] = {'k', 'e', 'y', '\0', 'x'};

  ck_assert(cashash_equal_fnv1a_bytes(a, b, sizeof(a)));
}
END_TEST

START_TEST(test_equal_fnv1a_bytes_different_data) {
  const char a[] = {'k', 'e', 'y', '\0', 'x'};
  const char b[] = {'k', 'e', 'y', '\0', 'y'};

  ck_assert(!cashash_equal_fnv1a_bytes(a, b, sizeof(a)));
}
END_TEST

START_TEST(test_equal_fnv1a_bytes_zero_length) {
  const char a[] = {'a'};
  const char b[] = {'b'};

  ck_assert(cashash_equal_fnv1a_bytes(a, b, 0));
}
END_TEST

START_TEST(test_copy_fnv1a_bytes_copies_exact_bytes) {
  const char key[] = {'a', 'b', '\0', 'c', 'd'};
  void *copy = cashash_copy_fnv1a_bytes(key, sizeof(key));

  ck_assert_ptr_nonnull(copy);
  ck_assert_int_eq(memcmp(copy, key, sizeof(key)), 0);

  cashash_key_destroy_fnv1a_bytes(copy);
}
END_TEST

START_TEST(test_copy_fnv1a_bytes_returns_independent_copy) {
  char key[] = {'a', 'b', '\0', 'c', 'd'};
  void *copy = cashash_copy_fnv1a_bytes(key, sizeof(key));

  ck_assert_ptr_nonnull(copy);

  key[0] = 'z';

  ck_assert(((const char *)copy)[0] == 'a');
  ck_assert(((const char *)copy)[1] == 'b');
  ck_assert(((const char *)copy)[2] == '\0');
  ck_assert(((const char *)copy)[3] == 'c');
  ck_assert(((const char *)copy)[4] == 'd');

  cashash_key_destroy_fnv1a_bytes(copy);
}
END_TEST

START_TEST(test_copy_fnv1a_bytes_long_key) {
  unsigned char key[4096];
  void *copy;
  size_t i;

  for (i = 0; i < sizeof(key); i++) {
    key[i] = (unsigned char)((i * 31) % 255);
  }

  copy = cashash_copy_fnv1a_bytes(key, sizeof(key));

  ck_assert_ptr_nonnull(copy);
  ck_assert_int_eq(memcmp(copy, key, sizeof(key)), 0);

  cashash_key_destroy_fnv1a_bytes(copy);
}
END_TEST

START_TEST(test_key_destroy_fnv1a_bytes_accepts_null) {
  cashash_key_destroy_fnv1a_bytes(NULL);
}
END_TEST

#ifdef CASHASH_USE_XXHASH

START_TEST(test_xxh3_bytes_is_deterministic) {
  const char data[] = {'x', 'x', 'h', '3', '\0', 'k', 'e', 'y'};

  size_t hash_a = cashash_hash_xxh3_bytes(data, sizeof(data));
  size_t hash_b = cashash_hash_xxh3_bytes(data, sizeof(data));

  ck_assert(hash_a == hash_b);
}
END_TEST

START_TEST(test_xxh3_binary_data_uses_full_length) {
  const char key_a[] = {'a', '\0', 'x'};
  const char key_b[] = {'a', '\0', 'y'};

  size_t hash_a = cashash_hash_xxh3_bytes(key_a, sizeof(key_a));
  size_t hash_b = cashash_hash_xxh3_bytes(key_b, sizeof(key_b));

  ck_assert(hash_a != hash_b);
}
END_TEST

START_TEST(test_xxh64_bytes_is_deterministic_with_same_seed) {
  const char data[] = {'x', 'x', 'h', '6', '4', '\0', 'k', 'e', 'y'};
  uint64_t seed = 12345;

  size_t hash_a = cashash_hash_xxh64_bytes(data, sizeof(data), seed);
  size_t hash_b = cashash_hash_xxh64_bytes(data, sizeof(data), seed);

  ck_assert(hash_a == hash_b);
}
END_TEST

START_TEST(test_xxh64_bytes_changes_with_different_seed) {
  const char data[] = {'x', 'x', 'h', '6', '4', '\0', 'k', 'e', 'y'};

  size_t hash_a = cashash_hash_xxh64_bytes(data, sizeof(data), (uint64_t)1);
  size_t hash_b = cashash_hash_xxh64_bytes(data, sizeof(data), (uint64_t)2);

  ck_assert(hash_a != hash_b);
}
END_TEST

START_TEST(test_xxh64_binary_data_uses_full_length) {
  const char key_a[] = {'a', '\0', 'x'};
  const char key_b[] = {'a', '\0', 'y'};
  uint64_t seed = 0;

  size_t hash_a = cashash_hash_xxh64_bytes(key_a, sizeof(key_a), seed);
  size_t hash_b = cashash_hash_xxh64_bytes(key_b, sizeof(key_b), seed);

  ck_assert(hash_a != hash_b);
}
END_TEST

#endif

static Suite *cashash_hash_suite(void) {
  Suite *suite = suite_create("cashash_hash");
  TCase *core = tcase_create("core");

  tcase_add_test(core, test_fnv1a_string_matches_bytes);
  tcase_add_test(core, test_fnv1a_empty_string_matches_empty_bytes);
  tcase_add_test(core, test_fnv1a_hash_is_deterministic);
  tcase_add_test(core, test_fnv1a_binary_data_uses_full_length);
  tcase_add_test(core, test_fnv1a_string_stops_at_null_byte);
  tcase_add_test(core, test_fnv1a_long_data_is_deterministic);
  tcase_add_test(core, test_fnv1a_long_data_changes_when_byte_changes);

  tcase_add_test(core, test_equal_fnv1a_bytes_same_data);
  tcase_add_test(core, test_equal_fnv1a_bytes_different_data);
  tcase_add_test(core, test_equal_fnv1a_bytes_zero_length);

  tcase_add_test(core, test_copy_fnv1a_bytes_copies_exact_bytes);
  tcase_add_test(core, test_copy_fnv1a_bytes_returns_independent_copy);
  tcase_add_test(core, test_copy_fnv1a_bytes_long_key);
  tcase_add_test(core, test_key_destroy_fnv1a_bytes_accepts_null);

#ifdef CASHASH_USE_XXHASH
  tcase_add_test(core, test_xxh3_bytes_is_deterministic);
  tcase_add_test(core, test_xxh3_binary_data_uses_full_length);
  tcase_add_test(core, test_xxh64_bytes_is_deterministic_with_same_seed);
  tcase_add_test(core, test_xxh64_bytes_changes_with_different_seed);
  tcase_add_test(core, test_xxh64_binary_data_uses_full_length);
#endif

  suite_add_tcase(suite, core);

  return suite;
}

int main(void) {
  Suite *suite = cashash_hash_suite();
  SRunner *runner = srunner_create(suite);

  srunner_run_all(runner, CK_NORMAL);

  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
