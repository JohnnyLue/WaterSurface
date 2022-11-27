#version 430 core

out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} f_in;


uniform sampler2D u_texture;
uniform sampler2D dudv;


void main()
{  
    
    vec3 color = vec3(texture(u_texture, f_in.texture_coordinate));

    f_color =mix(vec4(color,1.0),vec4(0,0.5,0.5,1),0.3);
}