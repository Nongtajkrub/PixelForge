#include <iostream>
#include <fstream>

#include "script/symbol_table.hpp"
#include "script/lexer.hpp"
#include "script/ast.hpp"
#include "script/parser.hpp"
#include "script/preprocessor.hpp"
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
			std::cout 
				<< scr::token_kind_as_str(token.kind) 
				<< ", " << token.lexeme.value() << '\n'; 
		} else {
			std::cout << scr::token_kind_as_str(token.kind)<< '\n'; 
		}
	}
	*/

	auto symbols = scr::SymbolTable(); 
	auto arena = BumpArena(320000); 

	auto preprocessor = scr::Preprocessor(lexer.get_token(), symbols, std::cout); 

	if (!preprocessor.process()) {
		std::cout << "Preprocessor fail\n";
		return -1;
	}

	auto parser = scr::Parser(lexer.get_token(), symbols, arena, std::cout);
	if (!parser.parse()) {
		std::cout << "Prase Fail" << '\n';
		return -1;
	}

	auto file = std::ofstream("ast_output.txt"); 
	std::println("size; {}", parser.get_ast().size());

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
