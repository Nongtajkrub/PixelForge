#include "vm_interpreter.h"

vm_interpreter_t vm_inter_new(pkg_script_cpool_t* cpool, pkg_sprite_t* sprite) {
	return (vm_interpreter_t) {
		.cpool = cpool,
		.sprite = sprite,
		.pc = 0,
	};
}

void vm_inter_tick(vm_interpreter_t* inter) {

}
