out vec4 f_color;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
} f_in;

uniform vec3 u_color;

uniform sampler2D u_texture;

uniform vec4 color_ambient = vec4(1,1,1,1.0);
uniform vec4 color_diffuse = vec4(0.2,0.3,0.6,1.0);
uniform vec4 color_specular = vec4(1.0,1.0,1.0,1.0);
uniform float shininess = 1.0f;
uniform vec3 light_position = vec3(50.0f,32.0f,560.0f);
uniform vec3 eyeDirection = vec3(0,0,1);

void main()
{  
    float x = f_in.texture_coordinate.x;
    float y = 1.0 - f_in.texture_coordinate.y;

    vec3 light_direction = normalize(light_position - f_in.position);
  
    vec3 half_vector = normalize(normalize(light_direction)+normalize(eyeDirection));
    float diffuse = max(0.0,dot(f_in.normal,light_direction));
    float specular = pow(max(0.0,dot(f_in.normal,half_vector)),shininess);

    vec3 color = vec3(texture(u_texture, f_in.texture_coordinate));
    f_color = min(vec4(color,1.0)*color_ambient,vec4(1.0)) + diffuse*color_diffuse*5 + specular*color_specular*5;
}