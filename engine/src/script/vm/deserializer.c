#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#include "deserializer.h"

#include "../../core/c/container/block.h"
#include "../specs.h"

#include <stdlib.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>

static inline package_t pack_new(size_t size) {
	return (package_t) {
		.size = size,
		.cpool = {
			.addr = {
				.size = 0,
				.map = vec_new(WORD_SIZE),
			},
			.entries = {
				.size = 0,
				.data = vec_new(sizeof(block_t)),
			}
		},
		.code = {
			.size = 0,
			.inst = vec_new(WORD_SIZE),
			.func = vec_new(WORD_SIZE),
		}
	};
}

package_t deserialize(char* bytes, size_t size) {
	package_t pack = pack_new(size);
	char* ptr = bytes;

	// deserialize address map section.
	memcpy(&pack.cpool.addr.size, ptr, WORD_SIZE);
	ptr += WORD_SIZE;

	const char* addr_end = ptr + pack.cpool.addr.size;
	while (ptr < addr_end) {
		word_t* entry = (word_t*)vec_push_null(&pack.cpool.addr.map);
		memcpy(entry, ptr, WORD_SIZE);
		ptr += WORD_SIZE;
	}

	// Deserialize entries section.
	memcpy(&pack.cpool.entries.size, ptr, WORD_SIZE);
	ptr += WORD_SIZE;

	const char* entries_end = ptr + pack.cpool.entries.size;
	while (ptr < entries_end) {
		block_t* entry = (block_t*)vec_push_null(&pack.cpool.entries.data);

		memcpy(&entry->size, ptr, WORD_SIZE);
		ptr += WORD_SIZE;

		entry->data = (char*)malloc(entry->size);
		memcpy(entry->data, ptr, entry->size);
		ptr += entry->size;
	}

	printf("const pool size %zu\n", pack.cpool.addr.size + pack.cpool.entries.size);
	printf("address map count %zu\n", pack.cpool.addr.map.size);
	printf("const pool entries count %zu\n", pack.cpool.entries.data.size);

	return pack;
}

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
