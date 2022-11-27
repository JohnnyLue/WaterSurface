/************************************************************************
     File:        TrainView.cpp

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

#include <iostream>
#include <Fl/fl.h>

// we will need OpenGL, and OpenGL needs windows.h
#include <windows.h>
//#include "GL/gl.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GL/glu.h>
#include <cmath>
#include <time.h>

#include "TrainView.H"
#include "TrainWindow.H"
#include "Utilities/3DUtils.H"



#ifdef EXAMPLE_SOLUTION
#	include "TrainExample/TrainExample.H"
#endif


//************************************************************************
//
// * Constructor to set up the GL window
//========================================================================
TrainView::
TrainView(int x, int y, int w, int h, const char* l) 
	: Fl_Gl_Window(x,y,w,h,l)
//========================================================================
{
	mode( FL_RGB|FL_ALPHA|FL_DOUBLE | FL_STENCIL );

	resetArcball();

	waterHeight=5;
	time=0;
	numWaves=0;
	amplitude[0] = 0.2;
	amplitude[1] = 0.3;
	amplitude[2] = 0.3;
	wavelength[0] = 20;
	wavelength[1] = 36;
	wavelength[2] = 41;
	speed[0] = 5;
	speed[1] = 3;
	speed[2] = 3.8;
	Pnt3f dir1(0, -1.0, 1.0);
	Pnt3f dir2(0.0, 1.0, 0.0);
	Pnt3f dir3(1.0, -1.0, 0.0);
	dir1.normalize();
	dir2.normalize();
	dir3.normalize();
	direction[0][0] = dir1.x;
	direction[0][1] = dir1.y;
	direction[1][0] = dir2.x;
	direction[1][1] = dir2.y;
	direction[2][0] = dir3.x;
	direction[2][1] = dir3.y;

	srand(std::time(0));
	memset(heightMap, 0, 100 * 100* sizeof(float));
}

//************************************************************************
//
// * Reset the camera to look at the world
//========================================================================
void TrainView::
resetArcball()
//========================================================================
{
	// Set up the camera to look at the world
	// these parameters might seem magical, and they kindof are
	// a little trial and error goes a long way
	arcball.setup(this, 40, 250, .2f, .4f, 0);
}

//************************************************************************
//
// * FlTk Event handler for the window
//########################################################################
// TODO: 
//       if you want to make the train respond to other events 
//       (like key presses), you might want to hack this.
//########################################################################
//========================================================================
int TrainView::handle(int event)
{
	// see if the ArcBall will handle the event - if it does, 
	// then we're done
	// note: the arcball only gets the event if we're in world view
	if (tw->worldCam->value())
		if (arcball.handle(event)) 
			return 1;

	// remember what button was used
	static int last_push;

	switch(event) {
		// Mouse button being pushed event
		case FL_PUSH:
			last_push = Fl::event_button();
			// if the left button be pushed is left mouse button
			if (last_push == FL_LEFT_MOUSE  ) {
				doPick();
				damage(1);
				return 1;
			};
			break;

	   // Mouse button release event
		case FL_RELEASE: // button release
			damage(1);
			last_push = 0;
			return 1;

		// Mouse button drag event
		case FL_DRAG:

			// Compute the new control point position
			if ((last_push == FL_LEFT_MOUSE) && (selectedCube >= 0)) {
				ControlPoint* cp = &m_pTrack->points[selectedCube];

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x, r1y, r1z, r2x, r2y, r2z);

				double rx, ry, rz;
				mousePoleGo(r1x, r1y, r1z, r2x, r2y, r2z, 
								static_cast<double>(cp->pos.x), 
								static_cast<double>(cp->pos.y),
								static_cast<double>(cp->pos.z),
								rx, ry, rz,
								(Fl::event_state() & FL_CTRL) != 0);

				cp->pos.x = (float) rx;
				cp->pos.y = (float) ry;
				cp->pos.z = (float) rz;
				damage(1);
			}
			break;

		// in order to get keyboard events, we need to accept focus
		case FL_FOCUS:
			return 1;

		// every time the mouse enters this window, aggressively take focus
		case FL_ENTER:	
			focus(this);
			break;

		case FL_KEYBOARD:
		 		int k = Fl::event_key();
				int ks = Fl::event_state();
				if (k == 'p') {
					// Print out the selected control point information
					if (selectedCube >= 0) 
						printf("Selected(%d) (%g %g %g) (%g %g %g)\n",
								 selectedCube,
								 m_pTrack->points[selectedCube].pos.x,
								 m_pTrack->points[selectedCube].pos.y,
								 m_pTrack->points[selectedCube].pos.z,
								 m_pTrack->points[selectedCube].orient.x,
								 m_pTrack->points[selectedCube].orient.y,
								 m_pTrack->points[selectedCube].orient.z);
					else
						printf("Nothing Selected\n");

					return 1;
				};
				break;
	}

	return Fl_Gl_Window::handle(event);
}


float TrainView::wave(int i, float x, float y) {
	float frequency = 2 * pi / wavelength[i];
	float phase = speed[i] * frequency;
	float theta = direction[i][0] * x + direction[i][1] * y;
	return amplitude[i] * sin(theta * frequency + time * phase);
}

float TrainView::waveHeight(float x, float y) {
	float height = 0.0;
	for (int i = 0; i < numWaves; ++i)
		height += wave(i, x, y);
	return height;
}
void TrainView::drawSence()
{
	//###############################################################
	// pool
	// 
	//bind shader

	glm::mat4 model_matrix_pool = glm::mat4();
	model_matrix_pool = glm::translate(model_matrix_pool, this->source_pos);
	model_matrix_pool = glm::scale(model_matrix_pool, glm::vec3(100.0f, 100.0f, 100.0f));
	model_matrix_pool = glm::translate(model_matrix_pool, glm::vec3(0, -0.35, 0));
	this->tileShader->Use();
	this->tileShader->setMat4("u_model", model_matrix_pool);

	this->textureTile->bind(0);
	glUniform1i(glGetUniformLocation(this->tileShader->Program, "u_texture"), 0);

	//bind VAO
	glBindVertexArray(this->pool->vao);

	glDrawArrays(GL_TRIANGLES, 0, 30);
	//unbind VAO
	glBindVertexArray(0);
	//###############################################################

	//###############################################################
	// skybox
	// 
	// draw skybox as last
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	
	//bind shader
	this->skyboxShader->Use();
	this->textureSky->bind(0);
	glUniform1i(glGetUniformLocation(this->skyboxShader->Program, "skybox"), 0);

	// skybox cube
	glBindVertexArray(this->skybox->vao);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
	glDepthMask(GL_TRUE);

	//###############################################################

	//###############################################################
	// plane
	// 
	//bind shader
	//this->shader->Use();
	//
	//glm::mat4 model_matrix = glm::mat4();
	//model_matrix = glm::translate(model_matrix, this->source_pos);
	//
	//this->shader->setMat4("u_model", model_matrix);
	//
	//this->texture->bind(0, "u_texture");
	//
	////bind VAO
	//glBindVertexArray(this->plane->vao);
	//
	//glDrawElements(GL_TRIANGLES, this->plane->element_amount, GL_UNSIGNED_INT, 0);
	//
	////unbind VAO
	//glBindVertexArray(0);
	//###############################################################
}
void TrainView::updateHeightMap()
{
	srand(rand()+5);
	static int lasttime = -1;
	if (autoGenerateWaves && (lasttime<0|| std::clock()-lasttime>waveTimeBreak)&&sources.size()< maxSources)
	{
		lasttime = std::clock();
		int x = rand() % 100;
		int y = rand() % 100;
		//heightMap[y * 100 + x]+=1.0;
		sources.push_back(new waveSource(x, y, waveHeigh));
	}
	if(!sources.empty())
	for (auto iter = sources.begin(); iter != sources.end(); iter++)
	{
		(*iter)->update(waveSpeed);
		if ((*iter)->dies())
		{
			delete (*iter);
			iter = sources.erase(iter);

			if (iter == sources.begin())
				break;
			else
				iter--;
		}

	}
	float newHeight[100 * 100];
	memset(newHeight, 0, 100 * 100 * sizeof(float));
	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			for (auto iter = sources.begin(); iter != sources.end(); iter++)
				newHeight[i * 100 + j] += (*iter)->getHeight(i, j);
		}
	}

	float moreMewHeight[100 * 100];
	memset(moreMewHeight, 0, 100 * 100 * sizeof(float));

	for (int i = 0; i < 100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			for (int x = -1; x < 1; x++)
			{
				for (int y = -1; y < 1; y++)
				{
					int posx = i + x, posy = j + y;
					if (posx < 0)posx *= -1;
					if (posx >= 100)posx -= 2 * (posx - 100) + 1;
					if (posy < 0)posy *= -1;
					if (posy >= 100)posy -= 2 * (posy - 100) + 1;

					moreMewHeight[i * 100 + j] += newHeight[posx * 100 + posy]/9.0;

				}
			}
		}
	}

	memcpy(heightMap, moreMewHeight, 100 * 100*sizeof(float));
}
//************************************************************************
//
// * this is the code that actually draws the window
//   it puts a lot of the work into other routines to simplify things
//========================================================================
void TrainView::draw()
{

	//*********************************************************************
	//
	// * Set up basic opengl informaiton
	//
	//**********************************************************************
	//initialized glad
	if (gladLoadGL())
	{
		// Set up the view port
		glViewport(0, 0, w(), h());

		// clear the window, be sure to clear the Z-Buffer too
		glClearColor(0, 0, .3f, 0);		// background should be blue

		// we need to clear out the stencil buffer since we'll use
		// it for shadows
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH);

		// Blayne prefers GL_DIFFUSE
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

		// prepare for projection
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		setProjection();		// put the code to set up matrices here

		//######################################################################
		// TODO: 
		// you might want to set the lighting up differently. if you do, 
		// we need to set up the lights AFTER setting up the projection
		//######################################################################
		// enable the lighting
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		// top view only needs one light
		if (tw->topCam->value()) {
			glDisable(GL_LIGHT1);
			glDisable(GL_LIGHT2);
		}
		else {
			glEnable(GL_LIGHT1);
			glEnable(GL_LIGHT2);
		}

		//initiailize VAO, VBO, Shader...
		
		if (!this->guiShader)
			this->guiShader = new
			Shader(
				PROJECT_DIR "/src/shaders/gui.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/gui.frag");

		if(!this->skyboxShader)
			this->skyboxShader = new
			Shader(
				PROJECT_DIR "/src/shaders/skybox.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/skybox.frag");

		if (!this->shader)
			this->shader = new
			Shader(
				PROJECT_DIR "/src/shaders/simple.vert",
				nullptr, nullptr, nullptr, 
				PROJECT_DIR "/src/shaders/simple.frag");
		
		if (!this->tileShader)
			this->tileShader = new
			Shader(
				PROJECT_DIR "/src/shaders/tile.vert",
				nullptr, nullptr, nullptr,
				PROJECT_DIR "/src/shaders/tile.frag");

		if (!this->commom_matrices)
			this->commom_matrices = new UBO();
		this->commom_matrices->size = 2 * sizeof(glm::mat4);
		glGenBuffers(1, &this->commom_matrices->ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
		glBufferData(GL_UNIFORM_BUFFER, this->commom_matrices->size, NULL, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);


		//set water surface

		GLfloat  vertices[10000 * 3];
		GLfloat  normal[10000 * 3];
		GLfloat  texture_coordinate[10000 * 2];
		GLuint element[99 * 99 * 2 * 3];

		
		float t = time;
		for (int i = 0; i < 100; i++)
		{
			for (int j = 0; j < 100; j++,t+=0.5)
			{
				float posx = i - 50, posy = 1, posz = j - 50;
				vertices[(i * 100 + j)*3 + 0] = posx;
				vertices[(i * 100 + j) * 3 + 1] = 3+this->heightMap[i*100+j];
				vertices[(i * 100 + j)*3 + 2] = posz;

				texture_coordinate[(i * 100 + j)*2 + 0] = (i) / 100.0;
				texture_coordinate[(i * 100 + j)*2 + 1] = (j) / 100.0;
			}
		}

		for (int i = 0; i < 100; i++)
		{
			for (int j = 0; j < 100; j++)
			{
				Pnt3f pos=Pnt3f(vertices[(i * 100 + j) * 3 + 0], vertices[(i * 100 + j) * 3 + 1], vertices[(i * 100 + j) * 3 + 2]);
				Pnt3f norm;

				if (i == 99 && j == 99)
				{
					Pnt3f a = Pnt3f(vertices[(i * 100 + j-1) * 3 + 0], vertices[(i * 100 + j-1) * 3 + 1], vertices[(i * 100 + j-1) * 3 + 2]) + -1 * pos;
					Pnt3f b = Pnt3f(vertices[((i - 1) * 100 + j) * 3 + 0], vertices[((i - 1) * 100 + j ) * 3 + 1], vertices[((i - 1) * 100 + j) * 3 + 2]) + -1 * pos;

					norm = b * a;
					norm.normalize();
				}
				else if (i == 99)
				{
					Pnt3f a = Pnt3f(vertices[(i * 100 + j + 1) * 3 + 0], vertices[(i * 100 + j + 1) * 3 + 1], vertices[(i * 100 + j + 1) * 3 + 2]) + -1 * pos;
					Pnt3f b = Pnt3f(vertices[((i - 1) * 100 + j) * 3 + 0], vertices[((i - 1) * 100 + j) * 3 + 1], vertices[((i - 1) * 100 + j) * 3 + 2]) + -1 * pos;

					norm = b * a;
					norm.normalize();
				}
				else if (j == 99)
				{
					Pnt3f a = Pnt3f(vertices[(i * 100 + j - 1) * 3 + 0], vertices[(i * 100 + j - 1) * 3 + 1], vertices[(i * 100 + j - 1) * 3 + 2]) + -1 * pos;
					Pnt3f b = Pnt3f(vertices[((i + 1) * 100 + j) * 3 + 0], vertices[((i + 1) * 100 + j) * 3 + 1], vertices[((i + 1) * 100 + j) * 3 + 2]) + -1 * pos;

					norm = b * a;
					norm.normalize();
				}
				else {
					Pnt3f a = Pnt3f(vertices[(i * 100 + j + 1) * 3 + 0], vertices[(i * 100 + j + 1) * 3 + 1], vertices[(i * 100 + j + 1) * 3 + 2]) + -1 * pos;
					Pnt3f b = Pnt3f(vertices[((i + 1) * 100 + j) * 3 + 0], vertices[((i + 1) * 100 + j) * 3 + 1], vertices[((i + 1) * 100 + j) * 3 + 2]) + -1 * pos;

					norm = a * b;
					norm.normalize();
				}

				normal[(i * 100 + j)*3 + 0] = norm.x;
				normal[(i * 100 + j)*3 + 1] = norm.y;
				normal[(i * 100 + j)*3 + 2] = norm.z;

			}
		}

		
		
		for (int i = 0; i < 100 - 1; i++)
		{
			for (int j = 0; j < 100 - 1; j++)
			{
				element[(i * 99 + j)*6 + 0] = i * 100 + j;
				element[(i * 99 + j)*6 + 1] = i * 100 + j + 1;
				element[(i * 99 + j)*6 + 2] = (i + 1) * 100 + j + 1;
							
				element[(i * 99 + j)*6 + 3] = i * 100 + j;
				element[(i * 99 + j)*6 + 4] = (i + 1) * 100 + j;
				element[(i * 99 + j)*6 + 5] = (i + 1) * 100 + j + 1;
			}
		}
		

		this->plane = new VAO;
		this->plane->element_amount = sizeof(element) / sizeof(GLuint);
		glGenVertexArrays(1, &this->plane->vao);
		glGenBuffers(3, this->plane->vbo);
		glGenBuffers(1, &this->plane->ebo);

		glBindVertexArray(this->plane->vao);

		// Position attribute
		glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		// Normal attribute
		glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normal), normal, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);

		// Texture Coordinate attribute
		glBindBuffer(GL_ARRAY_BUFFER, this->plane->vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texture_coordinate), texture_coordinate, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(2);

		//Element attribute
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->plane->ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(element), element, GL_STATIC_DRAW);

		// Unbind VAO
		glBindVertexArray(0);
	

		GLfloat poolVertex[] = {
			// positions		  //normal  // texture Coords
			-0.5f, -0.5f, -0.5f,  0,0,1,    0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  0,0,1,    1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  0,0,1,    1.0f, 1.0f,
			 0.5f,  0.5f, -0.5f,  0,0,1,    1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0,0,1,    0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0,0,1,    0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0,0,-1,   0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0,0,-1,   1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  0,0,-1,   1.0f, 1.0f,
			 0.5f,  0.5f,  0.5f,  0,0,-1,   1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0,0,-1,   0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0,0,-1,   0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1,0,0,    1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1,0,0,    1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  1,0,0,    0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  1,0,0,    0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  1,0,0,    0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1,0,0,    1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,  -1,0,0,   1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  -1,0,0,   1.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  -1,0,0,   0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  -1,0,0,   0.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  -1,0,0,   0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  -1,0,0,   1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0,1,0,    0.0f, 1.0f,
			 0.5f, -0.5f, -0.5f,  0,1,0,    1.0f, 1.0f,
			 0.5f, -0.5f,  0.5f,  0,1,0,    1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0,1,0,    1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0,1,0,    0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0,1,0,    0.0f, 1.0f,
		};

		// pool VAO
		this->pool = new VAO();
		glGenVertexArrays(1, &pool->vao);
		glGenBuffers(1, &pool->vbo[0]);
		glBindVertexArray(pool->vao);
		
		glBindBuffer(GL_ARRAY_BUFFER, pool->vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(poolVertex), &poolVertex, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);
		// Unbind VAO
		glBindVertexArray(0);

		
		float skyboxVertices[] = {
			// positions          
			-1.0f,  1.0f, -1.0f,
			-1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f, -1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,

			-1.0f, -1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f,
			-1.0f, -1.0f,  1.0f,

			-1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f, -1.0f,
			 1.0f,  1.0f,  1.0f,
			 1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f,  1.0f,
			-1.0f,  1.0f, -1.0f,

			-1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f, -1.0f,
			 1.0f, -1.0f, -1.0f,
			-1.0f, -1.0f,  1.0f,
			 1.0f, -1.0f,  1.0f
		};
		
		// skybox VAO
		this->skybox = new VAO();
		glGenVertexArrays(1, &skybox->vao);
		glGenBuffers(1, &skybox->vbo[0]);
		glBindVertexArray(skybox->vao);
		glBindBuffer(GL_ARRAY_BUFFER, skybox->vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
		
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// Unbind VAO
		glBindVertexArray(0);
		
		if (!this->DUDVmap)
			this->DUDVmap = new Texture2D(PROJECT_DIR "/Images/dudv.jpg");
		
		if (!this->texture)
			this->texture = new Texture2D(PROJECT_DIR "/Images/water.jpg");

		if (!this->textureTile)
			this->textureTile = new Texture2D(PROJECT_DIR "/Images/pool tile.jpg");
		
		if (!this->textureSky)
		{
			vector<std::string> faces
			{
				PROJECT_DIR "/skybox/right.jpg",
				PROJECT_DIR "/skybox/left.jpg",
				PROJECT_DIR "/skybox/top.jpg",
				PROJECT_DIR "/skybox/bottom.jpg",
				PROJECT_DIR "/skybox/front.jpg",
				PROJECT_DIR "/skybox/back.jpg"
			};
			this->textureSky = new TextureSkyBox(faces);
		}
		if (!this->device){
			//Tutorial: https://ffainelli.github.io/openal-example/
			this->device = alcOpenDevice(NULL);
			if (!this->device)
				puts("ERROR::NO_AUDIO_DEVICE");

			ALboolean enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
			if (enumeration == AL_FALSE)
				puts("Enumeration not supported");
			else
				puts("Enumeration supported");

			this->context = alcCreateContext(this->device, NULL);
			if (!alcMakeContextCurrent(context))
				puts("Failed to make context current");

			this->source_pos = glm::vec3(0.0f, 5.0f, 0.0f);

			ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
			alListener3f(AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alListener3f(AL_VELOCITY, 0, 0, 0);
			alListenerfv(AL_ORIENTATION, listenerOri);

			alGenSources((ALuint)1, &this->source);
			alSourcef(this->source, AL_PITCH, 1);
			alSourcef(this->source, AL_GAIN, 1.0f);
			alSource3f(this->source, AL_POSITION, source_pos.x, source_pos.y, source_pos.z);
			alSource3f(this->source, AL_VELOCITY, 0, 0, 0);
			alSourcei(this->source, AL_LOOPING, AL_TRUE);

			alGenBuffers((ALuint)1, &this->buffer);

			ALsizei size, freq;
			ALenum format;
			ALvoid* data;
			ALboolean loop = AL_TRUE;

			//Material from: ThinMatrix
			alutLoadWAVFile((ALbyte*)PROJECT_DIR "/Audios/boune.wav", &format, &data, &size, &freq, &loop);
			alBufferData(this->buffer, format, data, size, freq);
			alSourcei(this->source, AL_BUFFER, this->buffer);

			if (format == AL_FORMAT_STEREO16 || format == AL_FORMAT_STEREO8)
				puts("TYPE::STEREO");
			else if (format == AL_FORMAT_MONO16 || format == AL_FORMAT_MONO8)
				puts("TYPE::MONO");

			alSourcePlay(this->source);

			// cleanup context
			//alDeleteSources(1, &source);
			//alDeleteBuffers(1, &buffer);
			//device = alcGetContextsDevice(context);
			//alcMakeContextCurrent(NULL);
			//alcDestroyContext(context);
			//alcCloseDevice(device);
		}
	}
	else
		throw std::runtime_error("Could not initialize GLAD!");


	//*********************************************************************
	//
	// * set the light parameters
	//
	//**********************************************************************
	GLfloat lightPosition1[]	= {0,1,1,0}; // {50, 200.0, 50, 1.0};
	GLfloat lightPosition2[]	= {1, 0, 0, 0};
	GLfloat lightPosition3[]	= {0, -1, 0, 0};
	GLfloat yellowLight[]		= {0.5f, 0.5f, .1f, 1.0};
	GLfloat whiteLight[]		= {1.0f, 1.0f, 1.0f, 1.0};
	GLfloat blueLight[]			= {.1f,.1f,.3f,1.0};
	GLfloat grayLight[]			= {.3f, .3f, .3f, 1.0};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition1);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteLight);
	glLightfv(GL_LIGHT0, GL_AMBIENT, grayLight);

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, blueLight);

	// set linstener position 
	if(selectedCube >= 0)
		alListener3f(AL_POSITION, 
			m_pTrack->points[selectedCube].pos.x,
			m_pTrack->points[selectedCube].pos.y,
			m_pTrack->points[selectedCube].pos.z);
	else
		alListener3f(AL_POSITION, 
			this->source_pos.x, 
			this->source_pos.y,
			this->source_pos.z);


	//*********************************************************************
	// now draw the ground plane
	//*********************************************************************
	// set to opengl fixed pipeline(use opengl 1.x draw function)
	//glUseProgram(0);
	//setupFloor();
	//glDisable(GL_LIGHTING);
	//drawFloor(200,10);


	//*********************************************************************
	// now draw the object and we need to do it twice
	// once for real, and then once for shadows
	//*********************************************************************
	glEnable(GL_LIGHTING);
	setupObjects();

	drawStuff();

	// this time drawing is for shadows (except for top view)
	if (!tw->topCam->value()) {
		setupShadows();
		drawStuff(true);
		unsetupShadows();
	}
	
	setUBO();
	glBindBufferRange(
		GL_UNIFORM_BUFFER, /*binding point*/0, this->commom_matrices->ubo, 0, this->commom_matrices->size);
	
	WaterFrameBuffers* fbos = new WaterFrameBuffers();
	fbos->bindReflectionFrameBuffer();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	this->drawSence();
	
	fbos->unbindCurrentFrameBuffer();

	this->drawSence();

	//###############################################################
	// plane
	// 
	//bind shader
	
	//use gui
	//this->guiShader->Use();
	//GLfloat positions[] = { -1,1,-1,-1,1,1,1,-1 };
	//VAO* gui = new VAO();
	//glGenVertexArrays(1, &gui->vao);
	//glGenBuffers(1, &gui->vbo[0]);
	//glBindVertexArray(gui->vao);
	//glBindBuffer(GL_ARRAY_BUFFER, gui->vbo[0]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(positions), &positions, GL_STATIC_DRAW);
	//
	//glBindTexture(GL_TEXTURE_2D, fbos->getReflectionTexture());
	//glUniform1i(glGetUniformLocation(fbos->getReflectionTexture(), "u_texture"), 0);
	//
	//glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//use water surface
	this->shader->Use();
	
	glm::mat4 model_matrix = glm::mat4();
	model_matrix = glm::translate(model_matrix, this->source_pos);
	
	this->shader->setMat4("u_model", model_matrix);

	glActiveTexture(GL_TEXTURE0 + 0);
	glBindTexture(GL_TEXTURE_2D, fbos->getReflectionTexture());
	glUniform1i(glGetUniformLocation(this->shader->Program,"u_texture"), 0);

	this->textureSky->bind(1);
	glUniform1i(glGetUniformLocation(this->shader->Program, "skybox"), 1);

	this->textureTile->bind(2);
	glUniform1i(glGetUniformLocation(this->shader->Program, "dudv"), 2);
	
	glBindVertexArray(plane->vao);
	
	glDrawElements(GL_TRIANGLES, this->plane->element_amount, GL_UNSIGNED_INT, 0);
	

	// Unbind VAO
	glBindVertexArray(0);
	//###############################################################
	fbos->cleanUp();
	delete fbos;
	//unbind shader(switch to fixed pipeline)

	glDeleteVertexArrays(1, &plane->vao);
	glDeleteBuffers(3, plane->vbo);
	glDeleteBuffers(1, &plane->ebo);
	delete plane;

	glDeleteVertexArrays(1, &pool->vao);
	glDeleteBuffers(1, pool->vbo);
	delete pool;

	glDeleteVertexArrays(1, &skybox->vao);
	glDeleteBuffers(1, skybox->vbo);
	delete skybox;

	glDeleteBuffers(1, &commom_matrices->ubo);

	glUseProgram(0);
}

