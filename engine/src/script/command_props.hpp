#pragma once

#include "token.hpp"
#include <string>

namespace scr {

struct CommandProps {
	const size_t args_n;
	const std::vector<TokenKind> arg_types;

	CommandProps() :
		args_n(0), arg_types({ })
	{ }
	CommandProps(std::initializer_list<TokenKind> arg_types) :
		args_n(arg_types.size()), arg_types(arg_types)
	{ }
	CommandProps(TokenKind type) :
		args_n(1), arg_types({type})
	{ }
};

const CommandProps& get_command_prop(const std::string& cmd); 

} // namespace scr
