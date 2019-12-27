#version 430 core


out vec4 f_color;

uniform vec3 u_color;

void main()
{ 
    f_color = vec4(u_color, 1.0f);
    //f_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}