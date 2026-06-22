#include <cashash.c/cashash.h>

#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct cashash_test_struct_key {
  uint32_t id;
  uint16_t kind;
  uint8_t flags;
  char tag[8];
} cashash_test_struct_key_t;

#define CASHASH_KEY(data_, length_)                                            \
  ((cashash_key_datum_t){.data = (data_), .length = (length_)})

#define CKEY(key_) CASHASH_KEY((key_), sizeof(key_) - 1)

#define CVAL(value_) ((void *)(value_))

static const char *cashash_test_find_str(const cashash_t *map,
                                         cashash_key_datum_t key) {
  return (const char *)cashash_find(map, key);
}

START_TEST(test_create_destroy) {
  cashash_t *map = cashash_create_chain(128);

  ck_assert_ptr_nonnull(map);
  ck_assert_uint_eq(cashash_size(map), 0);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_insert_and_find) {
  cashash_t *map = cashash_create_chain(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));
  ck_assert(cashash_insert(map, CKEY("language"), CVAL("C")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("name")), "cashash");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("language")), "C");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_insert_and_find_binary_key) {
  cashash_t *map = cashash_create_chain(128);

  const char key[] = {'n', 'a', '\0', 'm', 'e'};

  ck_assert_ptr_nonnull(map);

  ck_assert(
      cashash_insert(map, CASHASH_KEY(key, sizeof(key)), CVAL("cashash")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_test_find_str(map, CASHASH_KEY(key, sizeof(key))),
                   "cashash");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_binary_keys_with_same_string_prefix_are_distinct) {
  cashash_t *map = cashash_create_chain(128);

  const char key_a[] = {'a', '\0', 'x'};
  const char key_b[] = {'a', '\0', 'y'};

  ck_assert_ptr_nonnull(map);

  ck_assert(
      cashash_insert(map, CASHASH_KEY(key_a, sizeof(key_a)), CVAL("value-a")));
  ck_assert(
      cashash_insert(map, CASHASH_KEY(key_b, sizeof(key_b)), CVAL("value-b")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_a, sizeof(key_a))), "value-a");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_b, sizeof(key_b))), "value-b");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_find_missing_key) {
  cashash_t *map = cashash_create_chain(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));

  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("missing")));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_update_existing_key) {
  cashash_t *map = cashash_create_chain(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("old")));
  ck_assert(cashash_insert(map, CKEY("name"), CVAL("new")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("name")), "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_update_existing_binary_key) {
  cashash_t *map = cashash_create_chain(128);

  const char key[] = {'i', 'd', '\0', '1'};

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CASHASH_KEY(key, sizeof(key)), CVAL("old")));
  ck_assert(cashash_insert(map, CASHASH_KEY(key, sizeof(key)), CVAL("new")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_test_find_str(map, CASHASH_KEY(key, sizeof(key))),
                   "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_collision_handling) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("k3"), CVAL("first")));
  ck_assert(cashash_insert(map, CKEY("k18"), CVAL("second")));
  ck_assert(cashash_insert(map, CKEY("k21"), CVAL("third")));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k3")), "first");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k18")), "second");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k21")), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_many_items) {
  cashash_t *map = cashash_create_chain(8);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("a"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("b"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("c"), CVAL("3")));
  ck_assert(cashash_insert(map, CKEY("d"), CVAL("4")));
  ck_assert(cashash_insert(map, CKEY("e"), CVAL("5")));
  ck_assert(cashash_insert(map, CKEY("f"), CVAL("6")));
  ck_assert(cashash_insert(map, CKEY("g"), CVAL("7")));
  ck_assert(cashash_insert(map, CKEY("h"), CVAL("8")));

  ck_assert_uint_eq(cashash_size(map), 8);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("a")), "1");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("h")), "8");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_invalid_arguments) {
  ck_assert_ptr_null(cashash_create_chain(0));

  ck_assert(!cashash_insert(NULL, CKEY("key"), CVAL("value")));
  ck_assert_ptr_null(cashash_test_find_str(NULL, CKEY("key")));
  ck_assert_uint_eq(cashash_size(NULL), 0);

  cashash_t *map = cashash_create_chain(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_insert(map, CASHASH_KEY(NULL, 0), CVAL("value")));
  ck_assert_ptr_null(cashash_test_find_str(map, CASHASH_KEY(NULL, 0)));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_dynamic_growth) {
  cashash_t *map = cashash_create_chain(2);

  ck_assert_ptr_nonnull(map);
  ck_assert_uint_eq(cashash_bucket_count(map), 2);

  ck_assert(cashash_insert(map, CKEY("one"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("two"), CVAL("2")));

  ck_assert_uint_gt(cashash_bucket_count(map), 2);
  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("one")), "1");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("two")), "2");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_dynamic_growth_preserves_many_items) {
  cashash_t *map = cashash_create_chain(1);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("a"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("b"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("c"), CVAL("3")));
  ck_assert(cashash_insert(map, CKEY("d"), CVAL("4")));
  ck_assert(cashash_insert(map, CKEY("e"), CVAL("5")));
  ck_assert(cashash_insert(map, CKEY("f"), CVAL("6")));
  ck_assert(cashash_insert(map, CKEY("g"), CVAL("7")));
  ck_assert(cashash_insert(map, CKEY("h"), CVAL("8")));

  ck_assert_uint_eq(cashash_size(map), 8);
  ck_assert_uint_gt(cashash_bucket_count(map), 1);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("a")), "1");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("b")), "2");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("c")), "3");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("d")), "4");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("e")), "5");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("f")), "6");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("g")), "7");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("h")), "8");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_dynamic_growth_preserves_binary_keys) {
  cashash_t *map = cashash_create_chain(1);

  const char key_a[] = {'a', '\0', '1'};
  const char key_b[] = {'b', '\0', '2'};
  const char key_c[] = {'c', '\0', '3'};
  const char key_d[] = {'d', '\0', '4'};

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CASHASH_KEY(key_a, sizeof(key_a)), CVAL("1")));
  ck_assert(cashash_insert(map, CASHASH_KEY(key_b, sizeof(key_b)), CVAL("2")));
  ck_assert(cashash_insert(map, CASHASH_KEY(key_c, sizeof(key_c)), CVAL("3")));
  ck_assert(cashash_insert(map, CASHASH_KEY(key_d, sizeof(key_d)), CVAL("4")));

  ck_assert_uint_eq(cashash_size(map), 4);
  ck_assert_uint_gt(cashash_bucket_count(map), 1);

  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_a, sizeof(key_a))), "1");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_b, sizeof(key_b))), "2");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_c, sizeof(key_c))), "3");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_d, sizeof(key_d))), "4");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_update_existing_key_does_not_grow_or_change_size) {
  cashash_t *map = cashash_create_chain(8);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("old")));

  size_t before_bucket_count = cashash_bucket_count(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("new")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_uint_eq(cashash_bucket_count(map), before_bucket_count);
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("name")), "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_existing_key) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));
  ck_assert(cashash_insert(map, CKEY("language"), CVAL("C")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert(cashash_remove(map, CKEY("name")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("name")));
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("language")), "C");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_existing_binary_key) {
  cashash_t *map = cashash_create_chain(32);

  const char key_a[] = {'a', '\0', 'x'};
  const char key_b[] = {'a', '\0', 'y'};

  ck_assert_ptr_nonnull(map);

  ck_assert(
      cashash_insert(map, CASHASH_KEY(key_a, sizeof(key_a)), CVAL("value-a")));
  ck_assert(
      cashash_insert(map, CASHASH_KEY(key_b, sizeof(key_b)), CVAL("value-b")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert(cashash_remove(map, CASHASH_KEY(key_a, sizeof(key_a))));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_ptr_null(
      cashash_test_find_str(map, CASHASH_KEY(key_a, sizeof(key_a))));
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_b, sizeof(key_b))), "value-b");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_missing_key) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("name"), CVAL("cashash")));

  ck_assert(!cashash_remove(map, CKEY("missing")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("name")), "cashash");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_from_empty_table) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_remove(map, CKEY("missing")));
  ck_assert_uint_eq(cashash_size(map), 0);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_null_arguments) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_remove(NULL, CKEY("key")));
  ck_assert(!cashash_remove(map, CASHASH_KEY(NULL, 0)));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_from_collision_chain) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("k3"), CVAL("first")));
  ck_assert(cashash_insert(map, CKEY("k18"), CVAL("second")));
  ck_assert(cashash_insert(map, CKEY("k21"), CVAL("third")));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert(cashash_remove(map, CKEY("k18")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k3")), "first");
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("k18")));
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k21")), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_first_node_in_collision_chain) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("k3"), CVAL("first")));
  ck_assert(cashash_insert(map, CKEY("k18"), CVAL("second")));
  ck_assert(cashash_insert(map, CKEY("k21"), CVAL("third")));

  ck_assert(cashash_remove(map, CKEY("k21")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k3")), "first");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k18")), "second");
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("k21")));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_middle_node_in_collision_chain) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("k3"), CVAL("first")));
  ck_assert(cashash_insert(map, CKEY("k18"), CVAL("second")));
  ck_assert(cashash_insert(map, CKEY("k21"), CVAL("third")));

  ck_assert(cashash_remove(map, CKEY("k18")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k3")), "first");
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("k18")));
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k21")), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_last_node_in_collision_chain) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("k3"), CVAL("first")));
  ck_assert(cashash_insert(map, CKEY("k18"), CVAL("second")));
  ck_assert(cashash_insert(map, CKEY("k21"), CVAL("third")));

  ck_assert(cashash_remove(map, CKEY("k3")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("k3")));
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k18")), "second");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("k21")), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_preserves_other_keys) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("a"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("b"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("c"), CVAL("3")));
  ck_assert(cashash_insert(map, CKEY("d"), CVAL("4")));

  ck_assert(cashash_remove(map, CKEY("b")));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert_str_eq(cashash_test_find_str(map, CKEY("a")), "1");
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("b")));
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("c")), "3");
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("d")), "4");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_clear_empty_table) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  size_t bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_clear_populated_table) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("one"), CVAL("1")));
  ck_assert(cashash_insert(map, CKEY("two"), CVAL("2")));
  ck_assert(cashash_insert(map, CKEY("three"), CVAL("3")));

  ck_assert_uint_eq(cashash_size(map), 3);

  size_t bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("one")));
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("two")));
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("three")));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_insert_after_clear) {
  cashash_t *map = cashash_create_chain(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CKEY("old"), CVAL("value")));

  size_t bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  ck_assert(cashash_insert(map, CKEY("new"), CVAL("value")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);
  ck_assert_ptr_null(cashash_test_find_str(map, CKEY("old")));
  ck_assert_str_eq(cashash_test_find_str(map, CKEY("new")), "value");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_clear_null_table) { cashash_clear(NULL); }
