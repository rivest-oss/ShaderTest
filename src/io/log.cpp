/*
** Rivest's Shader Tester: A program for visualizing and processing OpenGL shaders.
** Copyright (C) 2026 Rivest Osz
** 
** This file is part of "Rivest's Shader Tester".
** 
** "Rivest's Shader Tester" is free software: you can redistribute it and/or modify it under the terms of
** the GNU General Public License as published by the Free Software Foundation, either
** version 3 of the License, or (at your option) any later version.
** 
** "Rivest's Shader Tester" is distributed in the hope that it will be useful, but WITHOUT ANY
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
** A PARTICULAR PURPOSE. See the GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License along with
** "Rivest's Shader Tester". If not, see <https://www.gnu.org/licenses/>. 
**/

#include "log.hpp"
#include <cstdarg>
#include <cstdio>

namespace shadertest {
	int log_something(std::FILE *file, const char *pre, const char *format, std::va_list args) {
		(void)std::fprintf(file, "%s", pre);
		int rc = std::vfprintf(file, format, args);

		return rc;
	};

	int log_debug(const char *format, ...) {
		va_list args;
		va_start(args, format);

		int rc = log_something(stdout, "\x1b[96m[\x1b[95mDEBUG\x1b[96m]\x1b[0m ", format, args);

		va_end(args);

		return rc;
	};

	int log_info(const char *format, ...) {
		va_list args;
		va_start(args, format);

		int rc = log_something(stdout, "\x1b[93m[\x1b[33mINFO\x1b[93m]\x1b[0m ", format, args);

		va_end(args);

		return rc;
	};

	int log_error(const char *format, ...) {
		va_list args;

		va_start(args, format);
		int rc = log_something(stderr, "\x1b[97m[\x1b[31mERROR\x1b[97m]\x1b[0m ", format, args);
		va_end(args);

		return rc;
	};

	int log_fmt(const char *format, ...) {
		va_list args;

		va_start(args, format);
		int rc = std::vfprintf(stdout, format, args);
		va_end(args);

		return rc;
	};
};
