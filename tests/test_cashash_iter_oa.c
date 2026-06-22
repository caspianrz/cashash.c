#include <cashash.c/cashash.h>
#include <cashash.c/cashash_iter.h>

#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CASHASH_KEY(data_, length_)                                            \
  ((cashash_key_datum_t){.data = (data_), .length = (length_)})

typedef struct cashash_oa_iter_seen_ctx_s {
  bool seen[10];
  size_t count;
} cashash_oa_iter_seen_ctx_t;

typedef struct cashash_oa_iter_stop_ctx_s {
  size_t count;
  size_t stop_after;
} cashash_oa_iter_stop_ctx_t;

static int cashash_oa_test_key_to_int(cashash_key_datum_t key) {
  int value = 0;

  ck_assert(key.data != NULL);
  ck_assert(key.length == sizeof(int));

  memcpy(&value, key.data, sizeof(value));

  return value;
}

static void cashash_oa_test_insert_int_items(cashash_t *map, int *keys,
                                             int *values, size_t count) {
  for (size_t i = 0; i < count; i++) {
    keys[i] = (int)i;
    values[i] = (int)i + 100;

    ck_assert(cashash_insert(map, CASHASH_KEY(&keys[i], sizeof(keys[i])),
                             &values[i]));
  }
}

static void cashash_oa_test_assert_pair(cashash_pair_t pair, bool *seen,
                                        size_t seen_count) {
  int key = cashash_oa_test_key_to_int(pair.key);

  ck_assert_int_ge(key, 0);
  ck_assert((size_t)key < seen_count);
  ck_assert(pair.value != NULL);

  ck_assert_int_eq(*(int *)pair.value, key + 100);
  ck_assert(!seen[key]);

  seen[key] = true;
}

static bool cashash_oa_foreach_seen_callback(cashash_pair_t pair,
                                             void *user_data) {
  cashash_oa_iter_seen_ctx_t *ctx = user_data;

  cashash_oa_test_assert_pair(pair, ctx->seen, 10);
  ctx->count++;

  return true;
}

static bool cashash_oa_foreach_stop_callback(cashash_pair_t pair,
                                             void *user_data) {
  (void)pair;

  cashash_oa_iter_stop_ctx_t *ctx = user_data;

  ctx->count++;

  return ctx->count < ctx->stop_after;
}

static bool cashash_oa_foreach_key_seen_callback(cashash_key_datum_t key,
                                                 void *user_data) {
  cashash_oa_iter_seen_ctx_t *ctx = user_data;
  int key_value = cashash_oa_test_key_to_int(key);

  ck_assert_int_ge(key_value, 0);
  ck_assert_int_lt(key_value, 10);
  ck_assert(!ctx->seen[key_value]);

  ctx->seen[key_value] = true;
  ctx->count++;

  return true;
}

static bool cashash_oa_foreach_key_stop_callback(cashash_key_datum_t key,
                                                 void *user_data) {
  (void)key;

  cashash_oa_iter_stop_ctx_t *ctx = user_data;

  ctx->count++;

  return ctx->count < ctx->stop_after;
}

