#version 330 core

in vec3 fragment_position;

uniform sampler2D weather_map;

uniform sampler3D base_noise_texture;
uniform sampler3D detail_noise_texture;

uniform float drawing_distance;

uniform vec3 camera_position;

uniform float cloud_base;
uniform float cloud_top;

uniform float visibility;

uniform float cloud_coverage;

uniform vec3 light_position;
uniform vec3 light_tint;

uniform float local_time;

uniform vec3 wind_direction;

layout(location = 0) out vec4 fragment_color;

float map(float input_value, float input_minimum, float input_maximum, float output_minimum, float output_maximum)
{
	float slope = (output_maximum - output_minimum) / (input_maximum - input_minimum);

	return output_minimum + (slope * (input_value - input_minimum));
}

float clamp_ratio(float input_value)
{
	return clamp(input_value, 0.0, 1.0);
}

vec2 sample_base_density(vec3 ray_location)
{
	float height_ratio = (ray_location.y - cloud_base) / cloud_top;

	vec4 weather_map_sample = texture(weather_map, (ray_location.xz + local_time) / 100000.0)
	float weather_multiplier = max(weather_map_sample.x * weather_map_sample.x, 2.0 * clamp_ratio(cloud_coverage - 0.5) * weather_map_sample.y);

	float height_multiplier = clamp_ratio(map(height_ratio, 0.0, 0.02, 0.0, 1.0)) * clamp_ratio(map(height_ratio, 0.8, 1.0, 1.0, 0.0));
	float density_multiplier = 2.0 * clamp_ratio(map(height_ratio, 0.0, 0.15, 0.0, 1.0)) * clamp_ratio(map(height_ratio, 0.85, 1.0, 1.0, 0.0)) * cloud_coverage * weather_multiplier;

	vec4 base_noise_sample = texture(base_noise_texture, ray_location / 4000.0);
	float base_noise = map(base_noise_sample.x, (base_noise_sample.y * 0.625) + (base_noise_sample.z * 0.25) + (base_noise_sample.w * 0.125) - 1.0, 1.0, 0.0, 1.0);

	return vec2(clamp_ratio(map(base_noise * height_multiplier, 1.0 - (cloud_coverage * weather_multiplier), 1.0, 0.0, 1.0)), density_multiplier);
}

float sample_detail_density(vec3 ray_location, vec2 base_density)
{
	float height_ratio = (ray_location.y - cloud_base) / cloud_top;

	vec4 detail_noise_sample = texture(detail_noise_texture, ray_location / 1000.0);

	float detail_noise = (detail_noise_sample.x * 0.625) + (detail_noise_sample.y * 0.25) + (detail_noise_sample.z * 0.125) + (detail_noise_sample.w * 0.0625);
	detail_noise = 0.35 * exp(-0.75 * cloud_coverage) * mix(detail_noise, 1.0 - detail_noise, clamp_ratio(height_ratio * 4.0));

	return clamp_ratio(map(base_density.x, detail_noise, 1.0, 0.0, 1.0)) * base_density.y;
}

vec4 ray_march(int step_count)
{
	vec4 output_color = vec4(0.0, 0.0, 0.0, 0.0);

	vec3 ray_direction = normalize(fragment_position);

	float ray_start_distance = 0.0;
	float ray_end_distance = 0.0;

	if (camera_position.y < cloud_base)
	{
		ray_start_distance = (cloud_base - camera_position.y) / abs(ray_direction.y);
		ray_end_distance = (cloud_top - camera_position.y) / abs(ray_direction.y);
	}
	else if (camera_position.y > cloud_top)
	{
		ray_start_distance = (camera_position.y - cloud_top) / abs(ray_direction.y);
		ray_end_distance = (camera_position.y - cloud_base) / abs(ray_direction.y);
	}
	else
	{
        float x_intersect = drawing_distance / abs(ray_direction.x);

        float y_intersect_1 = -1.0 * (camera_position.y - cloud_base) / ray_direction.y;
        float y_intersect_2 = (cloud_top - camera_position.y) / ray_direction.y;

        float z_intersect = drawing_distance / abs(ray_direction.z);

        float min_1 = min(x_intersect, z_intersect);
        float max_1 = max(y_intersect_1, y_intersect_2);

        ray_end_distance = min(min_1, max_1);
	}

	float step_size = (ray_end_distance - ray_start_distance) / step_count;

	vec3 sample_ray_position = camera_position + (ray_direction * ray_start_distance);

	for (int sample_step = 0; sample_step < step_count; sample_step++)
	{
		vec2 base_density = sample_base_density(sample_ray_position);

		if (base_density.x > 0.01)
		{
			float detail_density = sample_detail_density(sample_ray_position, base_density);

			vec3 light_direction = normalize(light_position - sample_ray_position);

			vec3 light_ray_position = sample_ray_position + (light_direction * step_size);

			float obstructing_density = 0.0;

			for (int light_step = 0; light_step < 4; light_step++)
			{
				obstructing_density += sample_base_density(light_ray_position).x;

				light_ray_position += light_direction * step_size;
			}

			float alpha_multiplier = clamp_ratio(map(length(sample_ray_position.xz - camera_position.xz), visibility - 4096.0, visibility + 4096.0, 1.0, 0.0));
			if (alpha_multiplier <= 0.01) break;

			vec4 sample_color = vec4(light_tint * map(exp(-0.45 * obstructing_density), 1.0, 0.0, 1.4, 0.0), detail_density * alpha_multiplier);
			sample_color.xyz *= sample_color.w;

			output_color += sample_color * (1.0 - output_color.w);
		}

		if (output_color.w > 0.95) break;

		sample_ray_position += ray_direction * step_size;
	}

	return output_color;
}

void main(void)
{
	fragment_color = ray_march(128);
}