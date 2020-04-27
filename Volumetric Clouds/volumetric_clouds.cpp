#include <Windows.h>

#include <GL/glew.h>

#include <XPLMDefs.h>

#include <XPLMDataAccess.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMCamera.h>

#include <XPLMUtilities.h>

#include <cmath>

#include <cstring>

#include "shader_helpers.h"
#include "texture_helpers.h"

#define XPLANE_BUFFER_SIZE 256

#define MATRIX_SIZE 16

#define DRAWING_DISTANCE_CONSTANT 32768.0
#define DRAWING_DISTANCE_MULTIPLIER 8.0

#define SUN_HEIGHT 15240.0

#define RADIANS_PER_DEGREES 0.01745329251994329576

GLfloat bounding_box_vertices[] =
{
	-1.0, 0.0, 1.0,
	1.0, 0.0, 1.0,
	-1.0, 0.0, -1.0,	

	-1.0, 0.0, -1.0,
	1.0, 0.0, 1.0,
	1.0, 0.0, -1.0,
	
	

	-1.0, 0.0, 1.0,
	-1.0, 1.0, 1.0,
	1.0, 0.0, 1.0,

	1.0, 0.0, 1.0,
	-1.0, 1.0, 1.0,
	1.0, 1.0, 1.0,


	
	1.0, 0.0, -1.0,
	1.0, 0.0, 1.0,
	1.0, 1.0, -1.0,

	1.0, 1.0, -1.0,
	1.0, 0.0, 1.0,
	1.0, 1.0, 1.0,
	
	

	1.0, 0.0, -1.0,
	1.0, 1.0, -1.0,
	-1.0, 0.0, -1.0,

	-1.0, 0.0, -1.0,
	1.0, 1.0, -1.0,
	-1.0, 1.0, -1.0,
	
	

	-1.0, 0.0, 1.0,
	-1.0, 0.0, -1.0,
	-1.0, 1.0, 1.0,

	-1.0, 1.0, 1.0,
	-1.0, 0.0, -1.0,
	-1.0, 1.0, -1.0,



	-1.0, 1.0, 1.0,
	-1.0, 1.0, -1.0,
	1.0, 1.0, 1.0,

	1.0, 1.0, 1.0,
	-1.0, 1.0, -1.0,
	1.0, 1.0, -1.0
};

GLuint shader_program;

GLuint vertex_array;

GLuint weather_map;

GLuint base_noise_texture;
GLuint detail_noise_texture;

XPLMDataRef cloud_type_datarefs[3];

XPLMDataRef modelview_matrix_dataref;
XPLMDataRef projection_matrix_dataref;

XPLMDataRef cloud_base_datarefs[3];
XPLMDataRef cloud_top_datarefs[3];

XPLMDataRef cloud_coverage_datarefs[3];

XPLMDataRef visibility_dataref;

XPLMDataRef sun_pitch_dataref;
XPLMDataRef sun_heading_dataref;

XPLMDataRef light_tint_red_dataref;
XPLMDataRef light_tint_green_dataref;
XPLMDataRef light_tint_blue_dataref;

XPLMDataRef local_time_dataref;

XPLMDataRef wind_direction_x_dataref;
XPLMDataRef wind_direction_y_dataref;
XPLMDataRef wind_direction_z_dataref;

GLint shader_drawing_distance;

GLint shader_modelview_matrix;
GLint shader_projection_matrix;

GLint shader_camera_position;

GLint shader_cloud_base;
GLint shader_cloud_top;

GLint shader_visibility;

GLint shader_cloud_coverage;

GLint shader_light_position;
GLint shader_light_tint;

GLint shader_local_time;

GLint shader_wind_direction;

BOOL APIENTRY DllMain(IN HINSTANCE dll_handle, IN DWORD call_reason, IN LPVOID reserved)
{
	return TRUE;
}

