#version 430 core

in vec2 position;

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};

out V_OUT
{
   vec2 texture_coordinate;
   vec3 rayDir;
} v_out;

void main(void){

	vec4 wroldEyepos = inverse(u_view)*vec4(0,0,0,1);
    vec3 cameraPos = wroldEyepos.xyz;

	vec3 camDir = normalize(vec3(inverse(u_view)*vec4(0,0,-1,1))-cameraPos);
	vec3 camUp = normalize(vec3(inverse(u_view)*vec4(0,1,0,1))-cameraPos);

	gl_Position = vec4(vec2(position.x,position.y), 0.0, 1.0);
	v_out.texture_coordinate = vec2((position.x+1.0)/2.0, (position.y+1.0)/2.0);
	
	vec3 left = normalize(cross(camUp,camDir));

	v_out.rayDir = normalize(-left * gl_Position.x + camUp * gl_Position.y + camDir*sqrt(7));//fov = 60
}