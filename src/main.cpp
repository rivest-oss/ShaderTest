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

#include "io/log.hpp"
#include "misc/rtconfig.hpp"
#include "Ox/include/nuclei.hpp"
#include <string>
#include <vector>

namespace Raylib {
	#include <raylib.h>
};

const char *window_title = "Rivest's Shader Tester";

const char *help_text = ""
"Usage:\n"
"\n"
"F1  Open/close help screen.\n"
"A   Go back time by 10 seconds.\n"
"D   Advance time by 10 seconds.\n"
"R   Force shader reload.\n"
"F   Display number of frames per second.\n"
"U   (Un)cap framerate.\n"
"";

const char *default_fragshader_src = ""
	"#version 330 core\n"
	"uniform uint iScreenWidth;\n"
	"uniform uint iScreenHeight;\n"
	"uniform float iTime;\n"
	"out vec4 outColor;\n"
	"void main() {\n"
	"\toutColor = vec4(\n"
	"\t\tgl_FragCoord.x / float(iScreenWidth),\n"
	"\t\tgl_FragCoord.y / float(iScreenHeight),"
	"\t\t0.0,\n"
	"\t\t1.0\n"
	"\t);\n"
	"}\n"
	"";

const char *error_fragshader_src = ""
	"#version 330 core\n"
	"uniform uint iScreenWidth;\n"
	"uniform uint iScreenHeight;\n"
	"uniform float iTime;\n"
	"out vec4 outColor;\n"
	"void main() {\n"
	"\toutColor = vec4(\n"
	"\t\t1.0,\n"
	"\t\tsin(iTime * 0.5) * 0.25,"
	"\t\t0.0,\n"
	"\t\t1.0\n"
	"\t);\n"
	"}\n"
	"";

const char *font_paths[] = {
	"Ubuntu Monospace",
	"Ubuntu Regular",
	"UbuntuMonospace",
	"UbuntuRegular",
	"Ubuntu",
	"/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf",
	"/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf",
	"/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
	"/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
	nullptr,
};

Raylib::Shader default_shader, error_shader, user_shader;
bool window_should_close = false;

Raylib::Font default_font;
int shader_mode = 0;
std::string user_shader_path;

bool user_pressed_paste = false;

bool show_help = false;
bool show_fps = false;
bool cap_fps = true;

std::vector<Raylib::Texture2D> disposable_textures;

bool user_clear_background = true;
unsigned int user_screen_width = 0;
unsigned int user_screen_height = 0;
float user_acc_time = 0.0;
float user_view_offset_x = 0.0;
float user_view_offset_y = 0.0;
float user_view_zoom = 0.0;
Raylib::Texture2D user_texture;

int init(int argc, const char **argv) {
	Ox::Error err;
	
	(void)shadertest::read_config_from_args(argc, argv);
	
	Raylib::SetTargetFPS(60);
	Raylib::SetExitKey(Raylib::KEY_NULL);
	Raylib::SetConfigFlags(Raylib::FLAG_WINDOW_RESIZABLE);

	Raylib::InitWindow(640, 360, window_title);
	while(Raylib::IsWindowReady() == false) {};
	
	Raylib::InitAudioDevice();
	while(Raylib::IsAudioDeviceReady() == false) {};

	window_should_close = false;

	default_shader = Raylib::LoadShaderFromMemory(nullptr, default_fragshader_src);
	error_shader = Raylib::LoadShaderFromMemory(nullptr, error_fragshader_src);
	
	user_shader.id = 0;
	user_shader.locs = nullptr;
	user_shader_path = "";
	show_help = show_fps = false;
	cap_fps = true;
	shader_mode = 0;

	user_pressed_paste = false;

	user_clear_background = true;
	user_acc_time = 0.0;
	user_view_offset_x = 0.0;
	user_view_offset_y = 0.0;
	user_view_zoom = 0.0;

	disposable_textures.clear();

	{
		Raylib::Image img = Raylib::GenImageText(256, 256, "Lorem ipsum");
		user_texture = Raylib::LoadTextureFromImage(img);
		Raylib::UnloadImage(img);
	}

	if(Raylib::IsShaderValid(default_shader) == false) {
		shadertest::log_error("Default shader is not available. Exiting.");
		return -1;
	} else if(Raylib::IsShaderValid(error_shader) == false) {
		shadertest::log_error("'Error screen' shader is not available. Exiting.");
		return -1;
	} else if(Raylib::IsTextureValid(user_texture) == false) {
		shadertest::log_error("Default user texture is not available. Exiting.");
		return -1;
	}

	{
		for(int i = 0; font_paths[i] != nullptr; i++) {
			default_font = Raylib::LoadFontEx(font_paths[i], 128, nullptr, 0);
			if(Raylib::IsFontValid(default_font))
				break;
		};

		if(Raylib::IsFontValid(default_font) == false)
			default_font = Raylib::GetFontDefault();
	}

	return 0;
};

