#pragma once

#ifndef CASHASH_OPENADDRESSING_H
#define CASHASH_OPENADDRESSING_H

#include <cashash.c/cashash_type.h>

bool cashash_oa_insert(cashash_t *table, const cashash_key_datum_t key,
                          void *data);

bool cashash_oa_remove(cashash_t *table, const cashash_key_datum_t key);

void *cashash_oa_find(const cashash_t *table, const cashash_key_datum_t key);

void cashash_oa_clear(cashash_t *table);

#endif
