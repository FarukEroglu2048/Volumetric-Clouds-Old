#version 330 core

layout(location = 0) in vec3 input_vertex;

uniform float drawing_distance;

uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;

uniform vec3 camera_position;

uniform float cloud_base;
uniform float cloud_top;

out vec3 fragment_position;

void main(void)
{
	vec3 position_multiplier = vec3(drawing_distance, cloud_top - cloud_base, drawing_distance);
	vec3 position_offset = vec3(camera_position.x, cloud_base, camera_position.z);

	vec3 vertex_position = (input_vertex * position_multiplier) + position_offset;

	fragment_position = vertex_position - camera_position;

	gl_Position = projection_matrix * modelview_matrix * vec4(vertex_position, 1.0);
}