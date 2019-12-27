#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 text_pos;

out V_OUT
{
   vec3 normal;
   vec3 pos;
   vec2 text;
} v_out;


uniform mat4 u_model;

layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
    v_out.normal = mat3(transpose(inverse(u_model))) * normal; //normal matrix
    v_out.pos = vec3(u_model * vec4(position, 1.0f));
    v_out.text = vec2(text_pos.x, 1.0f - text_pos.y);
}