void deinit(void) {
	if(Raylib::IsShaderValid(default_shader))
		Raylib::UnloadShader(default_shader);
	if(Raylib::IsShaderValid(error_shader))
		Raylib::UnloadShader(error_shader);
	if(Raylib::IsShaderValid(user_shader))
		Raylib::UnloadShader(user_shader);
	if(Raylib::IsTextureValid(user_texture))
		Raylib::UnloadTexture(user_texture);

	Raylib::CloseAudioDevice();
	Raylib::CloseWindow();
};

void update_target_fps(void) {
	int fps;
	if(cap_fps)
		fps = Raylib::GetMonitorRefreshRate(Raylib::GetCurrentMonitor());
	else
		fps = 999;

	if(fps < 1) fps = 1;
	Raylib::SetTargetFPS(fps);
};

bool reload_shader = false;

void handle_clipboard(void) {
	if(user_pressed_paste == false)
		return;

	user_pressed_paste = false;

	Raylib::Image img = Raylib::GetClipboardImage();

	if(Raylib::IsImageValid(img)) {
		Raylib::Texture tex = Raylib::LoadTextureFromImage(img);
		if(Raylib::IsTextureValid(tex)) {
			Raylib::UnloadTexture(user_texture);
			user_texture = tex;
		}

		Raylib::UnloadImage(img);

		return;
	}

	const char *text = Raylib::GetClipboardText();

	if(text == nullptr) text = "";

	Raylib::Shader shader = Raylib::LoadShaderFromMemory(nullptr, text);

	if(Raylib::IsShaderValid(shader)) {
		Raylib::UnloadShader(shader);

		user_shader_path = nullptr;
		reload_shader = true;

		return;
	}
};

void handle_dropped_files(void) {
	if(Raylib::IsFileDropped() == false)
		return;

	Raylib::FilePathList files = Raylib::LoadDroppedFiles();

	for(unsigned int i = 0; i < files.count; i++) {
		ox_assert(files.paths[i] != nullptr, "dropped file MUST not be null");

		Raylib::Image img = Raylib::LoadImage(files.paths[i]);
		
		if(Raylib::IsImageValid(img)) {
			Raylib::Texture tex = Raylib::LoadTextureFromImage(img);
			if(Raylib::IsTextureValid(tex)) {
				Raylib::UnloadTexture(user_texture);
				user_texture = tex;
			}

			Raylib::UnloadImage(img);

			continue;
		}

		user_shader_path = files.paths[i];
		reload_shader = true;
	};

	Raylib::UnloadDroppedFiles(files);
};

long last_mod_time_shader_file = -1;

void update_shader_reload(void) {
	if(user_shader_path.size() < 1)
		return;

	long t = Raylib::GetFileModTime(user_shader_path.c_str());

	if(last_mod_time_shader_file != t) {
		reload_shader = true;
		last_mod_time_shader_file = t;
	}

	if(reload_shader) {
		if(Raylib::IsShaderValid(user_shader))
			Raylib::UnloadShader(user_shader);

		user_shader = Raylib::LoadShader(nullptr, user_shader_path.c_str());

		if(Raylib::IsShaderValid(user_shader))
			shader_mode = 1;
		else
			shader_mode = -1;
	}

	reload_shader = false;
};

