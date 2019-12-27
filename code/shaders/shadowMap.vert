#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out V_OUT
{
   vec3 normal;
   vec3 pos;
   vec4 pos_light_space;
} v_out;


layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};


uniform mat4 u_model;
uniform mat4 u_light_space_matrix;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
    v_out.normal = mat3(transpose(inverse(u_model))) * normal; //normal matrix
    v_out.pos = vec3(u_model * vec4(position, 1.0f));
    v_out.pos_light_space = u_light_space_matrix * vec4(v_out.pos, 1.0);
}