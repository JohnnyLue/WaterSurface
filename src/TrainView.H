/************************************************************************
     File:        TrainView.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						The TrainView is the window that actually shows the 
						train. Its a
						GL display canvas (Fl_Gl_Window).  It is held within 
						a TrainWindow
						that is the outer window with all the widgets. 
						The TrainView needs 
						to be aware of the window - since it might need to 
						check the widgets to see how to draw

	  Note:        we need to have pointers to this, but maybe not know 
						about it (beware circular references)

     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#pragma once

#include "RenderUtilities/BufferObject.h"
#include "RenderUtilities/Shader.h"
#include "RenderUtilities/Texture.h"
#include "WaterFrameBuffers.h"
// Preclarify for preventing the compiler error
class TrainWindow;
class CTrack;


//#######################################################################
// TODO
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//#######################################################################


// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

#include <AL/alut.h>
//#include <AL/al.h>
//#include <AL/alc.h>

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"

class waveSource {
	int x, y;
	float hei;
	float distance;
	
public:
	bool inUse;
	waveSource() {
		inUse = false;
	}

	float getHeight(int px, int py) {
		float totalHeight=0;
		int dis = (x - px) * (x - px) + (y - py) * (y - py);
		if (abs(dis - distance * distance) < 20) {
			totalHeight+= hei * (20 - abs(dis - distance * distance));
		}
		 dis = (-x - px) * (-x - px) + (y - py) * (y - py);
		if (abs(dis - distance * distance) < 20) {
			totalHeight += hei * (20 - abs(dis - distance * distance));
		}
		 dis = (x - px) * (x - px) + (-y - py) * (-y - py);
		if (abs(dis - distance * distance) < 20) {
			totalHeight += hei * (20 - abs(dis - distance * distance));
		}
		 dis = (99 * 2 - x - px) * (99 * 2 - x - px) + (y - py) * (y - py);
		if (abs(dis - distance * distance) < 20) {
			totalHeight += hei * (20 - abs(dis - distance * distance));
		}
		 dis = (x - px) * (x - px) + (99 * 2 - y - py) * (99 * 2 - y - py);
		if (abs(dis - distance * distance) < 20) {
			totalHeight += hei * (20 - abs(dis - distance * distance));
		}

		return totalHeight;
	}

	void update(float waveSpeed)
	{
		if (dies())
			return;
		hei *= 0.9;
		distance += waveSpeed;
	}

	bool dies()
	{
		if(abs(hei) < .005)
			inUse = false;
		return abs(hei) < .005;
	}

	void set(int x, int y, float hei)
	{
		inUse = true;
		this->x = x, this->y = y, this->hei = hei;
		distance = 0;
	}
};

class TrainView : public Fl_Gl_Window
{
	public:
		// note that we keep the "standard widget" constructor arguments
		TrainView(int x, int y, int w, int h, const char* l = 0);

		// overrides of important window things
		virtual int handle(int);
		virtual void draw();

		// all of the actual drawing happens in this routine
		// it has to be encapsulated, since we draw differently if
		// we're drawing shadows (no colors, for example)
		void drawStuff(bool doingShadows=false);

		// setup the projection - assuming that the projection stack has been
		// cleared for you
		void setProjection();

		// Reset the Arc ball control
		void resetArcball();

		// pick a point (for when the mouse goes down)
		void doPick();

		//set ubo
		void setUBO();

		float wave(int i, float x, float y);

		float waveHeight(float x, float y);

		void drawSence(int mode,bool cube=true);

		void updateHeightMap();

		void averageHeight(float* heightMap);
		
		void addSource(int x, int y, float height);
	public:
		ArcBallCam		arcball;			// keep an ArcBall for the UI
		
		int				selectedCube = -1;  // simple - just remember which cube is selected

		float eyex, eyey, eyez;

		TrainWindow*	tw;				// The parent of this display window
		CTrack*			m_pTrack;		// The track of the entire scene

		Shader* shader			= nullptr;
		Shader* tileShader		= nullptr;
		Shader* skyboxShader	= nullptr;
		Shader* guiShader		= nullptr;

		Texture2D* texture			= nullptr;
		Texture2D* textureTile		= nullptr;
		TextureSkyBox* textureSky	= nullptr;
		Texture2D* DUDVmap			= nullptr;

		VAO* plane			= nullptr;
		VAO* pool			= nullptr;
		VAO* skybox			= nullptr;
		UBO* commom_matrices= nullptr;
		UBO* points			= nullptr;

		//OpenAL
		glm::vec3 source_pos;
		glm::vec3 listener_pos;
		ALCdevice* device	= nullptr;
		ALCcontext* context = nullptr;
		ALuint source;
		ALuint buffer;

		const float pi = 3.14159;
		float waterHeight;
		float time;
		int numWaves;
		float amplitude[8];
		float wavelength[8];
		float speed[8];
		float direction[8][2];
		
		//control waves
		bool onDrag = false;
		int waveTimeBreak = 300;
		float heightMap[100 * 100];

		waveSource sources[100];

};