void update(double dt) {
	window_should_close = Raylib::WindowShouldClose();

	if(window_should_close)
		return;

	user_pressed_paste = (
		Raylib::IsKeyDown(Raylib::KEY_LEFT_CONTROL)
		&& Raylib::IsKeyPressed(Raylib::KEY_V)
	);
	
	handle_dropped_files();
	handle_clipboard();
	update_shader_reload();

	if(Raylib::IsShaderValid(user_shader) == false && shader_mode >= 1)
		shader_mode = -1;

	if(Raylib::IsKeyPressed(Raylib::KEY_F1))
		show_help = show_help == false;
	if(Raylib::IsKeyPressed(Raylib::KEY_A))
		user_acc_time -= 10.;
	if(Raylib::IsKeyPressed(Raylib::KEY_D))
		user_acc_time += 10.;
	if(Raylib::IsKeyPressed(Raylib::KEY_F))
		show_fps = show_fps == false;
	if(Raylib::IsKeyPressed(Raylib::KEY_R))
		reload_shader = true;

	if(Raylib::IsKeyPressed(Raylib::KEY_U)) {
		cap_fps = cap_fps == false;
		update_target_fps();
	}

	user_screen_width = Raylib::GetScreenWidth();
	user_screen_height = Raylib::GetScreenHeight();
	user_acc_time += dt;

	std::string win_title = window_title;

	if(shader_mode < 0)
		win_title += " | ERROR";
	else if(shader_mode < 1)
		win_title += " | Drop a shader!";

	Raylib::SetWindowTitle(win_title.c_str());
};

void update_shader_values(Raylib::Shader shader) {
	ox_assert(Raylib::IsShaderValid(shader), "shader MUST be valid");;

	int loc;

	loc = Raylib::GetShaderLocation(shader, "iScreenWidth");
	Raylib::SetShaderValue(shader, loc, &user_screen_width, Raylib::SHADER_UNIFORM_UINT);

	loc = Raylib::GetShaderLocation(shader, "iScreenHeight");
	Raylib::SetShaderValue(shader, loc, &user_screen_height, Raylib::SHADER_UNIFORM_UINT);

	loc = Raylib::GetShaderLocation(shader, "iTime");
	Raylib::SetShaderValue(shader, loc, &user_acc_time, Raylib::SHADER_UNIFORM_FLOAT);

	loc = Raylib::GetShaderLocation(shader, "iViewportX");
	Raylib::SetShaderValue(shader, loc, &user_view_offset_x, Raylib::SHADER_UNIFORM_FLOAT);

	loc = Raylib::GetShaderLocation(shader, "iViewportY");
	Raylib::SetShaderValue(shader, loc, &user_view_offset_y, Raylib::SHADER_UNIFORM_FLOAT);

	loc = Raylib::GetShaderLocation(shader, "iViewportZoom");
	Raylib::SetShaderValue(shader, loc, &user_view_zoom, Raylib::SHADER_UNIFORM_FLOAT);

	loc = Raylib::GetShaderLocation(shader, "iImage");
	Raylib::SetShaderValueTexture(shader, loc, user_texture);
};

std::vector<std::string> split_text(const char *src, char delimiter) {
	std::vector<std::string> lines_v;
	std::string curr_line;
	
	while(true) {
		char c = *src++;
		if(c == '\0') {
			if(curr_line.size() >= 1) {
				curr_line.push_back('\0');
				lines_v.push_back(curr_line);
			}

			break;
		}

		if(c == delimiter) {
			curr_line.push_back('\0');
			lines_v.push_back(curr_line);
			curr_line.clear();
			continue;
		}
		
		curr_line.push_back(c);
	};
	
	return lines_v;
};

void draw_text_fit(
	float off_x,
	float off_y,
	float width,
	float height,
	Raylib::Color foreground_color,
	const char *text,
	Raylib::Font font,
	float font_height
) {
	ox_assert(Raylib::IsFontValid(font), "font MUST be valid");
	ox_assert(font_height >= 1.f, "font_height must be at least 1");

	if(text == nullptr) text = "(null)";

	float x = 0.f, y = 0.f;
	std::vector<std::string> lines = split_text(text, '\n');

	for(std::string line : lines) {
		std::vector<std::string> words = split_text(line.c_str(), ' ');

		for(std::string word : words) {
			Raylib::Vector2 word_msr = Raylib::MeasureTextEx(
				font,
				word.c_str(),
				font_height,
				1.f
			);

			if((x + word_msr.x) >= width) {
				x = 0.f;
				y += word_msr.y;
			}

			if((y + word_msr.y) >= height)
				return;

			Raylib::DrawTextEx(
				font,
				word.c_str(),
				Raylib::Vector2 {
					off_x + x,
					off_y + y,
				},
				font_height,
				1.f,
				foreground_color
			);

			x += word_msr.x;
			x += Raylib::MeasureTextEx(font, " ", font_height, 1.f).x;
		};

		x = 0.f;
		y += Raylib::MeasureTextEx(font, line.c_str(), font_height, 1.f).y;
	};
};

