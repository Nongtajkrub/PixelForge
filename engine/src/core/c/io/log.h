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

    #define LOG_INFO(MSG) core_log_stdout("INFO", MSG, __FILE__, __func__, __LINE__)
    #define LOG_WARN(MSG) core_log_stdout("WARN", MSG, __FILE__, __func__, __LINE__)
    #define LOG_ERR(MSG) core_log_stdout("ERROR", MSG, __FILE__, __func__, __LINE__)
    #define BUG(MSG) core_log_stdout("BUG", MSG, __FILE__, __func__, __LINE__)

#else
    #define LOG_INFO(MSG) ((void)0)
    #define LOG_WARN(MSG) ((void)0)
    #define LOG_ERR(MSG) ((void)0)
    #define BUG(MSG) ((void)0)
#endif // #ifdef ENABLE_LOG 

#ifdef __cplusplus
} // extern "C"
#endif // #ifdef __cplusplus
