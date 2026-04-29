#pragma once

#define ENABLE_LOG

#ifdef ENABLE_LOG
	#include <source_location>
	#include <string_view>
	#include <iostream>

	#define LOG_INFO(MSG) ::core::log_stdout("INFO", MSG)
	#define LOG_WARN(MSG) ::core::log_stdout("WARN", MSG)
	#define LOG_ERR(MSG)  ::core::log_stdout("ERROR", MSG)
	#define BUG(MSG)  ::core::log_stdout("BUG", MSG)
#else
	#define LOG_INFO(MSG) ((void)0)
	#define LOG_WARN(MSG) ((void)0)
	#define LOG_ERR(MSG)  ((void)0)
	#define BUG(MSG)  ((void)0)
#endif // #ifdef LOG_ENABLE

#ifdef ENABLE_LOG

namespace core {

inline void log_stdout(
	const std::string_view prefix,
	const std::string_view msg,
	std::source_location location = std::source_location::current()) {
	std::cout 
		<< '('
		<< prefix
		<< ") "
		<< location.file_name() 
		<< " - " 
		<< location.function_name() 
		<< ", " << location.line() << ": " << msg << '\n';
}

} // namespace io

#endif // #ifdef ENABLE_LOG
