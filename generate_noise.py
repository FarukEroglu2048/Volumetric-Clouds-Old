import numpy
import scipy

import itertools

import matplotlib
import matplotlib.pyplot

import imageio

noise_size = 32
grid_size = 8

octave_count = 4

grid_count = (noise_size // grid_size) * pow(2, octave_count - 1)

perlin_gradients = [[1.0, 1.0, 0.0], [-1.0, 1.0, 0.0], [1.0, -1.0, 0.0], [-1.0, -1.0, 0.0], [0.0, 1.0, 1.0], [0.0, -1.0, 1.0], [0.0, 1.0, -1.0], [0.0, -1.0, -1.0], [1.0, 0.0, 1.0], [-1.0, 0.0, 1.0], [1.0, 0.0, -1.0], [-1.0, 0.0, -1.0]]
perlin_gradients = numpy.asarray(perlin_gradients)

random_number_generator = numpy.random.default_rng()

perlin_gradients = numpy.take(perlin_gradients, random_number_generator.integers(0, high=numpy.size(perlin_gradients, axis=0), size=(grid_count, grid_count, grid_count)), axis=0)
worley_points = random_number_generator.uniform(low=0.0, high=1.0, size=(grid_count, grid_count, grid_count, 3))

def mix(input_1, input_2, ratio):
    return input_1 + (ratio * (input_2 - input_1))

def perlin_gradient_vector(vertex_coordinates, octave_index):
    octave_grid_count = (noise_size // grid_size) * pow(2, octave_index)

    vertex_indices = vertex_coordinates % octave_grid_count

    return perlin_gradients[vertex_indices[0], vertex_indices[1], vertex_indices[2]]

def worley_point(grid_coordinates, octave_index):
    octave_grid_count = (noise_size // grid_size) * pow(2, octave_index)

    grid_indices = grid_coordinates % octave_grid_count

    return grid_coordinates + worley_points[grid_indices[0], grid_indices[1], grid_indices[2]]

def perlin_noise(x, y, z, octave_index):
    coordinates = [(x / grid_size) * pow(2, octave_index), (y / grid_size) * pow(2, octave_index), (z / grid_size) * pow(2, octave_index)]
    coordinates = numpy.asarray(coordinates)

    fractional, integer = numpy.modf(coordinates)
    integer = numpy.asarray(integer, dtype=int)

    vertex_1 = numpy.asarray([0, 0, 0], dtype=int)
    vertex_2 = numpy.asarray([1, 0, 0], dtype=int)
    vertex_3 = numpy.asarray([1, 0, 1], dtype=int)
    vertex_4 = numpy.asarray([0, 0, 1], dtype=int)
    vertex_5 = numpy.asarray([0, 1, 0], dtype=int)
    vertex_6 = numpy.asarray([1, 1, 0], dtype=int)
    vertex_7 = numpy.asarray([1, 1, 1], dtype=int)
    vertex_8 = numpy.asarray([0, 1, 1], dtype=int)

    dot_1 = numpy.dot(perlin_gradient_vector(integer + vertex_1, octave_index), fractional - vertex_1)
    dot_2 = numpy.dot(perlin_gradient_vector(integer + vertex_2, octave_index), fractional - vertex_2)
    dot_3 = numpy.dot(perlin_gradient_vector(integer + vertex_3, octave_index), fractional - vertex_3)
    dot_4 = numpy.dot(perlin_gradient_vector(integer + vertex_4, octave_index), fractional - vertex_4)
    dot_5 = numpy.dot(perlin_gradient_vector(integer + vertex_5, octave_index), fractional - vertex_5)
    dot_6 = numpy.dot(perlin_gradient_vector(integer + vertex_6, octave_index), fractional - vertex_6)
    dot_7 = numpy.dot(perlin_gradient_vector(integer + vertex_7, octave_index), fractional - vertex_7)
    dot_8 = numpy.dot(perlin_gradient_vector(integer + vertex_8, octave_index), fractional - vertex_8)

    fractional_smooth = (6.0 * numpy.power(fractional, 5.0)) - (15.0 * numpy.power(fractional, 4.0)) + (10.0 * numpy.power(fractional, 3.0))

    mix_1 = mix(dot_1, dot_2, fractional_smooth[0])
    mix_2 = mix(dot_5, dot_6, fractional_smooth[0])
    mix_3 = mix(dot_4, dot_3, fractional_smooth[0])
    mix_4 = mix(dot_8, dot_7, fractional_smooth[0])

    mix_5 = mix(mix_1, mix_2, fractional_smooth[1])
    mix_6 = mix(mix_3, mix_4, fractional_smooth[1])

    return mix(mix_5, mix_6, fractional_smooth[2])

def worley_noise(x, y, z, octave_index):
    coordinates = [(x / grid_size) * pow(2, octave_index), (y / grid_size) * pow(2, octave_index), (z / grid_size) * pow(2, octave_index)]
    coordinates = numpy.asarray(coordinates)

    integer = numpy.trunc(coordinates)
    integer = numpy.asarray(integer, dtype=int)

    distance_list = list()

    for coordinate_offset in itertools.product([-1, 0, 1], repeat=3):
        distance = numpy.linalg.norm(coordinates - worley_point(integer + numpy.asarray(coordinate_offset, dtype=int), octave_index))

        distance_list.append(distance)

    return min(distance_list)

def fractal_noise(noise_function, x, y, z, start_octave):
    output_value = 0.0

    for octave_index in range(start_octave, octave_count):
        output_value += noise_function(x, y, z, octave_index) * pow(2, (-1 * octave_index) - 1)

    return output_value

noise_array = numpy.zeros((noise_size * noise_size, noise_size, 4))

for z in range(noise_size):
    for y in range(noise_size):
        for x in range(noise_size):
            noise_array[(z * noise_size) + y, x, 0] = -1.0 * fractal_noise(worley_noise, x, y, z, 0)
            noise_array[(z * noise_size) + y, x, 1] = -1.0 * fractal_noise(worley_noise, x, y, z, 1)
            noise_array[(z * noise_size) + y, x, 2] = -1.0 * fractal_noise(worley_noise, x, y, z, 2)
            noise_array[(z * noise_size) + y, x, 3] = -1.0 * fractal_noise(worley_noise, x, y, z, 3)

minimum_values = numpy.expand_dims(numpy.min(noise_array, axis=(0, 1)), (0, 1))
maximum_values = numpy.expand_dims(numpy.max(noise_array, axis=(0, 1)), (0, 1))

noise_array = (noise_array - minimum_values) / (maximum_values - minimum_values)

output_array = numpy.asarray(noise_array * 255.0, dtype=numpy.uint8)

imageio.imsave("noise_texture_detail.png", output_array)

imageio.imsave("debug_channel_red.png", output_array[:, :, 0])
imageio.imsave("debug_channel_green.png", output_array[:, :, 1])
imageio.imsave("debug_channel_blue.png", output_array[:, :, 2])
imageio.imsave("debug_channel_alpha.png", output_array[:, :, 3])

"""
noise_array = numpy.zeros((noise_size, noise_size))

for y in range(noise_size):
    for x in range(noise_size):
        noise_array[y, x] = fractal_noise(perlin_noise, x, y, 0.0, 0) - fractal_noise(worley_noise, x, y, 0.0, 0)

minimum_value = numpy.min(noise_array)
maximum_value = numpy.max(noise_array)

noise_array = (noise_array - minimum_value) / (maximum_value - minimum_value)

output_array = numpy.asarray(noise_array * 255.0, dtype=numpy.uint8)

imageio.imsave("weather_map.png", output_array)
"""

print("Done!")