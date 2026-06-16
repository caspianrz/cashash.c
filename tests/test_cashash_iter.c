#include <check.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <cashash.c/cashash.h>
#include <cashash.c/cashash_iter.h>

#ifndef ck_assert_false
#define ck_assert_false(expr) ck_assert(!(expr))
#endif

static const int test_keys[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

static int test_values[10] = {15, 16, 17, 18, 19, 20, 21, 22, 23, 24};

static int datum_to_int(const void *data) {
  int value;
  memcpy(&value, data, sizeof value);
  return value;
}

static void populate_map(cashash_t *table) {
  for (size_t i = 0; i < 10; i++) {
    const cashash_key_datum_t key = {
        .data = &test_keys[i],
        .length = sizeof test_keys[i],
    };

    const cashash_value_datum_t value = {
        .data = &test_values[i],
        .length = sizeof test_values[i],
    };

    ck_assert(cashash_insert(table, key, value));
  }
}

START_TEST(test_iter_empty_table) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert_false(cashash_iter_has_next(&iter));
  ck_assert_false(cashash_iter_next(&iter, &pair));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_single_entry) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  static const int key_data = 42;
  static int value_data = 57;

  const cashash_key_datum_t key = {
      .data = &key_data,
      .length = sizeof key_data,
  };

  const cashash_value_datum_t value = {
      .data = &value_data,
      .length = sizeof value_data,
  };

  ck_assert(cashash_insert(map, key, value));

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_next(&iter, &pair));

  ck_assert_uint_eq(pair.key.length, sizeof key_data);
  ck_assert_uint_eq(pair.value.length, sizeof value_data);

  ck_assert_int_eq(datum_to_int(pair.key.data), 42);
  ck_assert_int_eq(datum_to_int(pair.value.data), 57);

  ck_assert_false(cashash_iter_has_next(&iter));
  ck_assert_false(cashash_iter_next(&iter, &pair));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_multiple_entries) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  bool seen[10] = {false};
  size_t count = 0;

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  while (cashash_iter_has_next(&iter)) {
    ck_assert(cashash_iter_next(&iter, &pair));

    ck_assert_uint_eq(pair.key.length, sizeof(int));
    ck_assert_uint_eq(pair.value.length, sizeof(int));

    int key = datum_to_int(pair.key.data);
    int value = datum_to_int(pair.value.data);

    ck_assert_int_ge(key, 0);
    ck_assert_int_lt(key, 10);
    ck_assert_int_eq(value, key + 15);

    ck_assert_false(seen[key]);
    seen[key] = true;

    count++;
  }

  ck_assert_uint_eq(count, 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(seen[i]);
  }

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_has_next_does_not_advance) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_has_next(&iter));

  size_t count = 0;

  while (cashash_iter_has_next(&iter)) {
    ck_assert(cashash_iter_next(&iter, &pair));
    count++;
  }

  ck_assert_uint_eq(count, 10);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_can_drive_iteration_without_has_next) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  size_t count = 0;

  while (cashash_iter_next(&iter, &pair)) {
    count++;
  }

  ck_assert_uint_eq(count, 10);
  ck_assert_false(cashash_iter_next(&iter, &pair));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_null_arguments) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert_false(cashash_iter_has_next(NULL));
  ck_assert_false(cashash_iter_next(NULL, &pair));
  ck_assert_false(cashash_iter_next(&iter, NULL));

  cashash_iter_init(map, NULL);

  cashash_destroy(map);
}
END_TEST

Suite *cashash_iter_suite(void) {
  Suite *suite = suite_create("cashash_iter");

  TCase *core = tcase_create("core");

  tcase_add_test(core, test_iter_empty_table);
  tcase_add_test(core, test_iter_single_entry);
  tcase_add_test(core, test_iter_multiple_entries);
  tcase_add_test(core, test_iter_has_next_does_not_advance);
  tcase_add_test(core, test_iter_next_can_drive_iteration_without_has_next);
  tcase_add_test(core, test_iter_null_arguments);

  suite_add_tcase(suite, core);

  return suite;
}

int main(void) {
  Suite *suite = cashash_iter_suite();
  SRunner *runner = srunner_create(suite);

  srunner_run_all(runner, CK_NORMAL);

  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? 0 : 1;
}
