#pragma once

#ifndef CASHASH_CHAIN_H
#define CASHASH_CHAIN_H

#include <cashash.c/cashash_type.h>

bool cashash_chain_insert(cashash_t *table, const cashash_key_datum_t key,
                          void *data);

bool cashash_chain_remove(cashash_t *table, const cashash_key_datum_t key);

void *cashash_chain_find(const cashash_t *table, const cashash_key_datum_t key);

void cashash_chain_clear(cashash_t *table);

void cashash_chain_destroy(cashash_t * table);

#endif
