#version 430 core

in V_OUT
{
   vec3 normal;
   vec3 pos;
   vec2 text;
} f_in;

out vec4 f_color;


struct Material
{
    sampler2D t_diffuse;
    sampler2D t_specular;
	//sampler2D t_emission;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};
struct DirectLight
{                   //offset
	vec3 direction; //0
    vec3 ambient;   //16
    vec3 diffuse;   //32
    vec3 specular;  //48
};

layout (std140, binding = 1) uniform Light
{
    DirectLight u_directLight;
};
layout (std140, binding = 2) uniform View
{
    vec3 u_view_dir;    //0
    vec3 u_view_pos;    //16
};


uniform Material u_material;

vec3 computeDirectLight(DirectLight light, vec3 normal, vec3 view_dir);
void main()
{ 
    vec3 normal = normalize(f_in.normal);
    vec3 color = computeDirectLight(u_directLight, normal, u_view_dir);
    f_color = vec4(color, 1.0f);
    //f_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
}
vec3 computeDirectLight(DirectLight light, vec3 normal, vec3 view_dir)
{
    vec3 light_dir = normalize(light.direction);

	//ambient
	vec3 ambient = light.ambient * u_material.ambient;	//texture() return vec4

	//diffuse
	float diff = max(dot(normal, -light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * vec3(texture(u_material.t_diffuse, f_in.text));

	//specular
	vec3 reflect_dir = reflect(-light_dir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), u_material.shininess);
	vec3 specular = light.specular * spec * vec3(texture(u_material.t_specular, f_in.text));

	return (ambient + diffuse + specular);
}