START_TEST(test_oa_iter_empty_table) {
  cashash_t *map = cashash_create_open_addressing(16);

  ck_assert_ptr_nonnull(map);

  cashash_iter_t iter;
  cashash_pair_t pair;
  cashash_key_datum_t key;

  cashash_iter_init(map, &iter);

  ck_assert(!cashash_iter_has_next(&iter));
  ck_assert(!cashash_iter_next(&iter, &pair));
  ck_assert(!cashash_iter_next_key(&iter, &key));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_iter_single_entry) {
  cashash_t *map = cashash_create_open_addressing(16);

  int key_data = 7;
  int value_data = 107;

  ck_assert_ptr_nonnull(map);

  ck_assert(cashash_insert(map, CASHASH_KEY(&key_data, sizeof(key_data)),
                           &value_data));

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert(cashash_iter_has_next(&iter));
  ck_assert(cashash_iter_next(&iter, &pair));

  ck_assert_int_eq(cashash_oa_test_key_to_int(pair.key), 7);
  ck_assert_ptr_eq(pair.value, &value_data);
  ck_assert_int_eq(*(int *)pair.value, 107);

  ck_assert(!cashash_iter_has_next(&iter));
  ck_assert(!cashash_iter_next(&iter, &pair));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_iter_multiple_entries) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];
  bool seen[10] = {0};
  size_t count = 0;

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  while (cashash_iter_has_next(&iter)) {
    ck_assert(cashash_iter_next(&iter, &pair));
    cashash_oa_test_assert_pair(pair, seen, 10);
    count++;
  }

  ck_assert(count == 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(seen[i]);
  }

  ck_assert(!cashash_iter_next(&iter, &pair));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_iter_skips_removed_entries) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];
  bool seen[10] = {0};
  size_t count = 0;

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  ck_assert(cashash_remove(map, CASHASH_KEY(&keys[2], sizeof(keys[2]))));
  ck_assert(cashash_remove(map, CASHASH_KEY(&keys[5], sizeof(keys[5]))));
  ck_assert(cashash_remove(map, CASHASH_KEY(&keys[7], sizeof(keys[7]))));

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  while (cashash_iter_has_next(&iter)) {
    ck_assert(cashash_iter_next(&iter, &pair));

    int key = cashash_oa_test_key_to_int(pair.key);

    ck_assert_int_ne(key, 2);
    ck_assert_int_ne(key, 5);
    ck_assert_int_ne(key, 7);

    cashash_oa_test_assert_pair(pair, seen, 10);
    count++;
  }

  ck_assert(count == 7);

  ck_assert(seen[0]);
  ck_assert(seen[1]);
  ck_assert(!seen[2]);
  ck_assert(seen[3]);
  ck_assert(seen[4]);
  ck_assert(!seen[5]);
  ck_assert(seen[6]);
  ck_assert(!seen[7]);
  ck_assert(seen[8]);
  ck_assert(seen[9]);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_iter_after_clear) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[5];
  int values[5];

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 5);

  cashash_clear(map);

  cashash_iter_t iter;
  cashash_pair_t pair;

  cashash_iter_init(map, &iter);

  ck_assert(!cashash_iter_has_next(&iter));
  ck_assert(!cashash_iter_next(&iter, &pair));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_iter_next_key_multiple_entries) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];
  bool seen[10] = {0};
  size_t count = 0;

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  cashash_iter_t iter;
  cashash_key_datum_t key;

  cashash_iter_init(map, &iter);

  while (cashash_iter_has_next(&iter)) {
    ck_assert(cashash_iter_next_key(&iter, &key));

    int key_value = cashash_oa_test_key_to_int(key);

    ck_assert_int_ge(key_value, 0);
    ck_assert_int_lt(key_value, 10);
    ck_assert(!seen[key_value]);

    seen[key_value] = true;
    count++;
  }

  ck_assert(count == 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(seen[i]);
  }

  ck_assert(!cashash_iter_next_key(&iter, &key));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_iter_null_arguments) {
  cashash_t *map = cashash_create_open_addressing(16);

  ck_assert_ptr_nonnull(map);

  cashash_iter_t iter;
  cashash_pair_t pair;
  cashash_key_datum_t key;

  cashash_iter_init(NULL, &iter);

  ck_assert(!cashash_iter_has_next(&iter));
  ck_assert(!cashash_iter_next(&iter, &pair));
  ck_assert(!cashash_iter_next_key(&iter, &key));

  cashash_iter_init(map, NULL);

  cashash_iter_init(map, &iter);

  ck_assert(!cashash_iter_next(NULL, &pair));
  ck_assert(!cashash_iter_next(&iter, NULL));
  ck_assert(!cashash_iter_next_key(NULL, &key));
  ck_assert(!cashash_iter_next_key(&iter, NULL));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_foreach_multiple_entries) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];

  cashash_oa_iter_seen_ctx_t ctx = {0};

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  ck_assert(cashash_foreach(map, cashash_oa_foreach_seen_callback, &ctx));

  ck_assert(ctx.count == 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(ctx.seen[i]);
  }

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_foreach_can_stop_early) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];

  cashash_oa_iter_stop_ctx_t ctx = {
      .count = 0,
      .stop_after = 3,
  };

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  ck_assert(!cashash_foreach(map, cashash_oa_foreach_stop_callback, &ctx));
  ck_assert(ctx.count == 3);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_foreach_null_arguments) {
  cashash_t *map = cashash_create_open_addressing(16);

  ck_assert_ptr_nonnull(map);

  ck_assert(!cashash_foreach(NULL, cashash_oa_foreach_seen_callback, NULL));
  ck_assert(!cashash_foreach(map, NULL, NULL));

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_foreach_key_multiple_entries) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];

  cashash_oa_iter_seen_ctx_t ctx = {0};

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  ck_assert(
      cashash_foreach_key(map, cashash_oa_foreach_key_seen_callback, &ctx));

  ck_assert(ctx.count == 10);

  for (size_t i = 0; i < 10; i++) {
    ck_assert(ctx.seen[i]);
  }

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_foreach_key_can_stop_early) {
  cashash_t *map = cashash_create_open_addressing(16);

  int keys[10];
  int values[10];

  cashash_oa_iter_stop_ctx_t ctx = {
      .count = 0,
      .stop_after = 3,
  };

  ck_assert_ptr_nonnull(map);

  cashash_oa_test_insert_int_items(map, keys, values, 10);

  ck_assert(
      !cashash_foreach_key(map, cashash_oa_foreach_key_stop_callback, &ctx));
  ck_assert(ctx.count == 3);

  cashash_destroy(map);
}
END_TEST

START_TEST(test_oa_foreach_key_null_arguments) {
  cashash_t *map = cashash_create_open_addressing(16);

  ck_assert_ptr_nonnull(map);

  ck_assert(
      !cashash_foreach_key(NULL, cashash_oa_foreach_key_seen_callback, NULL));
  ck_assert(!cashash_foreach_key(map, NULL, NULL));

  cashash_destroy(map);
}
END_TEST

static Suite *cashash_oa_suite(void) {
  Suite *suite = suite_create("cashash_open_addressing_iteration");
  TCase *core = tcase_create("core");

  tcase_add_test(core, test_oa_iter_empty_table);
  tcase_add_test(core, test_oa_iter_single_entry);
  tcase_add_test(core, test_oa_iter_multiple_entries);
  tcase_add_test(core, test_oa_iter_skips_removed_entries);
  tcase_add_test(core, test_oa_iter_after_clear);
  tcase_add_test(core, test_oa_iter_next_key_multiple_entries);
  tcase_add_test(core, test_oa_iter_null_arguments);

  tcase_add_test(core, test_oa_foreach_multiple_entries);
  tcase_add_test(core, test_oa_foreach_can_stop_early);
  tcase_add_test(core, test_oa_foreach_null_arguments);

  tcase_add_test(core, test_oa_foreach_key_multiple_entries);
  tcase_add_test(core, test_oa_foreach_key_can_stop_early);
  tcase_add_test(core, test_oa_foreach_key_null_arguments);

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