END_TEST

START_TEST(test_integer_keys_insert_and_find) {
  cashash_t *map = cashash_create_chain(32);

  int key_a = 10;
  int key_b = 20;
  int key_c = -30;

  ck_assert_ptr_nonnull(map);

  ck_assert(
      cashash_insert(map, CASHASH_KEY(&key_a, sizeof(key_a)), CVAL("ten")));
  ck_assert(
      cashash_insert(map, CASHASH_KEY(&key_b, sizeof(key_b)), CVAL("twenty")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key_c, sizeof(key_c)),
                           CVAL("minus-thirty")));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(&key_a, sizeof(key_a))), "ten");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(&key_b, sizeof(key_b))), "twenty");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(&key_c, sizeof(key_c))),
      "minus-thirty");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_integer_key_update) {
  cashash_t *map = cashash_create_chain(32);

  int key = 42;

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CASHASH_KEY(&key, sizeof(key)), CVAL("old")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key, sizeof(key)), CVAL("new")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_test_find_str(map, CASHASH_KEY(&key, sizeof(key))),
                   "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_integer_key_remove) {
  cashash_t *map = cashash_create_chain(32);

  int key_a = 1;
  int key_b = 2;

  ck_assert_ptr_nonnull(map);

  ck_assert(
      cashash_insert(map, CASHASH_KEY(&key_a, sizeof(key_a)), CVAL("one")));
  ck_assert(
      cashash_insert(map, CASHASH_KEY(&key_b, sizeof(key_b)), CVAL("two")));

  ck_assert(cashash_remove(map, CASHASH_KEY(&key_a, sizeof(key_a))));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_ptr_null(
      cashash_test_find_str(map, CASHASH_KEY(&key_a, sizeof(key_a))));
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(&key_b, sizeof(key_b))), "two");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_long_key_insert_and_find) {
  cashash_t *map = cashash_create_chain(32);

  char long_key[4096];

  ck_assert_ptr_nonnull(map);

  for (size_t i = 0; i < sizeof(long_key); i++) {
    long_key[i] = (char)(i % 251);
  }

  ck_assert(cashash_insert(map, CASHASH_KEY(long_key, sizeof(long_key)),
                           CVAL("long-value")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(long_key, sizeof(long_key))),
      "long-value");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_long_keys_with_same_prefix_are_distinct) {
  cashash_t *map = cashash_create_chain(32);

  char key_a[2048];
  char key_b[2048];

  ck_assert_ptr_nonnull(map);

  for (size_t i = 0; i < sizeof(key_a); i++) {
    key_a[i] = (char)(i % 127);
    key_b[i] = (char)(i % 127);
  }

  key_a[sizeof(key_a) - 1] = 'A';
  key_b[sizeof(key_b) - 1] = 'B';

  ck_assert(
      cashash_insert(map, CASHASH_KEY(key_a, sizeof(key_a)), CVAL("value-a")));
  ck_assert(
      cashash_insert(map, CASHASH_KEY(key_b, sizeof(key_b)), CVAL("value-b")));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_a, sizeof(key_a))), "value-a");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(key_b, sizeof(key_b))), "value-b");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_long_key_survives_dynamic_growth) {
  cashash_t *map = cashash_create_chain(1);

  char long_key[4096];
  int keys[32];

  ck_assert_ptr_nonnull(map);

  for (size_t i = 0; i < sizeof(long_key); i++) {
    long_key[i] = (char)((i * 31) % 255);
  }

  ck_assert(cashash_insert(map, CASHASH_KEY(long_key, sizeof(long_key)),
                           CVAL("long-value")));

  for (size_t i = 0; i < 32; i++) {
    keys[i] = (int)i;
    ck_assert(cashash_insert(map, CASHASH_KEY(&keys[i], sizeof(keys[i])),
                             CVAL("int-value")));
  }

  ck_assert_uint_gt(cashash_bucket_count(map), 1);
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(long_key, sizeof(long_key))),
      "long-value");

  for (size_t i = 0; i < 32; i++) {
    ck_assert_str_eq(
        cashash_test_find_str(map, CASHASH_KEY(&keys[i], sizeof(keys[i]))),
        "int-value");
  }

  cashash_destroy(map);
}
END_TEST

