#include <check.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include <cashash.c/cashash.h>
#include <cashash.c/cashash_iter.h>

#ifndef ck_assert_false
#define ck_assert_false(expr) ck_assert(!(expr))
#endif

typedef struct foreach_count_ctx {
  size_t count;
} foreach_count_ctx_t;

typedef struct foreach_seen_ctx {
  bool seen[10];
  size_t count;
} foreach_seen_ctx_t;

typedef struct foreach_stop_ctx {
  size_t count;
  size_t stop_after;
} foreach_stop_ctx_t;

typedef struct key_foreach_count_ctx {
  size_t count;
} key_foreach_count_ctx_t;

typedef struct key_foreach_seen_ctx {
  bool seen[10];
  size_t count;
} key_foreach_seen_ctx_t;

typedef struct key_foreach_stop_ctx {
  size_t count;
  size_t stop_after;
} key_foreach_stop_ctx_t;

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

    ck_assert(cashash_insert(table, key, &test_values[i]));
  }
}

/** Foreach callbacks */

static bool foreach_count_callback(cashash_pair_t pair, void *user_data) {
  (void)pair;
  foreach_count_ctx_t *ctx = user_data;
  ctx->count++;
  return true;
}

static bool foreach_seen_callback(cashash_pair_t pair, void *user_data) {
  foreach_seen_ctx_t *ctx = user_data;
  ck_assert_uint_eq(pair.key.length, sizeof(int));
  ck_assert_ptr_nonnull(pair.value);
  int key = datum_to_int(pair.key.data);
  int value = *(int *)pair.value;
  ck_assert_int_ge(key, 0);
  ck_assert_int_lt(key, 10);
  ck_assert_int_eq(value, key + 15);
  ck_assert(!ctx->seen[key]);
  ctx->seen[key] = true;
  ctx->count++;
  return true;
}

static bool foreach_stop_callback(cashash_pair_t pair, void *user_data) {
  (void)pair;
  foreach_stop_ctx_t *ctx = user_data;
  ctx->count++;
  return ctx->count < ctx->stop_after;
}

static bool foreach_add_ten_callback(cashash_pair_t pair, void *user_data) {
  (void)user_data;
  ck_assert_ptr_nonnull(pair.value);
  int *value = pair.value;
  *value += 10;
  return true;
}

static bool foreach_key_count_callback(cashash_key_datum_t key,
                                       void *user_data) {
  (void)key;
  key_foreach_count_ctx_t *ctx = user_data;
  ctx->count++;
  return true;
}

static bool foreach_key_seen_callback(cashash_key_datum_t key,
                                      void *user_data) {
  key_foreach_seen_ctx_t *ctx = user_data;
  ck_assert_uint_eq(key.length, sizeof(int));
  int key_value = datum_to_int(key.data);
  ck_assert_int_ge(key_value, 0);
  ck_assert_int_lt(key_value, 10);
  ck_assert(!ctx->seen[key_value]);
  ctx->seen[key_value] = true;
  ctx->count++;
  return true;
}

static bool foreach_key_stop_callback(cashash_key_datum_t key,
                                      void *user_data) {
  (void)key;
  key_foreach_stop_ctx_t *ctx = user_data;
  ctx->count++;
  return ctx->count < ctx->stop_after;
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

  ck_assert(cashash_insert(map, key, &value_data));

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_next(&iter, &pair));

  ck_assert_uint_eq(pair.key.length, sizeof key_data);
  ck_assert_ptr_nonnull(pair.value);

  ck_assert_int_eq(datum_to_int(pair.key.data), 42);
  ck_assert_int_eq(*(int *)pair.value, 57);

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
    ck_assert_ptr_nonnull(pair.value);

    int key = datum_to_int(pair.key.data);
    int value = *(int *)pair.value;

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