int draw_callback(XPLMDrawingPhase drawing_phase, int is_before, void* references)
{
	int cloud_type = XPLMGetDatai(cloud_type_datarefs[0]);

	if (cloud_type != 0)
	{
		XPLMSetGraphicsState(0, 3, 0, 1, 1, 1, 1);

		glUseProgram(shader_program);

		XPLMBindTexture2d(weather_map, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_3D, base_noise_texture);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_3D, detail_noise_texture);

		XPLMCameraPosition_t camera_position;
		XPLMReadCameraPosition(&camera_position);

		glUniform1f(shader_drawing_distance, DRAWING_DISTANCE_CONSTANT + (camera_position.y * DRAWING_DISTANCE_MULTIPLIER));

		float modelview_matrix[MATRIX_SIZE];
		float projection_matrix[MATRIX_SIZE];

		XPLMGetDatavf(modelview_matrix_dataref, modelview_matrix, 0, MATRIX_SIZE);
		XPLMGetDatavf(projection_matrix_dataref, projection_matrix, 0, MATRIX_SIZE);

		glUniformMatrix4fv(shader_modelview_matrix, 1, GL_FALSE, modelview_matrix);
		glUniformMatrix4fv(shader_projection_matrix, 1, GL_FALSE, projection_matrix);

		glUniform3f(shader_camera_position, camera_position.x, camera_position.y, camera_position.z);

		glUniform1f(shader_cloud_base, XPLMGetDataf(cloud_base_datarefs[0]));
		glUniform1f(shader_cloud_top, XPLMGetDataf(cloud_top_datarefs[0]));

		glUniform1f(shader_visibility, XPLMGetDataf(visibility_dataref));

		glUniform1f(shader_cloud_coverage, XPLMGetDataf(cloud_coverage_datarefs[0]) * 4.0);

		float sun_pitch = XPLMGetDataf(sun_pitch_dataref) * RADIANS_PER_DEGREES;
		float sun_heading = XPLMGetDataf(sun_heading_dataref) * RADIANS_PER_DEGREES;

		glUniform3f(shader_light_position, SUN_HEIGHT * cos(sun_pitch) * cos(sun_heading), SUN_HEIGHT * sin(sun_pitch), SUN_HEIGHT * cos(sun_pitch) * sin(sun_heading));
		glUniform3f(shader_light_tint, XPLMGetDataf(light_tint_red_dataref), XPLMGetDataf(light_tint_green_dataref), XPLMGetDataf(light_tint_blue_dataref));

		glUniform1f(shader_local_time, XPLMGetDataf(local_time_dataref));

		glUniform3f(shader_wind_direction, XPLMGetDataf(wind_direction_x_dataref), XPLMGetDataf(wind_direction_y_dataref), XPLMGetDataf(wind_direction_z_dataref));

		glBindVertexArray(vertex_array);

		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);

		XPLMBindTexture2d(0, 0);

		XPLMBindTexture2d(0, 1);
		XPLMBindTexture2d(0, 2);

		glUseProgram(0);
	}

	return 1;
}

