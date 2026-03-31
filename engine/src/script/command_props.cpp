#include "command_props.hpp"
#include "token.hpp"

#include <cassert>
#include <unordered_map>

namespace scr {

static const std::unordered_map<std::string, CommandProps> properties = {
	{CMD_UP_LEX, TokenKind::INT_T},
	{CMD_DOWN_LEX, TokenKind::INT_T},
	{CMD_RIGHT_LEX, TokenKind::INT_T},
	{CMD_LEFT_LEX, TokenKind::INT_T},
	{CMD_GOTO_LEX, {TokenKind::INT_T, TokenKind::INT_T}},
	{CMD_SPAWN_LEX, {}},
	{CMD_DESPAWN_LEX, {}},
	{CMD_SHOW_LEX, {}},
	{CMD_UPDATE_LEX, TokenKind::VOID_T},
	{CMD_COLLIDE_LEX, {TokenKind::SPRITE_T, TokenKind::VOID_T}},
};

const CommandProps& get_command_prop(const std::string& cmd) {
	auto it = properties.find(cmd);

	// Whether a command exist should already be checked during parsing.
	assert(it != properties.end());

	return it->second;
} 

} // namespace scr
