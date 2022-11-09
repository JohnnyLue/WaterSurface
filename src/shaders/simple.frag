#version 430 core
out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} f_in;

uniform vec3 u_color;

uniform sampler2D u_texture;

void main()
{  
    float x = f_in.texture_coordinate.x;
    float y = 1.0 - f_in.texture_coordinate.y;

    float r = 1.0-sqrt(x*x+y*y);
    float g = 1.0-sqrt((x-0.5)*(x-0.5)+((sqrt(3.0)/2)-y)*((sqrt(3.0)/2)-y));
    float b = 1.0-sqrt((1.0-x)*(1.0-x)+y*y);

    vec3 color = vec3(texture(u_texture, f_in.texture_coordinate));
    f_color = vec4(color * vec3(r,g,b), 1.0f);
}