#version 430 core
layout (location = 0) in vec3 position;

out vec3 v_texture_pos;


layout (std140, binding = 0) uniform Matrices
{
    mat4 u_projection;
    mat4 u_view;
};


void main()
{
  
    mat4 view = mat4(mat3(u_view));    //remove traslate
    vec4 pos = u_projection * view * vec4(position, 1.0);
    
    
    v_texture_pos = vec3(position.x, position.y, position.z);
    gl_Position = pos.xyww; //set depth to 1.0(deepest!)
    
}  