START_TEST(test_foreach_empty_table) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);
  foreach_count_ctx_t ctx = {
      .count = 0,
  };
  ck_assert(cashash_foreach(map, foreach_count_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 0);
  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_multiple_entries) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);
  populate_map(map);
  foreach_seen_ctx_t ctx = {0};
  ck_assert(cashash_foreach(map, foreach_seen_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 10);
  for (size_t i = 0; i < 10; i++) {
    ck_assert(ctx.seen[i]);
  }
  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_user_data_is_passed) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);
  populate_map(map);
  foreach_count_ctx_t ctx = {
      .count = 0,
  };
  ck_assert(cashash_foreach(map, foreach_count_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 10);
  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_can_stop_early) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);
  populate_map(map);
  foreach_stop_ctx_t ctx = {
      .count = 0,
      .stop_after = 3,
  };
  ck_assert(!cashash_foreach(map, foreach_stop_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 3);
  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_can_mutate_values) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);
  static const int keys[3] = {1, 2, 3};
  int values[3] = {10, 20, 30};
  for (size_t i = 0; i < 3; i++) {
    const cashash_key_datum_t key = {
        .data = &keys[i],
        .length = sizeof keys[i],
    };
    ck_assert(cashash_insert(map, key, &values[i]));
  }
  ck_assert(cashash_foreach(map, foreach_add_ten_callback, NULL));
  ck_assert_int_eq(values[0], 20);
  ck_assert_int_eq(values[1], 30);
  ck_assert_int_eq(values[2], 40);
  for (size_t i = 0; i < 3; i++) {
    const cashash_key_datum_t key = {
        .data = &keys[i],
        .length = sizeof keys[i],
    };
    int *value = cashash_find(map, key);
    ck_assert_ptr_nonnull(value);
    ck_assert_int_eq(*value, values[i]);
  }
  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_null_arguments) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);
  foreach_count_ctx_t ctx = {
      .count = 0,
  };
  ck_assert(!cashash_foreach(NULL, foreach_count_callback, &ctx));
  ck_assert(!cashash_foreach(map, NULL, &ctx));
  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_key_empty_table) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  cashash_iter_t iter;
  cashash_key_datum_t key;

  cashash_iter_init(map, &iter);

  ck_assert(!cashash_iter_next_key(&iter, &key));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_key_single_entry) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  static const int key_data = 42;
  static int value_data = 57;

  const cashash_key_datum_t key = {
      .data = &key_data,
      .length = sizeof key_data,
  };

  ck_assert(cashash_insert(map, key, &value_data));

  cashash_iter_t iter;
  cashash_key_datum_t out_key;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_next_key(&iter, &out_key));

  ck_assert_uint_eq(out_key.length, sizeof key_data);
  ck_assert_int_eq(datum_to_int(out_key.data), 42);

  ck_assert(!cashash_iter_next_key(&iter, &out_key));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_key_multiple_entries) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  bool seen[10] = {false};
  size_t count = 0;

  cashash_iter_t iter;
  cashash_key_datum_t key;

  cashash_iter_init(map, &iter);

  while (cashash_iter_next_key(&iter, &key)) {
    ck_assert_uint_eq(key.length, sizeof(int));

    int key_value = datum_to_int(key.data);

    ck_assert_int_ge(key_value, 0);
    ck_assert_int_lt(key_value, 10);

    ck_assert(!seen[key_value]);

    seen[key_value] = true;
    count++;
  }

  ck_assert_uint_eq(count, 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(seen[i]);
  }

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_key_after_has_next) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  cashash_iter_t iter;
  cashash_key_datum_t key;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_has_next(&iter));

  size_t count = 0;

  while (cashash_iter_has_next(&iter)) {
    ck_assert(cashash_iter_next_key(&iter, &key));
    count++;
  }

  ck_assert_uint_eq(count, 10);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_key_null_arguments) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  cashash_iter_t iter;
  cashash_key_datum_t key;

  cashash_iter_init(map, &iter);

  ck_assert(!cashash_iter_next_key(NULL, &key));
  ck_assert(!cashash_iter_next_key(&iter, NULL));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_key_empty_table) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  key_foreach_count_ctx_t ctx = {
      .count = 0,
  };

  ck_assert(cashash_foreach_key(map, foreach_key_count_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 0);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_key_multiple_entries) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  key_foreach_seen_ctx_t ctx = {0};

  ck_assert(cashash_foreach_key(map, foreach_key_seen_callback, &ctx));

  ck_assert_uint_eq(ctx.count, 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(ctx.seen[i]);
  }

  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_key_user_data_is_passed) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  key_foreach_count_ctx_t ctx = {
      .count = 0,
  };

  ck_assert(cashash_foreach_key(map, foreach_key_count_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 10);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_key_can_stop_early) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  populate_map(map);

  key_foreach_stop_ctx_t ctx = {
      .count = 0,
      .stop_after = 3,
  };

  ck_assert(!cashash_foreach_key(map, foreach_key_stop_callback, &ctx));
  ck_assert_uint_eq(ctx.count, 3);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_foreach_key_null_arguments) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  key_foreach_count_ctx_t ctx = {
      .count = 0,
  };

  ck_assert(!cashash_foreach_key(NULL, foreach_key_count_callback, &ctx));
  ck_assert(!cashash_foreach_key(map, NULL, &ctx));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_changing_returned_key_datum_does_not_change_map_key) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  int key_a = 42;
  int key_b = 99;
  int value = 123;

  const cashash_key_datum_t original_key = {
      .data = &key_a,
      .length = sizeof key_a,
  };

  const cashash_key_datum_t other_key = {
      .data = &key_b,
      .length = sizeof key_b,
  };

  ck_assert(cashash_insert(map, original_key, &value));

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_next(&iter, &pair));

  /*
   * This only changes the local copy of the key datum returned by iteration.
   * It must not modify the key stored inside the hash table.
   */
  pair.key = other_key;

  int *found_original = cashash_find(map, original_key);
  int *found_other = cashash_find(map, other_key);

  ck_assert_ptr_nonnull(found_original);
  ck_assert_int_eq(*found_original, 123);

  ck_assert_ptr_null(found_other);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_original_key_memory_change_does_not_change_stored_key) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  int key = 42;
  int value = 123;

  const cashash_key_datum_t insert_key = {
      .data = &key,
      .length = sizeof key,
  };

  ck_assert(cashash_insert(map, insert_key, &value));

  /*
   * The table should have copied the key internally.
   * Changing the original variable after insertion must not affect lookup.
   */
  key = 99;

  int old_key = 42;
  int new_key = 99;

  const cashash_key_datum_t old_lookup_key = {
      .data = &old_key,
      .length = sizeof old_key,
  };

  const cashash_key_datum_t new_lookup_key = {
      .data = &new_key,
      .length = sizeof new_key,
  };

  int *found_old = cashash_find(map, old_lookup_key);
  int *found_new = cashash_find(map, new_lookup_key);

  ck_assert_ptr_nonnull(found_old);
  ck_assert_int_eq(*found_old, 123);

  ck_assert_ptr_null(found_new);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_iter_next_key_returned_datum_is_only_local_copy) {
  cashash_t *map = cashash_create(8);
  ck_assert_ptr_nonnull(map);

  int key_a = 7;
  int key_b = 8;
  int value = 700;

  const cashash_key_datum_t original_key = {
      .data = &key_a,
      .length = sizeof key_a,
  };

  const cashash_key_datum_t other_key = {
      .data = &key_b,
      .length = sizeof key_b,
  };

  ck_assert(cashash_insert(map, original_key, &value));

  cashash_iter_t iter;
  cashash_key_datum_t iter_key;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_next_key(&iter, &iter_key));

  /*
   * Rebinding the returned datum should only affect this local variable.
   */
  iter_key = other_key;

  int *found_original = cashash_find(map, original_key);
  int *found_other = cashash_find(map, other_key);

  ck_assert_ptr_nonnull(found_original);
  ck_assert_int_eq(*found_original, 700);

  ck_assert_ptr_null(found_other);

  cashash_destroy(map);
}
END_TEST