//************************************************************************
//
// * This sets up both the Projection and the ModelView matrices
//   HOWEVER: it doesn't clear the projection first (the caller handles
//   that) - its important for picking
//========================================================================
void TrainView::
setProjection()
//========================================================================
{
	// Compute the aspect ratio (we'll need it)
	float aspect = static_cast<float>(w()) / static_cast<float>(h());

	// Check whether we use the world camp
	if (tw->worldCam->value())
		arcball.setProjection(false);
	// Or we use the top cam
	else if (tw->topCam->value()) {
		float wi, he;
		if (aspect >= 1) {
			wi = 110;
			he = wi / aspect;
		} 
		else {
			he = 110;
			wi = he * aspect;
		}

		// Set up the top camera drop mode to be orthogonal and set
		// up proper projection matrix
		glMatrixMode(GL_PROJECTION);
		glOrtho(-wi, wi, -he, he, 200, -200);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glRotatef(-90,1,0,0);
	} 
	// Or do the train view or other view here
	//####################################################################
	// TODO: 
	// put code for train view projection here!	
	//####################################################################
	else {
#ifdef EXAMPLE_SOLUTION
		trainCamView(this,aspect);
#endif
	}
}

//************************************************************************
//
// * this draws all of the stuff in the world
//
//	NOTE: if you're drawing shadows, DO NOT set colors (otherwise, you get 
//       colored shadows). this gets called twice per draw 
//       -- once for the objects, once for the shadows
//########################################################################
// TODO: 
// if you have other objects in the world, make sure to draw them
//########################################################################
//========================================================================
void TrainView::drawStuff(bool doingShadows)
{
	// Draw the control points
	// don't draw the control points if you're driving 
	// (otherwise you get sea-sick as you drive through them)
	/*
	if (!tw->trainCam->value()) {
		for(size_t i=0; i<m_pTrack->points.size(); ++i) {
			if (!doingShadows) {
				if ( ((int) i) != selectedCube)
					glColor3ub(240, 60, 60);
				else
					glColor3ub(240, 240, 30);
			}
			m_pTrack->points[i].draw();
		}
	}*/
	// draw the track
	//####################################################################
	// TODO: 
	// call your own track drawing code
	//####################################################################

#ifdef EXAMPLE_SOLUTION
	drawTrack(this, doingShadows);
#endif

	// draw the train
	//####################################################################
	// TODO: 
	//	call your own train drawing code
	//####################################################################
#ifdef EXAMPLE_SOLUTION
	// don't draw the train if you're looking out the front window
	if (!tw->trainCam->value())
		drawTrain(this, doingShadows);
#endif
}

