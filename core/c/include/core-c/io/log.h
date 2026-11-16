#pragma once

#ifdef __cplusplus
extern "C" {
#endif // #ifdef __cplusplus

#define ENABLE_LOG

#ifdef ENABLE_LOG
    #include <stdio.h>

    static inline void core_log_stdout(
		const char* prefix,
		const char* msg,
		const char* file_name, const char* function_name, int line) {
		printf(
			"(%s) %s - %s, %d: %s\n",
			prefix, file_name, function_name, line, msg);
    }

    #define LOG_INFO(msg_) core_log_stdout("INFO", msg_, __FILE__, __func__, __LINE__)
    #define LOG_WARN(msg_) core_log_stdout("WARN", msg_, __FILE__, __func__, __LINE__)
    #define LOG_ERR(msg_) core_log_stdout("ERROR", msg_, __FILE__, __func__, __LINE__)
    #define BUG(msg_) core_log_stdout("BUG", msg_, __FILE__, __func__, __LINE__)

#else
    #define LOG_INFO(msg_) ((void)0)
    #define LOG_WARN(msg_) ((void)0)
    #define LOG_ERR(msg_) ((void)0)
    #define BUG(msg_) ((void)0)
#endif // #ifdef ENABLE_LOG 

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
