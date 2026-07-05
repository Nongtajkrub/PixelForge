#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "fscript_package.h"

#include "../../core/c/container/block.h"
#include "../fscript_specs.h"

#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>

static inline fscript_pkg_t pkg_new() {
	return (fscript_pkg_t) {
		.cpool = {
			.size = 0,
			.data = vec_new(sizeof(block_t), block_freefn, block_copyfn),
		},
		.code = {
			.main = {
				.size = 0,
				.data = vec_new(WORD_SIZE, NULL, NULL)
			},
			.func = {
				.size = 0,
				.data = vec_new(sizeof(vec_t), vec_freefn, vec_copyfn)
			},
			.updates = {
				.size = 0,
				.data = vec_new(sizeof(vec_t), vec_freefn, vec_copyfn)
			}
		}
	};
}

// TODO: Fix deserializing problem.
fscript_pkg_t fscript_pkg_load(char* bytes) {
	fscript_pkg_t pack = pkg_new();
	char* ptr = bytes;

	// Deserialize const entries section.
	memcpy(&pack.cpool.size, ptr, WORD_SIZE);
	ptr += WORD_SIZE;

	const char* entries_end = ptr + pack.cpool.size;
	while (ptr < entries_end) {
		block_t* entry = (block_t*)vec_push_null(&pack.cpool.data);

		memcpy(&entry->size, ptr, WORD_SIZE);
		ptr += WORD_SIZE;

		entry->data = (char*)malloc(entry->size);
		memcpy(entry->data, ptr, entry->size);
		ptr += entry->size;
	}

	// Deserialize main instructions section.
	memcpy(&pack.code.main.size, ptr, WORD_SIZE);
	ptr += WORD_SIZE;

	const char* main_end = ptr + pack.code.main.size;
	while (ptr < main_end) {
		memcpy(vec_push_null(&pack.code.main.data), ptr, WORD_SIZE);
		ptr += WORD_SIZE;
	}

	// Deserialize functions definition and implementation section.
	memcpy(&pack.code.func.size, ptr, WORD_SIZE);
	ptr += WORD_SIZE;

	const char* func_end = ptr + pack.code.func.size;
	while (ptr < func_end) {
		// Create new function entry.
		vec_t* entry = (vec_t*)vec_push_null(&pack.code.func.data);
		vec_init(entry, WORD_SIZE, NULL, NULL);

		// Get entry size.
		size_t entry_size;
		memcpy(&entry_size, ptr, WORD_SIZE);
		ptr += WORD_SIZE;

		// Read function body into entry.
		const char* entry_end = ptr + entry_size;
		while (ptr < entry_end) {
			memcpy(vec_push_null(entry), ptr, WORD_SIZE);
			ptr += WORD_SIZE;
		}
	}

	// Deserialize update functions section.
	memcpy(&pack.code.updates.size, ptr, WORD_SIZE);
	ptr += WORD_SIZE;

	const char* updates_end = ptr + pack.code.updates.size;
	while (ptr < updates_end) {
		vec_t* entry = (vec_t*)vec_push_null(&pack.code.updates.data);
		vec_init(entry, WORD_SIZE, NULL, NULL);

		size_t entry_size;
		memcpy(&entry_size, ptr, WORD_SIZE);
		ptr += WORD_SIZE;

		const char* entry_end = ptr + entry_size;
		while (ptr < entry_end) {
			memcpy(vec_push_null(entry), ptr, WORD_SIZE);
			ptr += WORD_SIZE;
		}
	}

	return pack;
}

block_t* fscript_pkg_cpool_get(const fscript_pkg_t* pack, word_t index) {
	return (block_t*)vec_get(&pack->cpool.data, index);
}

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
