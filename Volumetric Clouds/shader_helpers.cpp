#include "shader_helpers.h"

#include <fstream>

GLuint load_shader(const char* shader_path, GLenum shader_type)
{
	std::ifstream shader_file(shader_path, std::ifstream::binary | std::ifstream::ate);
	GLint shader_file_size = GLint(shader_file.tellg());

	shader_file.seekg(0);

	GLchar* shader_string = new GLchar[shader_file_size];
	shader_file.read(shader_string, shader_file_size);

	GLuint shader = glCreateShader(shader_type);

	glShaderSource(shader, 1, &shader_string, &shader_file_size);
	glCompileShader(shader);

	delete[] shader_string;

	shader_file.close();

	return shader;
}