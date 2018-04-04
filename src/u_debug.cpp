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

#include "u_debug.h"

#include <cstdio>
#include <cstdlib>
#include <stdarg.h>

#ifdef ENABLE_DEBUG_LOGGING

#define DEBUG_LOG_MSG_BUF_SIZE 256

void debug_log(LOG_LEVEL level, char* msg, ...)
{
	const char *level_msg;
	FILE *stream;

	switch (level)
	{
	case LOG_LEVEL_FATAL:
		level_msg = "FATAL";
		stream = stderr;
		break;
	case LOG_LEVEL_ERROR:
		level_msg = "ERROR";
		stream = stderr;
		break;
	case LOG_LEVEL_WARN:
		level_msg = "WARN";
		stream = stderr;
		break;
	case LOG_LEVEL_INFO:
		level_msg = "INFO";
		stream = stdout;
		break;
	case LOG_LEVEL_DEBUG:
		level_msg = "DEBUG";
		stream = stdout;
		break;
	default: 
		level_msg = "?????"; 
		stream = stderr;
		break;
	}
	
	char msg_buf[DEBUG_LOG_MSG_BUF_SIZE];

	va_list vargs;
	va_start(vargs, msg);
	vsnprintf(msg_buf, DEBUG_LOG_MSG_BUF_SIZE, msg, vargs);
	va_end(vargs);

	fprintf(stream, "VkFPS: %5s: %s\n", level_msg, msg_buf);
}

#endif /* ENABLE_DEBUG_LOGGING */

#ifdef ENABLE_DEBUG_ASSERT

void debug_assert_fail(const char *b, const char *func, const char *file, int line)
{
	LOG_FATAL("Failed debug \"%s\" assertion in %s at %s:%d", b, func, file, line);

	abort();
}

#endif /* ENABLE_DEBUG_ASSERT */