START_TEST(test_struct_keys_insert_and_find) {
  cashash_t *map = cashash_create_chain(32);

  cashash_test_struct_key_t key_a;
  cashash_test_struct_key_t key_b;

  ck_assert_ptr_nonnull(map);

  memset(&key_a, 0, sizeof(key_a));
  memset(&key_b, 0, sizeof(key_b));

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
      cashash_test_find_str(map, CASHASH_KEY(&key_a, sizeof(key_a))),
      "struct-a");
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(&key_b, sizeof(key_b))),
      "struct-b");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_struct_key_update) {
  cashash_t *map = cashash_create_chain(32);

  cashash_test_struct_key_t key;

  ck_assert_ptr_nonnull(map);

  memset(&key, 0, sizeof(key));

  key.id = 777;
  key.kind = 3;
  key.flags = 1;
  memcpy(key.tag, "user", 4);

  ck_assert(cashash_insert(map, CASHASH_KEY(&key, sizeof(key)), CVAL("old")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key, sizeof(key)), CVAL("new")));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_test_find_str(map, CASHASH_KEY(&key, sizeof(key))),
                   "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_struct_key_remove) {
  cashash_t *map = cashash_create_chain(32);

  cashash_test_struct_key_t key_a;
  cashash_test_struct_key_t key_b;

  ck_assert_ptr_nonnull(map);

  memset(&key_a, 0, sizeof(key_a));
  memset(&key_b, 0, sizeof(key_b));

  key_a.id = 1;
  key_a.kind = 10;
  key_a.flags = 2;
  memcpy(key_a.tag, "left", 4);

  key_b.id = 2;
  key_b.kind = 20;
  key_b.flags = 4;
  memcpy(key_b.tag, "right", 5);

  ck_assert(cashash_insert(map, CASHASH_KEY(&key_a, sizeof(key_a)),
                           CVAL("left-value")));
  ck_assert(cashash_insert(map, CASHASH_KEY(&key_b, sizeof(key_b)),
                           CVAL("right-value")));

  ck_assert(cashash_remove(map, CASHASH_KEY(&key_a, sizeof(key_a))));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_ptr_null(
      cashash_test_find_str(map, CASHASH_KEY(&key_a, sizeof(key_a))));
  ck_assert_str_eq(
      cashash_test_find_str(map, CASHASH_KEY(&key_b, sizeof(key_b))),
      "right-value");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_struct_keys_survive_dynamic_growth) {
  cashash_t *map = cashash_create_chain(1);

  cashash_test_struct_key_t keys[32];

  ck_assert_ptr_nonnull(map);

  for (size_t i = 0; i < 32; i++) {
    memset(&keys[i], 0, sizeof(keys[i]));

    keys[i].id = (uint32_t)i;
    keys[i].kind = (uint16_t)(i % 5);
    keys[i].flags = (uint8_t)(i % 3);
    memcpy(keys[i].tag, "key", 3);

    ck_assert(cashash_insert(map, CASHASH_KEY(&keys[i], sizeof(keys[i])),
                             CVAL("struct-value")));
  }

  ck_assert_uint_eq(cashash_size(map), 32);
  ck_assert_uint_gt(cashash_bucket_count(map), 1);

  for (size_t i = 0; i < 32; i++) {
    ck_assert_str_eq(
        cashash_test_find_str(map, CASHASH_KEY(&keys[i], sizeof(keys[i]))),
        "struct-value");
  }

  cashash_destroy(map);
}
END_TEST