// 
//************************************************************************
//
// * this tries to see which control point is under the mouse
//	  (for when the mouse is clicked)
//		it uses OpenGL picking - which is always a trick
//########################################################################
// TODO: 
//		if you want to pick things other than control points, or you
//		changed how control points are drawn, you might need to change this
//########################################################################
//========================================================================
void TrainView::
doPick()
//========================================================================
{
	// since we'll need to do some GL stuff so we make this window as 
	// active window
	make_current();		

	// where is the mouse?
	int mx = Fl::event_x(); 
	int my = Fl::event_y();

	// get the viewport - most reliable way to turn mouse coords into GL coords
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Set up the pick matrix on the stack - remember, FlTk is
	// upside down!
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity ();
	gluPickMatrix((double)mx, (double)(viewport[3]-my), 
						5, 5, viewport);

	// now set up the projection
	setProjection();

	// now draw the objects - but really only see what we hit
	GLuint buf[100];
	glSelectBuffer(100,buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	// draw the cubes, loading the names as we go
	for(size_t i=0; i<m_pTrack->points.size(); ++i) {
		glLoadName((GLuint) (i+1));
		m_pTrack->points[i].draw();
	}

	// go back to drawing mode, and see how picking did
	int hits = glRenderMode(GL_RENDER);
	if (hits) {
		// warning; this just grabs the first object hit - if there
		// are multiple objects, you really want to pick the closest
		// one - see the OpenGL manual 
		// remember: we load names that are one more than the index
		selectedCube = buf[3]-1;
	} else // nothing hit, nothing selected
		selectedCube = -1;

	printf("Selected Cube %d\n",selectedCube);
}

void TrainView::setUBO()
{
	float wdt = this->pixel_w();
	float hgt = this->pixel_h();

	glm::mat4 view_matrix;
	glGetFloatv(GL_MODELVIEW_MATRIX, &view_matrix[0][0]);
	//HMatrix view_matrix; 
	//this->arcball.getMatrix(view_matrix);

	glm::mat4 projection_matrix;
	glGetFloatv(GL_PROJECTION_MATRIX, &projection_matrix[0][0]);
	//projection_matrix = glm::perspective(glm::radians(this->arcball.getFoV()), (GLfloat)wdt / (GLfloat)hgt, 0.01f, 1000.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, this->commom_matrices->ubo);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &projection_matrix[0][0]);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &view_matrix[0][0]);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}