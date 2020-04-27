#include "texture_helpers.h"

#include <XPLMGraphics.h>

#include <cstdio>

#include <png.h>

size_t square_root(size_t input_value)
{
	size_t output_value = 0;

	while ((output_value * output_value) < input_value) output_value++;

	return output_value;
}

GLuint create_texture(GLsizei width, GLsizei height, GLenum texture_color_mode, GLenum source_color_mode, GLubyte* texture_data)
{
	int texture_id;

	XPLMGenerateTextureNumbers(&texture_id, 1);
	XPLMBindTexture2d(texture_id, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, texture_color_mode, width, height, 0, source_color_mode, GL_UNSIGNED_BYTE, texture_data);

	XPLMBindTexture2d(0, 0);

	return texture_id;
}

GLuint create_texture(GLsizei width, GLsizei height, GLsizei depth, GLenum texture_color_mode, GLenum source_color_mode, GLubyte* texture_data)
{
	int texture_id;
	XPLMGenerateTextureNumbers(&texture_id, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, texture_id);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage3D(GL_TEXTURE_3D, 0, texture_color_mode, width, height, depth, 0, source_color_mode, GL_UNSIGNED_BYTE, texture_data);

	XPLMBindTexture2d(0, 0);

	return texture_id;
}

GLuint load_png_texture(const char* file_path, bool create_3d_texture)
{
	FILE* file;
	fopen_s(&file, file_path, "rb");

	png_structp png_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	png_infop png_info = png_create_info_struct(png_struct);

	png_init_io(png_struct, file);

	png_read_info(png_struct, png_info);

	size_t bit_depth = png_get_bit_depth(png_struct, png_info);
	size_t color_type = png_get_color_type(png_struct, png_info);

	png_set_scale_16(png_struct);
	png_set_packing(png_struct);
	png_set_expand(png_struct);
	png_set_gray_to_rgb(png_struct);
	png_set_add_alpha(png_struct, 255, PNG_FILLER_AFTER);

	size_t width = png_get_image_width(png_struct, png_info);
	size_t height = png_get_image_height(png_struct, png_info);

	GLubyte* bitmap = new GLubyte[height * width * 4];
	png_bytepp bitmap_row_pointers = new png_bytep[height];

	for (size_t y = 0; y < height; y++) bitmap_row_pointers[y] = bitmap + (y * width * 4);
	png_read_image(png_struct, bitmap_row_pointers);

	GLuint texture_id;

	if (create_3d_texture == true) texture_id = create_texture(width, square_root(height), square_root(height), GL_RGBA, GL_RGBA, bitmap);
	else texture_id = create_texture(width, height, GL_RGBA, GL_RGBA, bitmap);

	delete[] bitmap_row_pointers;
	delete[] bitmap;

	png_destroy_read_struct(&png_struct, &png_info, NULL);

	fclose(file);

	return texture_id;
}