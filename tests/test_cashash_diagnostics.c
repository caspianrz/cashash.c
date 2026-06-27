#include <check.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cashash.c/cashash.h>
#include <cashash.c/cashash_diagnostics.h>
#include <cashash.c/cashash_type.h>

static cashash_key_datum_t test_key(const char *s) {
  return (cashash_key_datum_t){
      .data = s,
      .length = strlen(s),
  };
}

static void *test_value(const char *s) { return (void *)s; }

START_TEST(test_validate_null_table) {
  cashash_validate_result_t result;

  bool ok = cashash_validate(NULL, &result);

  ck_assert(!ok);
  ck_assert(!result.valid);
  ck_assert_int_eq(result.error, CASHASH_VALIDATE_NULL_TABLE);
}
END_TEST

START_TEST(test_validate_null_result_is_allowed) {
  cashash_t *table = cashash_create(8);

  ck_assert_ptr_ne(table, NULL);

  bool ok = cashash_validate(table, NULL);

  ck_assert(ok);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_validate_empty_table) {
  cashash_t *table = cashash_create(8);

  ck_assert_ptr_ne(table, NULL);

  cashash_validate_result_t result;

  bool ok = cashash_validate(table, &result);

  ck_assert(ok);
  ck_assert(result.valid);
  ck_assert_int_eq(result.error, CASHASH_VALIDATE_OK);
  ck_assert_uint_eq(result.bucket_index, 0);
  ck_assert_uint_eq(result.entry_index, 0);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_stats_null_table) {
  cashash_stats_t stats;

  bool ok = cashash_stats(NULL, &stats);

  ck_assert(!ok);
}
END_TEST

START_TEST(test_stats_null_stats) {
  cashash_t *table = cashash_create(8);

  ck_assert_ptr_ne(table, NULL);

  bool ok = cashash_stats(table, NULL);

  ck_assert(!ok);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_stats_empty_table) {
  cashash_t *table = cashash_create(8);

  ck_assert_ptr_ne(table, NULL);

  cashash_stats_t stats;

  bool ok = cashash_stats(table, &stats);

  ck_assert(ok);

  ck_assert_uint_eq(stats.entries, 0);
  ck_assert_uint_eq(stats.capacity, 8);
  ck_assert_double_eq_tol(stats.load_factor, 0.0, 0.000001);
  ck_assert_uint_eq(stats.collisions, 0);

  ck_assert_uint_eq(stats.tombstones, 0);
  ck_assert_uint_eq(stats.max_probe_length, 0);
  ck_assert_double_eq_tol(stats.avg_probe_length, 0.0, 0.000001);

  ck_assert_uint_eq(stats.used_buckets, 0);
  ck_assert_uint_eq(stats.empty_buckets, 8);
  ck_assert_uint_eq(stats.longest_chain, 0);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_stats_chaining_collisions) {
  cashash_t *table = cashash_create(1);

  ck_assert_ptr_ne(table, NULL);

  ck_assert(cashash_insert(table, test_key("one"), test_value("1")));
  ck_assert(cashash_insert(table, test_key("two"), test_value("2")));
  ck_assert(cashash_insert(table, test_key("three"), test_value("3")));

  cashash_stats_t stats;

  bool ok = cashash_stats(table, &stats);

  ck_assert(ok);

  ck_assert_uint_eq(stats.entries, 3);
  ck_assert_uint_eq(stats.capacity, 4);
  ck_assert_double_eq_tol(stats.load_factor, 0.75, 0.000001);

  ck_assert_uint_eq(stats.collisions, 1);
  ck_assert_uint_eq(stats.used_buckets, 2);
  ck_assert_uint_eq(stats.empty_buckets, 2);
  ck_assert_uint_eq(stats.longest_chain, 2);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_validate_chaining_with_entries) {
  cashash_t *table = cashash_create(4);

  ck_assert_ptr_ne(table, NULL);

  ck_assert(cashash_insert(table, test_key("alpha"), test_value("a")));
  ck_assert(cashash_insert(table, test_key("beta"), test_value("b")));
  ck_assert(cashash_insert(table, test_key("gamma"), test_value("g")));

  cashash_validate_result_t result;

  bool ok = cashash_validate(table, &result);

  ck_assert(ok);
  ck_assert(result.valid);
  ck_assert_int_eq(result.error, CASHASH_VALIDATE_OK);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_validate_size_mismatch) {
  cashash_t *table = cashash_create(4);

  ck_assert_ptr_ne(table, NULL);

  ck_assert(cashash_insert(table, test_key("alpha"), test_value("a")));
  ck_assert(cashash_insert(table, test_key("beta"), test_value("b")));

  size_t original_size = table->storage.chain.size;

  table->storage.chain.size = original_size + 1;

  cashash_validate_result_t result;

  bool ok = cashash_validate(table, &result);

  ck_assert(!ok);
  ck_assert(!result.valid);
  ck_assert_int_eq(result.error, CASHASH_VALIDATE_SIZE_MISMATCH);

  table->storage.chain.size = original_size;

  cashash_destroy(table);
}
END_TEST

