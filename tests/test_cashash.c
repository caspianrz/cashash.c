#include <cashash.c/cashash.h>

#include <check.h>
#include <stdlib.h>

START_TEST(test_create_destroy) {
  cashash_t *map = cashash_create(128);

  ck_assert_ptr_nonnull(map);
  ck_assert_uint_eq(cashash_size(map), 0);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_insert_and_find) {
  cashash_t *map = cashash_create(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "name", "cashash"));
  ck_assert(cashash_insert(map, "language", "C"));

  ck_assert_uint_eq(cashash_size(map), 2);

  const char *name = cashash_find(map, "name");
  const char *language = cashash_find(map, "language");

  ck_assert_str_eq(name, "cashash");
  ck_assert_str_eq(language, "C");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_find_missing_key) {
  cashash_t *map = cashash_create(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "name", "cashash"));

  ck_assert_ptr_null(cashash_find(map, "missing"));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_update_existing_key) {
  cashash_t *map = cashash_create(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "name", "old"));
  ck_assert(cashash_insert(map, "name", "new"));

  ck_assert_uint_eq(cashash_size(map), 1);

  const char *value = cashash_find(map, "name");

  ck_assert_str_eq(value, "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_collision_handling) {
  /*
      bucket_count = 1 forces every key into the same bucket.

      This directly tests separate chaining.
  */

  cashash_t *map = cashash_create(1);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "one", "1"));
  ck_assert(cashash_insert(map, "two", "2"));
  ck_assert(cashash_insert(map, "three", "3"));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert_str_eq(cashash_find(map, "one"), "1");
  ck_assert_str_eq(cashash_find(map, "two"), "2");
  ck_assert_str_eq(cashash_find(map, "three"), "3");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_many_items) {
  cashash_t *map = cashash_create(8);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "a", "1"));
  ck_assert(cashash_insert(map, "b", "2"));
  ck_assert(cashash_insert(map, "c", "3"));
  ck_assert(cashash_insert(map, "d", "4"));
  ck_assert(cashash_insert(map, "e", "5"));
  ck_assert(cashash_insert(map, "f", "6"));
  ck_assert(cashash_insert(map, "g", "7"));
  ck_assert(cashash_insert(map, "h", "8"));

  ck_assert_uint_eq(cashash_size(map), 8);

  ck_assert_str_eq(cashash_find(map, "a"), "1");
  ck_assert_str_eq(cashash_find(map, "h"), "8");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_invalid_arguments) {
  ck_assert_ptr_null(cashash_create(0));

  ck_assert(!cashash_insert(NULL, "key", "value"));
  ck_assert_ptr_null(cashash_find(NULL, "key"));
  ck_assert_uint_eq(cashash_size(NULL), 0);

  cashash_t *map = cashash_create(128);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_insert(map, NULL, "value"));
  ck_assert_ptr_null(cashash_find(map, NULL));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_dynamic_growth) {
  cashash_t *map = cashash_create(2);

  ck_assert_ptr_nonnull(map);
  ck_assert_uint_eq(cashash_bucket_count(map), 2);

  ck_assert(cashash_insert(map, "one", "1"));
  ck_assert(cashash_insert(map, "two", "2"));

  ck_assert_uint_gt(cashash_bucket_count(map), 2);

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_find(map, "one"), "1");
  ck_assert_str_eq(cashash_find(map, "two"), "2");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_dynamic_growth_preserves_many_items) {
  cashash_t *map = cashash_create(1);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "a", "1"));
  ck_assert(cashash_insert(map, "b", "2"));
  ck_assert(cashash_insert(map, "c", "3"));
  ck_assert(cashash_insert(map, "d", "4"));
  ck_assert(cashash_insert(map, "e", "5"));
  ck_assert(cashash_insert(map, "f", "6"));
  ck_assert(cashash_insert(map, "g", "7"));
  ck_assert(cashash_insert(map, "h", "8"));

  ck_assert_uint_eq(cashash_size(map), 8);
  ck_assert_uint_gt(cashash_bucket_count(map), 1);

  ck_assert_str_eq(cashash_find(map, "a"), "1");
  ck_assert_str_eq(cashash_find(map, "b"), "2");
  ck_assert_str_eq(cashash_find(map, "c"), "3");
  ck_assert_str_eq(cashash_find(map, "d"), "4");
  ck_assert_str_eq(cashash_find(map, "e"), "5");
  ck_assert_str_eq(cashash_find(map, "f"), "6");
  ck_assert_str_eq(cashash_find(map, "g"), "7");
  ck_assert_str_eq(cashash_find(map, "h"), "8");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_update_existing_key_does_not_grow_or_change_size) {
  cashash_t *map = cashash_create(8);

  size_t before_bucket_count;

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "name", "old"));

  before_bucket_count = cashash_bucket_count(map);

  ck_assert(cashash_insert(map, "name", "new"));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_uint_eq(cashash_bucket_count(map), before_bucket_count);
  ck_assert_str_eq(cashash_find(map, "name"), "new");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_existing_key) {
  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "name", "cashash"));
  ck_assert(cashash_insert(map, "language", "C"));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert(cashash_remove(map, "name"));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_ptr_null(cashash_find(map, "name"));
  ck_assert_str_eq(cashash_find(map, "language"), "C");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_missing_key) {
  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "name", "cashash"));

  ck_assert(!cashash_remove(map, "missing"));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_str_eq(cashash_find(map, "name"), "cashash");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_from_empty_table) {
  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_remove(map, "missing"));
  ck_assert_uint_eq(cashash_size(map), 0);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_null_arguments) {
  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_remove(NULL, "key"));
  ck_assert(!cashash_remove(map, NULL));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_from_collision_chain) {
  /*
      With the 64-bit FNV-1a implementation used by cashash.c,
      these keys collide when bucket_count is 32:

          "k3"
          "k18"
          "k21"

      Since insertion prepends nodes, the chain order becomes:

          k21 -> k18 -> k3
  */

  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "k3", "first"));
  ck_assert(cashash_insert(map, "k18", "second"));
  ck_assert(cashash_insert(map, "k21", "third"));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert(cashash_remove(map, "k18"));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_find(map, "k3"), "first");
  ck_assert_ptr_null(cashash_find(map, "k18"));
  ck_assert_str_eq(cashash_find(map, "k21"), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_first_node_in_collision_chain) {
  /*
      Insertion prepends nodes.

      Insert order:
          k3, k18, k21

      Chain order:
          k21 -> k18 -> k3

      Removing k21 removes the first node in the chain.
  */

  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "k3", "first"));
  ck_assert(cashash_insert(map, "k18", "second"));
  ck_assert(cashash_insert(map, "k21", "third"));

  ck_assert(cashash_remove(map, "k21"));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_find(map, "k3"), "first");
  ck_assert_str_eq(cashash_find(map, "k18"), "second");
  ck_assert_ptr_null(cashash_find(map, "k21"));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_middle_node_in_collision_chain) {
  /*
      Chain order:
          k21 -> k18 -> k3

      Removing k18 removes the middle node.
  */

  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "k3", "first"));
  ck_assert(cashash_insert(map, "k18", "second"));
  ck_assert(cashash_insert(map, "k21", "third"));

  ck_assert(cashash_remove(map, "k18"));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_str_eq(cashash_find(map, "k3"), "first");
  ck_assert_ptr_null(cashash_find(map, "k18"));
  ck_assert_str_eq(cashash_find(map, "k21"), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_last_node_in_collision_chain) {
  /*
      Chain order:
          k21 -> k18 -> k3

      Removing k3 removes the last node.
  */

  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "k3", "first"));
  ck_assert(cashash_insert(map, "k18", "second"));
  ck_assert(cashash_insert(map, "k21", "third"));

  ck_assert(cashash_remove(map, "k3"));

  ck_assert_uint_eq(cashash_size(map), 2);

  ck_assert_ptr_null(cashash_find(map, "k3"));
  ck_assert_str_eq(cashash_find(map, "k18"), "second");
  ck_assert_str_eq(cashash_find(map, "k21"), "third");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_remove_preserves_other_keys) {
  cashash_t *map = cashash_create(32);

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "a", "1"));
  ck_assert(cashash_insert(map, "b", "2"));
  ck_assert(cashash_insert(map, "c", "3"));
  ck_assert(cashash_insert(map, "d", "4"));

  ck_assert(cashash_remove(map, "b"));

  ck_assert_uint_eq(cashash_size(map), 3);

  ck_assert_str_eq(cashash_find(map, "a"), "1");
  ck_assert_ptr_null(cashash_find(map, "b"));
  ck_assert_str_eq(cashash_find(map, "c"), "3");
  ck_assert_str_eq(cashash_find(map, "d"), "4");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_clear_empty_table) {
  cashash_t *map = cashash_create(32);
  size_t bucket_count;

  ck_assert_ptr_nonnull(map);

  bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_clear_populated_table) {
  cashash_t *map = cashash_create(32);
  size_t bucket_count;

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "one", "1"));
  ck_assert(cashash_insert(map, "two", "2"));
  ck_assert(cashash_insert(map, "three", "3"));

  ck_assert_uint_eq(cashash_size(map), 3);

  bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  ck_assert_ptr_null(cashash_find(map, "one"));
  ck_assert_ptr_null(cashash_find(map, "two"));
  ck_assert_ptr_null(cashash_find(map, "three"));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_insert_after_clear) {
  cashash_t *map = cashash_create(32);
  size_t bucket_count;

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, "old", "value"));

  bucket_count = cashash_bucket_count(map);

  cashash_clear(map);

  ck_assert_uint_eq(cashash_size(map), 0);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);

  ck_assert(cashash_insert(map, "new", "value"));

  ck_assert_uint_eq(cashash_size(map), 1);
  ck_assert_uint_eq(cashash_bucket_count(map), bucket_count);
  ck_assert_ptr_null(cashash_find(map, "old"));
  ck_assert_str_eq(cashash_find(map, "new"), "value");

  cashash_destroy(map);
}
END_TEST

START_TEST(test_clear_null_table) { cashash_clear(NULL); }
END_TEST

static Suite *cashash_suite(void) {
  Suite *suite = suite_create("cashash");

  TCase *core = tcase_create("core");

  tcase_add_test(core, test_create_destroy);
  tcase_add_test(core, test_insert_and_find);
  tcase_add_test(core, test_find_missing_key);
  tcase_add_test(core, test_update_existing_key);
  tcase_add_test(core, test_collision_handling);
  tcase_add_test(core, test_many_items);
  tcase_add_test(core, test_invalid_arguments);

  tcase_add_test(core, test_dynamic_growth);
  tcase_add_test(core, test_dynamic_growth_preserves_many_items);
  tcase_add_test(core, test_update_existing_key_does_not_grow_or_change_size);

  tcase_add_test(core, test_remove_existing_key);
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
