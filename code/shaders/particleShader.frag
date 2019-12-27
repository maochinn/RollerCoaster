#version 430 core
#define MAX_AMOUNT 1000

in vec2 v_texture_pos;
in vec4 v_color;

out vec4 f_color;

uniform sampler2D u_texture0;


void main()
{
    //if(texture(u_texture0, v_texture_pos).r == 0.0f)discard;
    f_color = (texture(u_texture0, v_texture_pos) * v_color);
    //f_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}