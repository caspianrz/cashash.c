#include <cashash.c/cashash.h>

#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct cashash_oa_test_struct_key {
  uint32_t id;
  uint16_t kind;
  uint8_t flags;
  char tag[8];
} cashash_oa_test_struct_key_t;

#define CASHASH_KEY(data_, length_)                                            \
  ((cashash_key_datum_t){.data = (data_), .length = (length_)})

#define CKEY(key_) CASHASH_KEY((key_), sizeof(key_) - 1)
#define CVAL(value_) ((void *)(value_))

static const char *cashash_oa_test_find_str(const cashash_t *map,
                                            cashash_key_datum_t key) {
  return (const char *)cashash_find(map, key);
}

static size_t cashash_test_second_hash(const void *key, const size_t len, ...) {
  const unsigned char *bytes = key;
  size_t hash = 1469598103934665603ULL;

  for (size_t i = 0; i < len; i++) {
    hash ^= bytes[i];
    hash *= 1099511628211ULL;
  }

  return hash | 1U;
}

START_TEST(test_oa_create_destroy_default) {
  cashash_t *map = cashash_create_open_addressing(128);

  ck_assert_ptr_nonnull(map);
  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), 128);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_create_invalid_arguments) {
  ck_assert_ptr_null(cashash_create_open_addressing(0));

  ck_assert_ptr_null(cashash_create_open_addressing_with_double_hash(
      32, CASHASH_HASH_STRATEGY_FNV1A, (cashash_strategy_option_t){0}, NULL,
      NULL));
}
END_TEST

START_TEST(test_oa_create_linear_strategy) {
  cashash_t *map = cashash_create_open_addressing_with_strategy(
      32, CASHASH_HASH_STRATEGY_FNV1A, (cashash_strategy_option_t){0},
      CASHASH_OA_PROBE_LINEAR);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("name")), "cashash");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_create_quadratic_strategy) {
  cashash_t *map = cashash_create_open_addressing_with_strategy(
      32, CASHASH_HASH_STRATEGY_FNV1A, (cashash_strategy_option_t){0},
      CASHASH_OA_PROBE_QUADRATIC);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("name")), "cashash");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_create_double_hash_strategy) {
  cashash_t *map = cashash_create_open_addressing_with_double_hash(
      32, CASHASH_HASH_STRATEGY_FNV1A, (cashash_strategy_option_t){0},
      cashash_test_second_hash, NULL);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("name")), "cashash");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_insert_and_find) {
  cashash_t *map = cashash_create_open_addressing(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));
  ck_assert(cashash_insert(map, CKEY("language"), CVAL("C")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("name")), "cashash");
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("language")), "C");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_update_existing_key) {
  cashash_t *map = cashash_create_open_addressing(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("old")));
  ck_assert(cashash_insert(map, CKEY("name"), CVAL("new")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("name")), "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_find_missing_key) {
  cashash_t *map = cashash_create_open_addressing(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));

  ck_assert_ptr_null(cashash_oa_test_find_str(map, CKEY("missing")));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_binary_keys) {
  cashash_t *map = cashash_create_open_addressing(128);

  const char key_a[] = {'a', '\0', 'x'};
  const char key_b[] = {'a', '\0', 'y'};

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CASHASH_KEY(key_a, sizeof(key_a)),
                           CVAL("value-a")));
  ck_assert(cashash_insert(map, CASHASH_KEY(key_b, sizeof(key_b)),
                           CVAL("value-b")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(key_a, sizeof(key_a))),
      "value-a");
  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(key_b, sizeof(key_b))),
      "value-b");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_integer_keys) {
  cashash_t *map = cashash_create_open_addressing(32);

  int key_a = 10;
  int key_b = 20;
  int key_c = -30;

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CASHASH_KEY(&key_a, sizeof(key_a)),
                           CVAL("ten")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key_b, sizeof(key_b)),
                           CVAL("twenty")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key_c, sizeof(key_c)),
                           CVAL("minus-thirty")));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(&key_a, sizeof(key_a))),
      "ten");
  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(&key_b, sizeof(key_b))),
      "twenty");
  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(&key_c, sizeof(key_c))),
      "minus-thirty");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_struct_keys) {
  cashash_t *map = cashash_create_open_addressing(32);

  cashash_oa_test_struct_key_t key_a;
  cashash_oa_test_struct_key_t key_b;

  ck_assert_ptr_nonnull(map);

  memset(&key_a, 0, sizeof key_a);
  memset(&key_b, 0, sizeof key_b);

  key_a.id = 100;
  key_a.kind = 1;
  key_a.flags = 7;
  memcpy(key_a.tag, "alpha", 5);

  key_b.id = 200;
  key_b.kind = 2;
  key_b.flags = 9;
  memcpy(key_b.tag, "beta", 4);

  ck_assert(cashash_insert(map, CASHASH_KEY(&key_a, sizeof(key_a)),
                           CVAL("struct-a")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key_b, sizeof(key_b)),
                           CVAL("struct-b")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(&key_a, sizeof(key_a))),
      "struct-a");
  ck_assert_str_eq(
      cashash_oa_test_find_str(map, CASHASH_KEY(&key_b, sizeof(key_b))),
      "struct-b");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_remove_existing_key_creates_tombstone) {
  cashash_t *map = cashash_create_open_addressing(8);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("a"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("b"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("c"), CVAL("3")));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert(cashash_remove(map, CKEY("b")));

  ck_assert_uint_eq(cashash_size(map), 2);
  ck_assert_ptr_null(cashash_oa_test_find_str(map, CKEY("b")));
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("a")), "1");
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("c")), "3");

  ck_assert(cashash_insert(map, CKEY("d"), CVAL("4")));

  ck_assert_uint_eq(cashash_size(map), 3);
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("d")), "4");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_remove_missing_key) {
  cashash_t *map = cashash_create_open_addressing(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));

  ck_assert(!cashash_remove(map, CKEY("missing")));
  ck_assert_uint_eq(cashash_size(map), 1);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_remove_from_empty_table) {
  cashash_t *map = cashash_create_open_addressing(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_remove(map, CKEY("missing")));
  ck_assert_uint_eq(cashash_size(map), 0);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_clear_empty_table) {
  cashash_t *map = cashash_create_open_addressing(32);

  ck_assert_ptr_nonnull(map);

  size_t bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_clear_populated_table) {
  cashash_t *map = cashash_create_open_addressing(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("one"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("two"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("three"), CVAL("3")));

  ck_assert_uint_eq(cashash_size(map), 3);

  size_t bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  ck_assert_ptr_null(cashash_oa_test_find_str(map, CKEY("one")));
  ck_assert_ptr_null(cashash_oa_test_find_str(map, CKEY("two")));
  ck_assert_ptr_null(cashash_oa_test_find_str(map, CKEY("three")));

  ck_assert(cashash_insert(map, CKEY("new"), CVAL("value")));
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("new")), "value");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_dynamic_growth_preserves_items) {
  cashash_t *map = cashash_create_open_addressing(2);

  ck_assert_ptr_nonnull(map);
  ck_assert_uint_eq(cashash_bucket_count(map), 2);

  ck_assert(cashash_insert(map, CKEY("a"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("b"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("c"), CVAL("3")));
  ck_assert(cashash_insert(map, CKEY("d"), CVAL("4")));

  ck_assert_uint_gt(cashash_bucket_count(map), 2);
  ck_assert_uint_eq(cashash_size(map), 4);

  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("a")), "1");
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("b")), "2");
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("c")), "3");
  ck_assert_str_eq(cashash_oa_test_find_str(map, CKEY("d")), "4");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_key_is_copied) {
  cashash_t *map = cashash_create_open_addressing(16);

  ck_assert_ptr_nonnull(map);

  int original_key = 42;
  int value = 123;

  cashash_key_datum_t key = {
      .data = &original_key,
      .length = sizeof original_key,
  };

  ck_assert(cashash_insert(map, key, &value));

  original_key = 99;

  int old_key = 42;
  int new_key = 99;

  ck_assert_ptr_nonnull(
      cashash_find(map, CASHASH_KEY(&old_key, sizeof old_key)));
  ck_assert_ptr_null(
      cashash_find(map, CASHASH_KEY(&new_key, sizeof new_key)));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_mutable_pointer_value) {
  cashash_t *map = cashash_create_open_addressing(16);

  ck_assert_ptr_nonnull(map);

  int key_data = 7;
  int value_data = 10;

  cashash_key_datum_t key = {
      .data = &key_data,
      .length = sizeof key_data,
  };

  ck_assert(cashash_insert(map, key, &value_data));

  int *found = cashash_find(map, key);

  ck_assert_ptr_nonnull(found);
  ck_assert_int_eq(*found, 10);

  *found += 5;

  ck_assert_int_eq(value_data, 15);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_null_arguments) {
  cashash_t *map = cashash_create_open_addressing(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_insert(NULL, CKEY("key"), CVAL("value")));
  ck_assert(!cashash_insert(map, CASHASH_KEY(NULL, 0), CVAL("value")));

  ck_assert_ptr_null(cashash_find(NULL, CKEY("key")));
  ck_assert_ptr_null(cashash_find(map, CASHASH_KEY(NULL, 0)));

  ck_assert(!cashash_remove(NULL, CKEY("key")));
  ck_assert(!cashash_remove(map, CASHASH_KEY(NULL, 0)));

  cashash_clear(NULL);
  cashash_destroy(NULL);

  cashash_destroy(map);
}
END_TEST