PLUGIN_API int XPluginStart(char* plugin_name, char* plugin_signature, char* plugin_description)
{
	strcpy_s(plugin_name, XPLANE_BUFFER_SIZE, "Volumetric Clouds");
	strcpy_s(plugin_signature, XPLANE_BUFFER_SIZE, "FarukEroglu2048.volumetric_clouds");
	strcpy_s(plugin_description, XPLANE_BUFFER_SIZE, "3D / Volumetric Clouds for X-Plane 11");

	cloud_type_datarefs[0] = XPLMFindDataRef("sim/weather/cloud_type[0]");
	cloud_type_datarefs[1] = XPLMFindDataRef("sim/weather/cloud_type[1]");
	cloud_type_datarefs[2] = XPLMFindDataRef("sim/weather/cloud_type[2]");

	modelview_matrix_dataref = XPLMFindDataRef("sim/graphics/view/world_matrix");
	projection_matrix_dataref = XPLMFindDataRef("sim/graphics/view/projection_matrix");

	cloud_base_datarefs[0] = XPLMFindDataRef("sim/weather/cloud_base_msl_m[0]");
	cloud_base_datarefs[1] = XPLMFindDataRef("sim/weather/cloud_base_msl_m[1]");
	cloud_base_datarefs[2] = XPLMFindDataRef("sim/weather/cloud_base_msl_m[2]");

	cloud_top_datarefs[0] = XPLMFindDataRef("sim/weather/cloud_tops_msl_m[0]");
	cloud_top_datarefs[1] = XPLMFindDataRef("sim/weather/cloud_tops_msl_m[1]");
	cloud_top_datarefs[2] = XPLMFindDataRef("sim/weather/cloud_tops_msl_m[2]");

	cloud_coverage_datarefs[0] = XPLMFindDataRef("sim/weather/thermal_percent");
	cloud_coverage_datarefs[1] = XPLMFindDataRef("sim/weather/thermal_percent");
	cloud_coverage_datarefs[2] = XPLMFindDataRef("sim/weather/thermal_percent");

	visibility_dataref = XPLMFindDataRef("sim/graphics/view/visibility_effective_m");

	sun_pitch_dataref = XPLMFindDataRef("sim/graphics/scenery/sun_pitch_degrees");
	sun_heading_dataref = XPLMFindDataRef("sim/graphics/scenery/sun_heading_degrees");

	light_tint_red_dataref = XPLMFindDataRef("sim/graphics/misc/outside_light_level_r");
	light_tint_green_dataref = XPLMFindDataRef("sim/graphics/misc/outside_light_level_g");
	light_tint_blue_dataref = XPLMFindDataRef("sim/graphics/misc/outside_light_level_b");

	local_time_dataref = XPLMFindDataRef("sim/time/local_time_sec");

	wind_direction_x_dataref = XPLMFindDataRef("sim/weather/wind_now_x_msc");
	wind_direction_y_dataref = XPLMFindDataRef("sim/weather/wind_now_y_msc");
	wind_direction_z_dataref = XPLMFindDataRef("sim/weather/wind_now_z_msc");

	glewInit();

	XPLMSetGraphicsState(0, 1, 0, 0, 0, 0, 0);

	GLuint vertex_shader = load_shader("Resources/plugins/Volumetric Clouds/shaders/vertex_shader.glsl", GL_VERTEX_SHADER);
	GLuint fragment_shader = load_shader("Resources/plugins/Volumetric Clouds/shaders/fragment_shader.glsl", GL_FRAGMENT_SHADER);

	shader_program = glCreateProgram();

	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);

	glLinkProgram(shader_program);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glUseProgram(shader_program);

	weather_map = load_png_texture("Resources/plugins/Volumetric Clouds/textures/weather_map.png", false);

	base_noise_texture = load_png_texture("Resources/plugins/Volumetric Clouds/textures/base_noise_texture.png", true);
	detail_noise_texture = load_png_texture("Resources/plugins/Volumetric Clouds/textures/detail_noise_texture.png", true);

	GLint shader_weather_map = glGetUniformLocation(shader_program, "weather_map");

	GLint shader_base_noise_texture = glGetUniformLocation(shader_program, "base_noise_texture");
	GLint shader_detail_noise_texture = glGetUniformLocation(shader_program, "detail_noise_texture");
	
	shader_drawing_distance = glGetUniformLocation(shader_program, "drawing_distance");

	shader_modelview_matrix = glGetUniformLocation(shader_program, "modelview_matrix");
	shader_projection_matrix = glGetUniformLocation(shader_program, "projection_matrix");

	shader_camera_position = glGetUniformLocation(shader_program, "camera_position");

	shader_visibility = glGetUniformLocation(shader_program, "visibility");

	shader_cloud_base = glGetUniformLocation(shader_program, "cloud_base");
	shader_cloud_top = glGetUniformLocation(shader_program, "cloud_top");

	shader_cloud_coverage = glGetUniformLocation(shader_program, "cloud_coverage");

	shader_light_position = glGetUniformLocation(shader_program, "light_position");
	shader_light_tint = glGetUniformLocation(shader_program, "light_tint");

	shader_local_time = glGetUniformLocation(shader_program, "local_time");

	shader_wind_direction = glGetUniformLocation(shader_program, "wind_direction");

	glUniform1i(shader_weather_map, 0);

	glUniform1i(shader_base_noise_texture, 1);
	glUniform1i(shader_detail_noise_texture, 2);

	glUseProgram(0);

	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	GLuint vertex_buffer;

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(bounding_box_vertices), bounding_box_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	XPLMRegisterDrawCallback(draw_callback, xplm_Phase_Modern3D, 0, NULL);

	return 1;
}

PLUGIN_API void XPluginStop(void)
{

}

PLUGIN_API int XPluginEnable(void)
{
	return 1;
}

PLUGIN_API void XPluginDisable(void)
{

}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID sender_plugin, int message_type, void* parameters)
{

}