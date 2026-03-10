#include <iostream>

#include "script/lexer.hpp"
#include "script/ast.hpp"
#include "script/parser.hpp"
#include "util/bump_arena.hpp"
#include "io/file.hpp"

int main() {
	const auto code = io::file::load_str("script.gby"); 

	if (!code.has_value()) {
		std::cout << "No code file.\n";
		return 1;
	}

	auto lexer = scr::Lexer(*code);

	/*
	for (const auto token : lexer.get_token()) {
		if (token.lexeme.has_value()) {
			std::cout << token.kind_as_str() << ", " << token.lexeme.value() << '\n'; 
		} else {
			std::cout << token.kind_as_str() << '\n'; 
		}
	}
	*/

	auto arena = BumpArena(3200); 

	auto parser = scr::Parser(lexer.get_token(), arena, std::cout);
	if (!parser.parse()) {
		std::cout << "Prase Fail" << '\n';
		return -1;
	}

	for (auto ast : parser.get_ast()) {
		scr::ast_output(std::cout, ast);
	}

	/*
	auto inner_expression = 
		reinterpret_cast<const scr::BinaryExpr*>(expression->right.adr);

	std::cout << "Inner lvel op: " << inner_expression->op.kind_as_str() << '\n';
	*/
}
