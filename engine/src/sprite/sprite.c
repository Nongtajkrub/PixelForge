#include "sprite.h"

#include "sprite_instance.h"
#include "sprite_looks.h"

sprite_t spr_load(char* data) {
	sprite_t buf;
	char* ptr = data;

	buf.looks = spr_looks_load(ptr);
	buf.script = fscript_pkg_load(ptr);
	buf.instances = vec_new(sizeof(sprite_instance_t), NULL, NULL);

	return buf;
}
