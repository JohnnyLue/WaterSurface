#version 430 core

out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 clipSpace;
} f_in;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};


uniform sampler2D u_texture;
uniform samplerCube skybox;
uniform sampler2D dudv;

uniform vec4 color_ambient = vec4(1,1,1,1.0);
uniform vec4 color_diffuse = vec4(0.2,0.3,0.6,1.0);
uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform float shininess = 0.9;
uniform vec3 light_position = vec3(50.0f,32.0f,560.0f);
uniform vec3 eyeDirection = vec3(0,0,1);

void main()
{  
    
    vec4 wroldEyepos = inverse(u_view)*vec4(0,0,0,1);
    vec3 cameraPos = wroldEyepos.xyz;
    vec3 I = normalize(f_in.position- cameraPos);
    vec3 Rr;
    if(I.y>0)
        Rr = refract(I, normalize(-f_in.normal),0.75);
    else
        Rr = refract(I, normalize(f_in.normal),0.75);

    vec3 Re = reflect(I, normalize(f_in.normal));

    vec4 flectColor = vec4(texture(skybox, Re).rgb, 1.0);
    vec4 fractColor = vec4(texture(skybox, Rr).rgb, 1.0);

    vec2 ndc = (f_in.clipSpace.xy/f_in.clipSpace.w)/2.0+0.5;
    vec2 reflectCoord = ndc.xy;
    vec2 refractCoord = vec2(ndc.x,-ndc.y);

    //vec2 distort = (vec3(texture(dudv, vec2(reflectCoord.x,reflectCoord.y))).rg*2.0-1.0)*0.02;
    //reflectCoord += distort;
    //reflectCoord.x = clamp(reflectCoord.x,0.001,0.999);
    //reflectCoord.y = clamp(reflectCoord.y,0.001,0.999);

    vec3 color = vec3(texture(u_texture, reflectCoord));

    /*min(vec4(color,1.0)*color_ambient,vec4(1.0)) + diffuse*color_diffuse + specular*color_specular;*/

    f_color = mix(flectColor,vec4(color,1.0),abs(I.y));
}