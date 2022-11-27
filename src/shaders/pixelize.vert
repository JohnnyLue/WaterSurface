#version 430 core
in vec2 position;

out vec2 textureCoords;


void main(void)
{
    gl_Position = vec4(position, 0.0, 1.0);
	/*
	float stepx = 1.0/blockx;
	float stepy = 1.0/blocky;
	*/
	vec2 newposition = (position)/2.0 + vec2(0.5);
	/*
	int pixelX = 0;
	int pixelY = 0;
	for(int i=0;i<blockx;i++)
	{
	
		if(pixelX*stepx < newposition.x)
			pixelX += 1;
		if(pixelY*stepy < newposition.y)
			pixelY += 1;

	}
	
	float list[]={0.0,0.25,0.5,0.75,1.0};
	for(int i=0;i<4;i++)
	{
		if(newposition.x-list[i]<0)
		{
			newposition.x = list[i];
			break;
		}
	}for(int i=0;i<4;i++)
	{
		if(newposition.y-list[i]<0)
		{
			newposition.y = list[i];
			break;
		}
	}
	*/
	textureCoords = vec2(newposition.x, newposition.y);
}