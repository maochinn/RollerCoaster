#version 430 core
#define MAX_AMOUNT 300

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_positon;

out vec2 v_texture_pos;
out vec4 v_color;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};

uniform mat4 u_model;

uniform vec3 u_offsets[MAX_AMOUNT];
uniform vec4 u_colors[MAX_AMOUNT];


void main()
{
    float scale = 5.0f * (1.0f - u_colors[gl_InstanceID].a) + 0.5f;
    vec3 offset = u_offsets[gl_InstanceID];
    vec4 pos = u_model * vec4(position*scale, 1.0f);
    gl_Position = u_projection * u_view * (pos + vec4(offset, 0.0f));
    v_texture_pos = vec2(texture_positon.x, 1.0 - texture_positon.y);
    v_color =  u_colors[gl_InstanceID];
}