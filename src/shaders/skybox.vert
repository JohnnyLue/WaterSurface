#version 430 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

void main()
{
    TexCoords = aPos;
    vec4 pos =  u_projection * mat4(mat3(u_view)) * vec4(aPos, 1.0f);
    gl_Position = pos.xyww;
}  