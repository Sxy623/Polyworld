#version 330 core
layout (location = 0) in vec2 aVertex;

uniform mat4 PVMatrix;
uniform float scalefactor;
uniform float water_height;
uniform vec3 scene_offset;
uniform int scene_size;
void main() {
    int idx = gl_InstanceID / scene_size, idy = gl_InstanceID % scene_size;
    float deltax = idx - scene_size / 2.0f, deltay = idy - scene_size / 2.0f;
    vec3 pos = vec3(aVertex.x + deltax, 0.0f, aVertex.y + deltay) * scalefactor;
    pos.y = water_height;
    gl_Position = PVMatrix * vec4(pos + scene_offset, 1.0f);
}