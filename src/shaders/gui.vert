#version 140

in vec2 position;

out vec2 textureCoords;

uniform mat4 transformationMatrix;

uniform float scale;
uniform vec2 pos;

void main(void){

	gl_Position = vec4(scale*vec2(position.x+pos.x,position.y+pos.y), 0.0, 1.0);
	textureCoords = vec2((position.x+1.0)/2.0, (position.y+1.0)/2.0);
}