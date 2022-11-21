#version 430 core

out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} f_in;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

uniform vec3 u_color;

uniform vec3 eyepos;

uniform sampler2D u_texture;
uniform samplerCube skybox;

uniform vec4 color_ambient = vec4(1,1,1,1.0);
uniform vec4 color_diffuse = vec4(0.2,0.3,0.6,1.0);
uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform float shininess = 0.9;
uniform vec3 light_position = vec3(50.0f,32.0f,560.0f);
uniform vec3 eyeDirection = vec3(0,0,1);

void main()
{  
    
    vec4 wroldEyepos = inverse(u_view)*inverse(u_projection)*vec4(0,0,0,1);
    vec3 cameraPos = wroldEyepos.xyz;
    vec3 I = normalize(f_in.position- cameraPos);
    vec3 R = reflect(I, normalize(f_in.normal));

    vec4 flectColor = vec4(texture(skybox, R).rgb, 1.0);

    float x = f_in.texture_coordinate.x;
    float y = 1.0 - f_in.texture_coordinate.y;

    vec3 light_direction = normalize(light_position - f_in.position);
  
    vec3 half_vector = normalize(normalize(light_direction)+normalize(eyeDirection));
    float diffuse = max(0.0,dot(f_in.normal,light_direction));
    float specular = pow(max(0,dot(f_in.normal,half_vector)),shininess);

    vec3 color = vec3(texture(u_texture, f_in.texture_coordinate));
    /*min(vec4(color,1.0)*color_ambient,vec4(1.0)) + diffuse*color_diffuse + specular*color_specular;*/
    f_color = flectColor;
}