void draw_help_screen(void) {
	Raylib::DrawRectangle(
		user_screen_width * .1,
		user_screen_height * .1,
		user_screen_width * 0.8,
		user_screen_height * 0.8,
		Raylib::Color {
			0x10,
			0x10,
			0x10,
			0xc0,
		}
	);

	draw_text_fit(
		user_screen_width * .12,
		user_screen_height * .12,
		user_screen_width * 0.76,
		user_screen_height * 0.76,
		Raylib::WHITE,
		help_text,
		default_font,
		24.f
	);
};

void draw_fps(void) {
	int fps = Raylib::GetFPS();
	const char *text = Raylib::TextFormat("%i FPS", fps);

	Raylib::Vector2 msr = Raylib::MeasureTextEx(
		default_font,
		text,
		32.f,
		1.f
	);

	float margin_right = 16.f;
	float margin_top = 16.f;
	int box_size = 16;

	Raylib::Image img = Raylib::GenImageColor(
		msr.x + box_size * 2.f,
		msr.y + box_size + 2.f,
		Raylib::BLANK
	);

	Raylib::ImageDrawTextEx(
		&img,
		default_font,
		text,
		Raylib::Vector2 {
			(float)box_size,
			(float)box_size,
		},
		32.f,
		1.f,
		Raylib::WHITE
	);

	Raylib::Image img_blurred = Raylib::ImageCopy(img);
	Raylib::ImageBlurGaussian(&img_blurred, 1);

	for(int y = 0; y < img.height; y++) {
		for(int x = 0; x < img.width; x++) {
			Raylib::Color color_src = Raylib::GetImageColor(img, x, y);

			if(color_src.a > 0x80)
				continue;

			Raylib::Color color_blur = Raylib::GetImageColor(img_blurred, x, y);
			color_blur.r = color_blur.g = color_blur.b = 0x00;

			int alpha = color_blur.a;
			alpha *= 8;
			if(alpha > 255) alpha = 255;

			color_blur.a = alpha;

			Raylib::ImageDrawPixel(
				&img,
				x,
				y,
				color_blur
			);
		};
	};

	Raylib::Texture2D tex = Raylib::LoadTextureFromImage(img);
	Raylib::UnloadImage(img);

	if(Raylib::IsTextureValid(tex)) {
		disposable_textures.push_back(tex);

		Raylib::DrawTextureEx(
			tex,
			Raylib::Vector2 {
				Raylib::GetScreenWidth() - tex.width - margin_right,
				margin_top,
			},
			0.f,
			1.f,
			Raylib::WHITE
		);
	}
};

void draw(double dt) {
	(void)dt;

	Raylib::BeginDrawing();

	if(user_clear_background)
		Raylib::ClearBackground(Raylib::BLACK);

	if(window_should_close) {
		Raylib::EndDrawing();
		Raylib::SwapScreenBuffer();
		return;
	}

	Raylib::Shader shader;
	
	if(shader_mode >= 1)
		shader = user_shader;
	else if(shader_mode >= 0)
		shader = default_shader;
	else
		shader = error_shader;

	Raylib::BeginShaderMode(shader);

	update_shader_values(shader);

	Raylib::DrawRectangle(
		0,
		0,
		user_screen_width,
		user_screen_height,
		Raylib::WHITE
	);

	Raylib::EndShaderMode();

	if(show_help)
		draw_help_screen();
	if(show_fps)
		draw_fps();
	
	Raylib::EndDrawing();
	Raylib::SwapScreenBuffer();

	for(Raylib::Texture2D tex : disposable_textures)
		Raylib::UnloadTexture(tex);

	disposable_textures.clear();
};

void update_and_draw(void) {
	double dt = Raylib::GetFrameTime();
	update(dt);
	draw(dt);
};

int main(int argc, const char **argv) {
	int rc = init(argc, argv);
	
	if(rc < 0) {
		deinit();
		return 1;
	}
	
	update_target_fps();

	while(window_should_close == false)
		update_and_draw();

	deinit();
	return 0;
};

