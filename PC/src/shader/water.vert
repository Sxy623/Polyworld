#version 330 core
layout (location = 0) in vec2 aVertex;

out VS_OUT {
    vec3 FragPos;
    vec3 viewPos;
    vec4 FragPosLightSpace;
} vs_out;

struct Wave{
    float A, Q, phi, w;
    vec2 D;
};
#define wave_number 4

uniform int scene_size;
uniform float scalefactor;
uniform vec3 scene_offset;

uniform float water_height;
uniform Wave wave[wave_number];
uniform float Timer;

uniform mat4 view;
uniform mat4 PVMatrix;
uniform mat4 lightSpaceMatrix;

void main() {
    //realworld coordinate calc
    int idx = gl_InstanceID / scene_size, idy = gl_InstanceID % scene_size;
    float deltax = idx - scene_size / 2.0f, deltay = idy - scene_size / 2.0f;
    vec3 pos = vec3(aVertex.x + deltax, 0.0f, aVertex.y + deltay) * scalefactor;
    pos.y = water_height;
    pos += scene_offset;

    vec3 opos = pos;
    for(int i = 0; i < wave_number; i++){
        float Qs = 1.0 / (wave_number * wave[i].w * wave[i].A);
        pos.x += Qs * wave[i].A * wave[i].D.x * cos(wave[i].w * dot(wave[i].D, opos.xz) + wave[i].phi * Timer);
        pos.z += Qs * wave[i].A * wave[i].D.y * cos(wave[i].w * dot(wave[i].D, opos.xz) + wave[i].phi * Timer);
        pos.y += wave[i].A * sin(wave[i].w * dot(wave[i].D, opos.xz) + wave[i].phi * Timer);
    }

    gl_Position = PVMatrix * vec4(pos, 1.0f);

    //world coordinate
    vs_out.FragPos = pos;
    vs_out.viewPos = vec3(view * vec4(pos, 1.0f));
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);
}