#version 430 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coordinate;

uniform mat4 u_model;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

out V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 clipSpace;
} v_out;


void main()
{
   

    v_out.clipSpace= u_projection * u_view *u_model * vec4(position, 1.0f);
    gl_Position = v_out.clipSpace;
    vec3 oriPos = vec3(position.x,0,position.z);
    vec4 origlPos = vec4(u_projection * u_view * u_model * vec4(oriPos, 1.0f));
    vec4 texpos = origlPos/(2*origlPos.z)+vec4(0.5,0.5,0,0);
    v_out.position = vec3( u_model*vec4(position, 1.0f));
    v_out.normal = mat3(transpose(inverse(u_model))) * normal;
    v_out.texture_coordinate = vec2(position.x/2.0+0.5,position.y/2.0+0.5)*6.0;
}