Suite *cashash_iter_suite(void) {
  Suite *suite = suite_create("cashash_iter");

  TCase *iter_next_case = tcase_create("iter_next");
  TCase *foreach_case = tcase_create("foreach");
  TCase *iter_next_key_case = tcase_create("iter_next_key");
  TCase *foreach_key_case = tcase_create("foreach_key");

  tcase_add_test(iter_next_case, test_iter_empty_table);
  tcase_add_test(iter_next_case, test_iter_single_entry);
  tcase_add_test(iter_next_case, test_iter_multiple_entries);
  tcase_add_test(iter_next_case, test_iter_has_next_does_not_advance);
  tcase_add_test(iter_next_case,
                 test_iter_next_can_drive_iteration_without_has_next);
  tcase_add_test(iter_next_case, test_iter_null_arguments);

  tcase_add_test(foreach_case, test_foreach_empty_table);
  tcase_add_test(foreach_case, test_foreach_multiple_entries);
  tcase_add_test(foreach_case, test_foreach_user_data_is_passed);
  tcase_add_test(foreach_case, test_foreach_can_stop_early);
  tcase_add_test(foreach_case, test_foreach_can_mutate_values);
  tcase_add_test(foreach_case, test_foreach_null_arguments);

  tcase_add_test(iter_next_key_case, test_iter_next_key_empty_table);
  tcase_add_test(iter_next_key_case, test_iter_next_key_single_entry);
  tcase_add_test(iter_next_key_case, test_iter_next_key_multiple_entries);
  tcase_add_test(iter_next_key_case, test_iter_next_key_after_has_next);
  tcase_add_test(iter_next_key_case, test_iter_next_key_null_arguments);

  tcase_add_test(iter_next_key_case,
                 test_iter_changing_returned_key_datum_does_not_change_map_key);
  tcase_add_test(iter_next_key_case,
                 test_original_key_memory_change_does_not_change_stored_key);
  tcase_add_test(iter_next_key_case,
                 test_iter_next_key_returned_datum_is_only_local_copy);

  tcase_add_test(foreach_key_case, test_foreach_key_empty_table);
  tcase_add_test(foreach_key_case, test_foreach_key_multiple_entries);
  tcase_add_test(foreach_key_case, test_foreach_key_user_data_is_passed);
  tcase_add_test(foreach_key_case, test_foreach_key_can_stop_early);
  tcase_add_test(foreach_key_case, test_foreach_key_null_arguments);

  suite_add_tcase(suite, iter_next_case);
  suite_add_tcase(suite, foreach_case);

  suite_add_tcase(suite, iter_next_key_case);
  suite_add_tcase(suite, foreach_key_case);

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
