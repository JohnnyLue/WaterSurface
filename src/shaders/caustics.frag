#version 430 core

in V_OUT
{
   vec2 texture_coordinate;
   vec3 rayDir;
} f_in;

out vec4 out_Color;

uniform sampler2D senceTexture;

uniform int boxNum;
uniform float boxScale;
uniform vec3 pos[100];

uniform bool useCaustics;

//uniform vec3 normal[10000];//to big = =, fucking size limit

vec3 lightPos = vec3(130,200,200);
float strong = 1;//light strong

layout (std140, binding = 0) uniform commom_matrices
{
    mat4 u_projection;
    mat4 u_view;
};



vec3 planePs[]={vec3(0,0,-1),vec3(0,0,1),vec3(-1,0,0),vec3(1,0,0),vec3(0,-1,0),vec3(0,1,0)};

vec3 planeNs[]={vec3(0,0,1),vec3(0,0,-1),vec3(1,0,0),vec3(-1,0,0),vec3(0,1,0),vec3(0,-1,0)};

vec3 Intersect(vec3 planeP, vec3 planeN, vec3 rayP, vec3 rayD)
{
    float d = dot(planeP, -planeN);
    float t = -(d + dot(rayP, planeN)) / dot(rayD, planeN);
    return rayP + t * rayD;
}

vec4 RayBoxs(vec3 rayP, vec3 rayD)
{
    vec3 resPos =vec3(999999);

    int collSurfaceNormal;

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
                    collSurfaceNormal = i;
                }
            }
        }
    }
    for(int i=4;i>=0;i--)
    {
        if(dot(rayD,planeNs[i])>0)continue;

        vec3 tempPos = Intersect(planePs[i]*50.0,planeNs[i],rayP,rayD);
        
        if(abs(tempPos.x)<=50.0+0.01&&tempPos.y<=10.0+0.01&&tempPos.y>=-50.0-0.01&&abs(tempPos.z)<=50.0+0.01)
        { 
            if(dot(vec3(tempPos-rayP),rayD)<0)
                    continue;
            if(distance(tempPos,rayP)<distance(resPos,rayP))
            { 
                resPos=tempPos;
                collSurfaceNormal = i;
            }
        }
    }

    for(int i=5;i>=0;i--)
    {
        if(dot(rayD,planeNs[i])>0)continue;

         vec3 tempPos = Intersect(planePs[i]*1000.0,planeNs[i],rayP,rayD);
        
        if(abs(tempPos.x)<=1000.0+0.01&&abs(tempPos.y)<=1000.0+0.01&&abs(tempPos.z)<=1000.0+0.01)
        { 
            if(dot(vec3(tempPos-rayP),rayD)<0)
                    continue;
            if(distance(tempPos,rayP)<distance(resPos,rayP))
            { 
                resPos=tempPos;
                collSurfaceNormal = -1;
            }
        }
    }

    return vec4(resPos.xyz,collSurfaceNormal);
}

//screen 600*600

vec3 distribute[33]={
vec3(0,1,0),
vec3(-0.3,1,0),vec3(-0.3,1,-0.3),vec3(-0.3,1,0.3),vec3(0.3,1,-0.3),vec3(0.3,1,0),vec3(0.3,1,0.3),vec3(0,1,-0.3),vec3(0,1,0.3),
vec3(-1.4,1,0),vec3(-1.4,1,-1.4),vec3(-1.4,1,1.4),vec3(1.4,1,-1.4),vec3(1.4,1,0),vec3(1.4,1,1.4),vec3(0,1,-1.4),vec3(0,1,1.4),
vec3(-2.6,1,0),vec3(-2.6,1,-2.6),vec3(-2.6,1,2.6),vec3(2.6,1,-2.6),vec3(2.6,1,0),vec3(2.6,1,2.6),vec3(0,1,-2.6),vec3(0,1,2.6),
vec3(-4.0,1,0),vec3(-4.0,1,-4.0),vec3(-4.0,1,4.0),vec3(4.0,1,-4.0),vec3(4.0,1,0),vec3(4.0,1,4.0),vec3(0,1,-4.0),vec3(0,1,4.0),
};

void main(void){
    
    vec4 wroldEyepos = inverse(u_view)*vec4(0,0,0,1);
    vec3 cameraPos = wroldEyepos.xyz;

    vec4 firstColl = RayBoxs(cameraPos,f_in.rayDir);//w value is surface normal index,-1 means far away

    //vec3 waterCollNormal;

    vec3 surfaceIntersectPos = Intersect(vec3(0), vec3(0,1,0), cameraPos,f_in.rayDir);

    if(firstColl.w == -1)
	    out_Color = texture(senceTexture,f_in.texture_coordinate);
    else{
        if(abs(surfaceIntersectPos.x)<50&&abs(surfaceIntersectPos.y)<50)//through water
        {
            vec3 I = normalize(surfaceIntersectPos- cameraPos);
            vec3 Rr = vec3(0,1,0);
            
            highp int index = int(floor(surfaceIntersectPos.x)*100 + floor(surfaceIntersectPos.y));
             

            //if(I.y>0)
            //    Rr = refract(I, -normalize(normal[index]),0.75);
            //else
            //    Rr = refract(I, normalize(normal[index]),0.75);

            
            firstColl = RayBoxs(surfaceIntersectPos,Rr);//a little error but i allow it.

        }

        float totalLight=0.0;

            for(int i=0;i<33;i++)
            {

                highp int w = int(firstColl.w);
                vec3 secondDir = normalize(distribute[i].x * planeNs[(w+2)%6]+ distribute[i].z * planeNs[(w+4)%6] + planeNs[w] );

                //check through water or not
                vec3 secondSurfaceIntersect = Intersect(vec3(0), vec3(0,1,0), firstColl.xyz ,secondDir);

                if(abs(secondSurfaceIntersect.x)<50&&abs(secondSurfaceIntersect.y)<50)//through water
                {
                    vec3 I = secondDir;
                    vec3 Rr=vec3(0,1,0);

                    highp int index = int(floor(secondSurfaceIntersect.x)*100 + floor(secondSurfaceIntersect.y));

                    //if(I.y>0)
                    //    Rr = refract(I, -normalize(normal[index]),0.75);
                    //else
                    //    Rr = refract(I, normalize(normal[index]),0.75);

                    if(dot(Rr,lightPos-secondSurfaceIntersect)>0)
                    {
                        vec3 diff = normalize(lightPos-secondSurfaceIntersect) - normalize(Rr);

                        totalLight += (sqrt(2)-length(diff))/(sqrt(2)*33.0); //average
                    }

                }else
                {
                    if(dot(secondDir,lightPos-firstColl.xyz)>0)
                    {
                        vec3 diff = normalize(lightPos-firstColl.xyz) - normalize(secondDir);

                        totalLight += (sqrt(2.0)-length(diff))/(sqrt(2.0)*33.0); //average
                    }
                }

            }

            out_Color = mix(vec4(1),texture(senceTexture,f_in.texture_coordinate) , totalLight*strong);

        }


}