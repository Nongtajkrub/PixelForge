#include <core-cplusplus/io/byte_io.hpp>

#include "fscript_packer.hpp"

#include <span>

namespace scr {


CodePackage pack_script(std::span<const u8> cpool, std::span<const u8> code) {
	CodePackage buf;
	auto io = core::BytesBufferWriter(buf);

	io.extend(cpool);
	io.extend(code);

	return buf;
}

} // namespace scr
