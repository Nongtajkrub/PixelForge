#pragma once

#include "symbol_table.hpp"
#include "token.hpp"

#include <ostream>
#include <vector>

namespace scr {

class Preprocessor {
private:
	MutTokenStream tokens;
	SymbolTable& symbols;

	std::ostream& err_stream;

	// Name of the sprite that own the script.
	std::string script_sprite;

public:
	Preprocessor(
		std::vector<Token>& tokens, 
		SymbolTable& symbols, std::ostream& err_stream) :
		tokens(tokens),
		symbols(symbols),
		err_stream(err_stream)
	{ }

	bool process();

private:
	bool process_direct();
	bool process_sprite_direct();
	bool process_use_direct();
	void process_self_direct();
	void process_update_direct();
	void process_collide_direct();
};

} // namespace scr