START_TEST(test_stats_print_basic_output) {
  cashash_stats_t stats = {
      .entries = 1204,
      .capacity = 2048,
      .load_factor = 0.587890625,
      .collisions = 331,

      .tombstones = 0,
      .max_probe_length = 0,
      .avg_probe_length = 0.0,

      .used_buckets = 900,
      .empty_buckets = 1148,
      .longest_chain = 4,
  };

  FILE *file = tmpfile();

  ck_assert_ptr_ne(file, NULL);

  cashash_stats_print(file, &stats);

  fflush(file);
  rewind(file);

  char buffer[1024];

  size_t read_count = fread(buffer, 1, sizeof(buffer) - 1, file);
  buffer[read_count] = '\0';

  ck_assert_ptr_ne(strstr(buffer, "Entries: 1204"), NULL);
  ck_assert_ptr_ne(strstr(buffer, "Capacity: 2048"), NULL);
  ck_assert_ptr_ne(strstr(buffer, "Load Factor: 0.59"), NULL);
  ck_assert_ptr_ne(strstr(buffer, "Collisions: 331"), NULL);
  ck_assert_ptr_ne(strstr(buffer, "Used Buckets: 900"), NULL);
  ck_assert_ptr_ne(strstr(buffer, "Empty Buckets: 1148"), NULL);
  ck_assert_ptr_ne(strstr(buffer, "Longest Chain: 4"), NULL);

  fclose(file);
}
END_TEST

START_TEST(test_stats_print_null_stream_does_not_crash) {
  cashash_stats_t stats = {
      .entries = 1,
      .capacity = 8,
      .load_factor = 0.125,
      .collisions = 0,
  };

  cashash_stats_print(NULL, &stats);
}
END_TEST

START_TEST(test_stats_print_null_stats_does_not_crash) {
  cashash_stats_print(stdout, NULL);
}
END_TEST

static cashash_t *create_linear_probing_table(size_t capacity) {
  cashash_strategy_option_t option = {0};

  return cashash_create_open_addressing_with_strategy(
      capacity, CASHASH_HASH_STRATEGY_FNV1A, option, CASHASH_OA_PROBE_LINEAR);
}