static Suite *cashash_oa_suite(void) {
  Suite *suite = suite_create("cashash_open_addressing");
  TCase *core = tcase_create("core");

  tcase_add_test(core, test_oa_create_destroy_default);
  tcase_add_test(core, test_oa_create_invalid_arguments);
  tcase_add_test(core, test_oa_create_linear_strategy);
  tcase_add_test(core, test_oa_create_quadratic_strategy);
  tcase_add_test(core, test_oa_create_double_hash_strategy);

  tcase_add_test(core, test_oa_insert_and_find);
  tcase_add_test(core, test_oa_update_existing_key);
  tcase_add_test(core, test_oa_find_missing_key);
  tcase_add_test(core, test_oa_binary_keys);
  tcase_add_test(core, test_oa_integer_keys);
  tcase_add_test(core, test_oa_struct_keys);

  tcase_add_test(core, test_oa_remove_existing_key_creates_tombstone);
  tcase_add_test(core, test_oa_remove_missing_key);
  tcase_add_test(core, test_oa_remove_from_empty_table);

  tcase_add_test(core, test_oa_clear_empty_table);
  tcase_add_test(core, test_oa_clear_populated_table);

  tcase_add_test(core, test_oa_dynamic_growth_preserves_items);
  tcase_add_test(core, test_oa_key_is_copied);
  tcase_add_test(core, test_oa_mutable_pointer_value);
  tcase_add_test(core, test_oa_null_arguments);

  suite_add_tcase(suite, core);

  return suite;
}

int main(void) {
  Suite *suite = cashash_oa_suite();
  SRunner *runner = srunner_create(suite);

  srunner_run_all(runner, CK_NORMAL);

  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
