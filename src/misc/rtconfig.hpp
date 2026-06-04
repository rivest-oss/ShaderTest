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

#pragma once
#include <string>

namespace shadertest {
	typedef struct config_t {
		std::string shader_path = "";
	} config_t;

	config_t read_config_from_args(int argc, const char **argv);
};
