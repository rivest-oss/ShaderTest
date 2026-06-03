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

#include "rtconfig.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace shadertest {
	void i_print_help(void) {
		std::printf(
			""
			"Usage: shadertest [OPTIONS]...\n"
			"\n"
			"Options:\n"
			"  -h, --help       displays this screen and exits\n"
			"  -v, --version    displays the version information and exits\n"
			""
		);
	};

	void i_print_version(void) {
		std::printf(
			""
			"Rivest's Shader Tester 0.1.0a\n"
			"Copyright © 2026 Rivest Osz\n"
			"License: GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n"
			"This is free software: you are free to change and redistribute it.\n"
			"There is NO WARRANTY, to the extent permitted by law.\n"
			""
		);
	};

	const char *i_eat_next_arg(int& argc, const char **argv, int &i) {
		i++;
		if(i >= argc) {
			std::fprintf(stderr, "shadertest: option \"%s\" requires an argument.\n", argv[i - 1]);
			std::exit(1);
		}
		
		return argv[i];
	};

	config_t read_config_from_args(int argc, const char **argv) {
		config_t conf;

		for(int i = 1; i < argc; i++) {
			if(
				(std::strcmp(argv[i], "-h") == 0)
				|| (std::strcmp(argv[i], "--help") == 0)
			) {
				i_print_help();
				std::exit(0);
			}

			if(
				(std::strcmp(argv[i], "-v") == 0)
				|| (std::strcmp(argv[i], "--version") == 0)
			) {
				i_print_version();
				std::exit(0);
			}

			std::fprintf(stderr, "Bad option: %s\n", argv[i]);
			std::exit(1);
		};

		return conf;
	};
};