static Suite *cashash_suite(void) {
  Suite *suite = suite_create("cashash");
  TCase *core = tcase_create("core");

  tcase_add_test(core, test_create_destroy);
  tcase_add_test(core, test_insert_and_find);
  tcase_add_test(core, test_insert_and_find_binary_key);
  tcase_add_test(core, test_binary_keys_with_same_string_prefix_are_distinct);
  tcase_add_test(core, test_find_missing_key);
  tcase_add_test(core, test_update_existing_key);
  tcase_add_test(core, test_update_existing_binary_key);
  tcase_add_test(core, test_collision_handling);
  tcase_add_test(core, test_many_items);
  tcase_add_test(core, test_invalid_arguments);

  tcase_add_test(core, test_dynamic_growth);
  tcase_add_test(core, test_dynamic_growth_preserves_many_items);
  tcase_add_test(core, test_dynamic_growth_preserves_binary_keys);
  tcase_add_test(core, test_update_existing_key_does_not_grow_or_change_size);

  tcase_add_test(core, test_remove_existing_key);
  tcase_add_test(core, test_remove_existing_binary_key);
  tcase_add_test(core, test_remove_missing_key);
  tcase_add_test(core, test_remove_from_empty_table);
  tcase_add_test(core, test_remove_null_arguments);
  tcase_add_test(core, test_remove_from_collision_chain);
  tcase_add_test(core, test_remove_first_node_in_collision_chain);
  tcase_add_test(core, test_remove_middle_node_in_collision_chain);
  tcase_add_test(core, test_remove_last_node_in_collision_chain);
  tcase_add_test(core, test_remove_preserves_other_keys);

  tcase_add_test(core, test_clear_empty_table);
  tcase_add_test(core, test_clear_populated_table);
  tcase_add_test(core, test_insert_after_clear);
  tcase_add_test(core, test_clear_null_table);

  tcase_add_test(core, test_integer_keys_insert_and_find);
  tcase_add_test(core, test_integer_key_update);
  tcase_add_test(core, test_integer_key_remove);

  tcase_add_test(core, test_long_key_insert_and_find);
  tcase_add_test(core, test_long_keys_with_same_prefix_are_distinct);
  tcase_add_test(core, test_long_key_survives_dynamic_growth);

  tcase_add_test(core, test_struct_keys_insert_and_find);
  tcase_add_test(core, test_struct_key_update);
  tcase_add_test(core, test_struct_key_remove);
  tcase_add_test(core, test_struct_keys_survive_dynamic_growth);

  suite_add_tcase(suite, core);

  return suite;
}

int main(void) {
  Suite *suite = cashash_suite();
  SRunner *runner = srunner_create(suite);

  srunner_run_all(runner, CK_NORMAL);

  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
