#pragma once

#ifdef ENABLE_LOG
	#include <source_location>
	#include <string_view>
	#include <iostream>

	#define LOG_INFO(MSG) ::core::log_info(MSG)
	#define LOG_WARN(MSG) ::core::log_warn(MSG)
	#define LOG_ERR(MSG)  ::core::log_error(MSG)
#else
	#define LOG_INFO(MSG) ((void)0)
	#define LOG_WARN(MSG) ((void)0)
	#define LOG_ERR(MSG)  ((void)0)
#endif // #ifdef LOG_ENABLE

#ifdef ENABLE_LOG

namespace core {

inline void log_info(
	const std::string_view msg,
	std::source_location location = std::source_location::current()) {
	std::cout 
		<< "(INFO) "
		<< location.file_name() 
		<< " - " 
		<< location.function_name() 
		<< ", " << location.line() << ": " << msg << '\n';
}

inline void log_warn(
	const std::string_view msg,
	std::source_location location = std::source_location::current()) {
	std::cout 
		<< "(WARN) "
		<< location.file_name() 
		<< " - " 
		<< location.function_name() 
		<< ", " << location.line() << ": " << msg << '\n';
}

inline void log_error(
	const std::string_view msg,
	std::source_location location = std::source_location::current()) {
	std::cout 
		<< "(ERROR) "
		<< location.file_name() 
		<< " - " 
		<< location.function_name() 
		<< ", " << location.line() << ": " << msg << '\n';
}

} // namespace io

#endif // #ifdef ENABLE_LOG
