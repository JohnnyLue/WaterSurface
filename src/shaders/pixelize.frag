#version 430 core

in vec2 textureCoords;


uniform sampler2D Texture;

const vec2 TexSize = vec2(1000, 1000);


uniform int blockx;
uniform int blocky;


void main(){

    vec2 MosaicSize = vec2(blockx, blocky);
    vec2 intXY = vec2(textureCoords.x * TexSize.x, textureCoords.y * TexSize.y);
    

    vec2 XYMosaic = vec2(floor(intXY.x/MosaicSize.x)*MosaicSize.x, floor(intXY.y/MosaicSize.y)*MosaicSize.y);

    vec2 UVMosaic = vec2(XYMosaic.x/TexSize.x, XYMosaic.y/TexSize.y);
    vec4 color = texture2D(Texture, UVMosaic);
    gl_FragColor = color;
}