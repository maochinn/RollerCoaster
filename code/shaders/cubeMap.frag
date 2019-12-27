#version 330 core
out vec4 f_color;

in vec3 v_texture_pos;

uniform samplerCube u_cubemap;

void main()
{    
    f_color = texture(u_cubemap, v_texture_pos);
    //f_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}