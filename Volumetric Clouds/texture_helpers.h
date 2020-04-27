#ifndef TEXTURE_HELPERS_H
#define TEXTURE_HELPERS_H

#include <GL/glew.h>

GLuint create_texture(GLsizei width, GLsizei height, GLenum texture_color_mode, GLenum source_color_mode, GLubyte* texture_data);
GLuint create_texture(GLsizei width, GLsizei height, GLsizei depth, GLenum texture_color_mode, GLenum source_color_mode, GLubyte* texture_data);

GLuint load_png_texture(const char* file_path, bool create_3d_texture);

#endif