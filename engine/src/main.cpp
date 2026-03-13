#include <iostream>
#include <fstream>

#include "script/lexer/lexer.hpp"
#include "script/ast/ast.hpp"
#include "script/parser/parser.hpp"
#include "core/arena/bump_arena.hpp"
#include "core/io/file.hpp"

using namespace core;

int main() {
	const auto code = fload_str("script.gby"); 

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

	auto arena = BumpArena(8000); 

	auto parser = scr::Parser(lexer.get_token(), arena, std::cout);
	if (!parser.parse()) {
		std::cout << "Prase Fail" << '\n';
		return -1;
	}

	auto file = std::ofstream("ast_output.txt"); 

	for (auto ast : parser.get_ast()) {
		scr::ast_output(std::cout, ast);
	}

	file.close();

	/*
	auto inner_expression = 
		reinterpret_cast<const scr::BinaryExpr*>(expression->right.adr);

	std::cout << "Inner lvel op: " << inner_expression->op.kind_as_str() << '\n';
	*/
}
