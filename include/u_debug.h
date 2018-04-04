/******************************************************************************
* Copyright 2017 James Fitzpatrick <james_fitzpatrick@outlook.com>           *
*                                                                            *
* Permission is hereby granted, free of charge, to any person obtaining a    *
* copy of this software and associated documentation files (the "Software"), *
* to deal in the Software without restriction, including without limitation  *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
* and/or sell copies of the Software, and to permit persons to whom the      *
* Software is furnished to do so, subject to the following conditions:       *
*                                                                            *
* The above copyright notice and this permission notice shall be included in *
* all copies or substantial portions of the Software.                        *
*                                                                            *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
* DEALINGS IN THE SOFTWARE.                                                  *
******************************************************************************/

#pragma once

typedef enum _LOG_LEVEL_
{
	LOG_LEVEL_FATAL,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARN,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
} LOG_LEVEL;

#ifdef ENABLE_DEBUG_LOGGING
#define LOG(level, msg, ...) debug_log(level, msg, ##__VA_ARGS__)

void debug_log(LOG_LEVEL level, char *msg, ...);
#else
#define LOG(level, msg, ...)
#endif

#define LOG_FATAL(msg, ...) LOG(LOG_LEVEL_FATAL, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...) LOG(LOG_LEVEL_ERROR, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...) LOG(LOG_LEVEL_WARN, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...) LOG(LOG_LEVEL_INFO, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) LOG(LOG_LEVEL_DEBUG, msg, ##__VA_ARGS__)

#ifdef ENABLE_DEBUG_ASSERT
#define DEBUG_ASSERT(b) do {                                                     \
		if (!(b)) {                                                      \
			debug_assert_fail(#b, __FUNCTION__, __FILE__, __LINE__); \
		}                                                                \
	} while(0)

void debug_assert_fail(const char * b, const char *func, const char *file, int line);
#else
#define DEBUG_ASSERT(b)
#endif

