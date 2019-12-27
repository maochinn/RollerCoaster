#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texture_positon;

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
uniform vec2 u_texture_size;
uniform mat4 u_light_space_matrix;

uniform sampler2D u_texture0;   //height map

void main()
{
    float scale = 2.0f;

    vec4 texel = texture(u_texture0, texture_positon);
    vec3 pos = position;
    pos.y += texel.r;  //height map is gray scale r = g = b
    pos.y -= 0.5f;   //normalize to [-0.5, 0.5]
    
    //calculate normal
    ivec3 off = ivec3(-1,0,1);
    vec2 size = 1.0f / u_texture_size;

    float s11 = texel.x;
    float s01 = textureOffset(u_texture0, texture_positon, off.xy).r;
    float s21 = textureOffset(u_texture0, texture_positon, off.zy).r;
    float s10 = textureOffset(u_texture0, texture_positon, off.yx).r;
    float s12 = textureOffset(u_texture0, texture_positon, off.yz).r;
    vec3 va = normalize(vec3(size.xy,s21-s01));
    vec3 vb = normalize(vec3(size.yx,s12-s10));
    //vec3 va = normalize(vec3(size.x ,s21-s01, size.y));
    //vec3 vb = normalize(vec3(size.x, s12-s10, size.y));
   
    v_out.normal = transpose(inverse(mat3(u_model))) * (cross(va,vb));
    v_out.pos = vec3(u_model * vec4(pos, 1.0f));
    v_out.pos_light_space = u_light_space_matrix * vec4(v_out.pos, 1.0);
    
    gl_Position = u_projection * u_view * u_model * vec4(pos, 1.0f);
}