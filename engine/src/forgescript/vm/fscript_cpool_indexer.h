#pragma once

#include <core-c/container/mem_sector.h>

typedef struct {
	mem_sector_t cpool;
} cpool_indexer_t;

void cpool_indexer_new();
