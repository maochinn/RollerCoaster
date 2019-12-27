#version 430 core
layout (location = 0) in vec3 position;

//uniform mat4 lightSpaceMatrix;
layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};

uniform mat4 u_model;

void main()
{
    //gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
    gl_Position = u_projection * u_view * u_model * vec4(position, 1.0f);
}