START_TEST(test_validate_linear_probing_with_entries) {
  cashash_t *table = create_linear_probing_table(16);

  ck_assert_ptr_ne(table, NULL);

  ck_assert(cashash_insert(table, test_key("alpha"), test_value("a")));
  ck_assert(cashash_insert(table, test_key("beta"), test_value("b")));
  ck_assert(cashash_insert(table, test_key("gamma"), test_value("g")));

  cashash_validate_result_t result;

  bool ok = cashash_validate(table, &result);

  ck_assert(ok);
  ck_assert(result.valid);
  ck_assert_int_eq(result.error, CASHASH_VALIDATE_OK);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_stats_linear_probing_basic) {
  cashash_t *table = create_linear_probing_table(16);

  ck_assert_ptr_ne(table, NULL);

  ck_assert(cashash_insert(table, test_key("alpha"), test_value("a")));
  ck_assert(cashash_insert(table, test_key("beta"), test_value("b")));
  ck_assert(cashash_insert(table, test_key("gamma"), test_value("g")));
  ck_assert(cashash_insert(table, test_key("delta"), test_value("d")));

  cashash_stats_t stats;

  bool ok = cashash_stats(table, &stats);

  ck_assert(ok);

  ck_assert_uint_eq(stats.entries, 4);
  ck_assert_uint_eq(stats.capacity, 16);
  ck_assert_double_eq_tol(stats.load_factor, 0.25, 0.000001);

  ck_assert_uint_eq(stats.used_buckets, 4);
  ck_assert_uint_eq(stats.empty_buckets + stats.used_buckets + stats.tombstones,
                    16);

  cashash_destroy(table);
}
END_TEST

START_TEST(test_stats_linear_probing_tombstones) {
  cashash_t *table = create_linear_probing_table(16);

  ck_assert_ptr_ne(table, NULL);

  ck_assert(cashash_insert(table, test_key("alpha"), test_value("a")));
  ck_assert(cashash_insert(table, test_key("beta"), test_value("b")));
  ck_assert(cashash_insert(table, test_key("gamma"), test_value("g")));

  ck_assert(cashash_remove(table, test_key("beta")));

  cashash_stats_t stats;

  bool ok = cashash_stats(table, &stats);

  ck_assert(ok);

  ck_assert_uint_eq(stats.entries, 2);
  ck_assert_uint_eq(stats.capacity, 16);

  ck_assert_uint_eq(stats.used_buckets, 2);
  ck_assert_uint_eq(stats.tombstones, 1);
  ck_assert_uint_eq(stats.empty_buckets + stats.used_buckets + stats.tombstones,
                    16);

  cashash_destroy(table);
}
END_TEST

static Suite *cashash_diagnostics_suite(void) {
  Suite *suite = suite_create("cashash_diagnostics");

  TCase *tc_validate = tcase_create("validate");
  TCase *tc_stats = tcase_create("stats");
  TCase *tc_print = tcase_create("print");
  TCase *tc_open_addressing = tcase_create("open_addressing");

  tcase_add_test(tc_validate, test_validate_null_table);
  tcase_add_test(tc_validate, test_validate_null_result_is_allowed);
  tcase_add_test(tc_validate, test_validate_empty_table);
  tcase_add_test(tc_validate, test_validate_chaining_with_entries);
  tcase_add_test(tc_validate, test_validate_size_mismatch);

  tcase_add_test(tc_stats, test_stats_null_table);
  tcase_add_test(tc_stats, test_stats_null_stats);
  tcase_add_test(tc_stats, test_stats_empty_table);
  tcase_add_test(tc_stats, test_stats_chaining_collisions);

  tcase_add_test(tc_print, test_stats_print_basic_output);
  tcase_add_test(tc_print, test_stats_print_null_stream_does_not_crash);
  tcase_add_test(tc_print, test_stats_print_null_stats_does_not_crash);

  tcase_add_test(tc_open_addressing, test_validate_linear_probing_with_entries);
  tcase_add_test(tc_open_addressing, test_stats_linear_probing_basic);
  tcase_add_test(tc_open_addressing, test_stats_linear_probing_tombstones);

  suite_add_tcase(suite, tc_validate);
  suite_add_tcase(suite, tc_stats);
  suite_add_tcase(suite, tc_print);
  suite_add_tcase(suite, tc_open_addressing);

  return suite;
}

int main(void) {
  Suite *suite = cashash_diagnostics_suite();
  SRunner *runner = srunner_create(suite);

  srunner_run_all(runner, CK_NORMAL);

  int failed = srunner_ntests_failed(runner);

  srunner_free(runner);

  return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
