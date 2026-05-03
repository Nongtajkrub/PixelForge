#include "fscript_interpreter.h"

#include "../../core/c/io/vec_stream.h"
#include "../../core/c/io/log.h"
#include "fscript_instruction.h"
#include "fscript_mem.h"

#include <stdlib.h>
#include <stdio.h>

void interpret(package_t* pkg) {
	vec_stream_t code = vstream_make(&pkg->code.main.data);
	memory_t memory = mem_new();

	while (!vstream_end(&code)) {
		const opcode_t op = *(opcode_t*)vstream_advance(&code);

		switch (op) {
		case OP_CONST: {
			printf("Processing OP_CONST\n");
			const word_t index = *(word_t*)vstream_advance(&code);
			printf("Const index: %d\n", index);

			const block_t* c = (block_t*)vec_get(&pkg->cpool.data, index);
			printf("Load const!\n");

			mem_push(&memory, c->data, c->size);
			printf("Pushed memory\n");

			break;
		}
		case OP_STORE: {
			printf("Processing OP_STORE\n");
			const word_t slot = *(word_t*)vstream_advance(&code);
			printf("Slot: %d\n", slot);
				
			const block_t data = mem_pop(&memory);
			printf("Pop stack!\n");

			mem_store(&memory, data.data, data.size, slot);
			printf("Stored memory\n");

			break;
		}
		default:
			BUG("Unimplemented opcode.");
			exit(1);
		}
	}
}
