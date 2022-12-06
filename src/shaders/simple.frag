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


uniform sampler2D refract_texture;
uniform sampler2D refract_no_cube_texture;
uniform sampler2D reflect_texture;

uniform int boxNum;
uniform float boxScale;
uniform vec3 pos[100];

uniform bool both;
uniform bool reflectOnly;
uniform bool refractOnly;

bool encountered;

vec3 planePs[]={vec3(0,0,-1),vec3(0,0,1),vec3(-1,0,0),vec3(1,0,0),vec3(0,-1,0),vec3(0,1,0)};

vec3 planeNs[]={vec3(0,0,1),vec3(0,0,-1),vec3(1,0,0),vec3(-1,0,0),vec3(0,1,0),vec3(0,-1,0)};

vec3 Intersect(vec3 planeP, vec3 planeN, vec3 rayP, vec3 rayD)
{
    float d = dot(planeP, -planeN);
    float t = -(d + dot(rayP, planeN)) / dot(rayD, planeN);
    return rayP + t * rayD;
}

vec3 RayBoxs(vec3 rayP, vec3 rayD)
{
    vec3 resPos =vec3(999);
    encountered = false;

    for(int n=0;n<boxNum;n++)
    {
        vec3 boxPs[]={
        vec3(pos[n].x ,pos[n].y ,pos[n].z-boxScale),
        vec3(pos[n].x ,pos[n].y ,pos[n].z+boxScale),
        vec3(pos[n].x-boxScale,pos[n].y ,pos[n].z ),
        vec3(pos[n].x+boxScale,pos[n].y ,pos[n].z ),
        vec3(pos[n].x ,pos[n].y-boxScale,pos[n].z ),
        vec3(pos[n].x ,pos[n].y+boxScale,pos[n].z ),
        };

        for(int i=5;i>=0;i--)
        {

            vec3 tempPos = Intersect(boxPs[i],planeNs[i],rayP,rayD);
            
            
            if(
                tempPos.x<pos[n].x+boxScale+0.001&&
                tempPos.x>pos[n].x-boxScale-0.001&&
                tempPos.y<pos[n].y+boxScale+0.001&&
                tempPos.y>pos[n].y-boxScale-0.001&&
                tempPos.z<pos[n].z+boxScale+0.001&&
                tempPos.z>pos[n].z-boxScale-0.001)
            {
                if(dot(vec3(tempPos-rayP),rayD)<0)
                    continue;
                if(distance(tempPos,rayP)<distance(resPos,rayP))
                { 
                    resPos=tempPos;
                    encountered = true;
                }
            }
        }
    }
    for(int i=5;i>=0;i--)
    {
        if(dot(rayD,planeNs[i])>0)continue;

        vec3 tempPos = Intersect(planePs[i]*50.0,planeNs[i],rayP,rayD);
        
        if(abs(tempPos.x)<=50.0+0.1&&tempPos.y>=-50.0-0.1&&abs(tempPos.z)<=50.0+0.1)
        { 
            if(distance(tempPos,rayP)<distance(resPos,rayP))
            { 
                resPos=tempPos;
                encountered = false;
                break;
            }
        }
    }

    return resPos;
}

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

    vec3 oriRr;
    if(I.y>0)
        oriRr = refract(I, vec3(0,-1,0),0.75);
    else
        oriRr = refract(I,vec3(0,1,0),0.75);

    vec3 Re = reflect(I, normalize(f_in.normal));
    //=====================================================================
    vec3 refractPos = RayBoxs(f_in.position, I + Rr - oriRr);
    vec4 refractClip;
    if (cameraPos.y > 0)
	{
		mat4 refract = mat4(
		1.0,0.0,0.0,0.0,
		0.0,0.75,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0
		);
		refractClip= u_projection * u_view * refract * vec4(refractPos, 1.0f); 

	}
	else
	{
		mat4 refract= mat4(
		1.0,0.0,0.0,0.0,
		0.0,1.33,0.0,0.0,
		0.0,0.0,1.0,0.0,
		0.0,0.0,0.0,1.0
		);
		refractClip= u_projection * u_view * refract* vec4(refractPos, 1.0f); 
	}
    refractClip = u_projection * u_view * vec4(refractPos, 1.0f); 

    vec2 ndc = (refractClip.xy/refractClip.w)/2.0+0.5;
    vec2 refractCoord = ndc.xy;

    vec3  fractColor;
    if(true)
        fractColor= vec3(texture(refract_texture, refractCoord));
    else
        fractColor= vec3(texture(refract_no_cube_texture, refractCoord));
    //====================================================================
    vec3 reflectPos = RayBoxs(f_in.position, vec3(Re.x,-Re.y,Re.z));

    vec4 reflectClip = u_projection * u_view * vec4(reflectPos, 1.0f); 

    //vec2 ndc2 = (f_in.clipSpace.xy/f_in.clipSpace.w)/2.0+0.5;
    vec2 ndc2 = (reflectClip.xy/reflectClip.w)/2.0+0.5;
    vec2 relectCoord = vec2(ndc2.x,ndc2.y);

    vec3 flectColor = vec3(texture(reflect_texture,relectCoord));

    //===================================================================
    if(both)
        f_color = mix(vec4(0,0,1,1),mix(vec4(flectColor,1.0),vec4(fractColor,1.0),abs(I.y)),0.95);
    else if(reflectOnly)
        f_color  = vec4(flectColor,1.0);
    else
        f_color  = vec4(fractColor,1.0);
}