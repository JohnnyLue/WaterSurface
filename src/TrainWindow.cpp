/************************************************************************
     File:        TrainWindow.H

     Author:     
                  Michael Gleicher, gleicher@cs.wisc.edu

     Modifier
                  Yu-Chi Lai, yu-chi@cs.wisc.edu
     
     Comment:     
						this class defines the window in which the project 
						runs - its the outer windows that contain all of 
						the widgets, including the "TrainView" which has the 
						actual OpenGL window in which the train is drawn

						You might want to modify this class to add new widgets
						for controlling	your train

						This takes care of lots of things - including installing 
						itself into the FlTk "idle" loop so that we get periodic 
						updates (if we're running the train).


     Platform:    Visio Studio.Net 2003/2005

*************************************************************************/

#include <FL/fl.h>
#include <FL/Fl_Box.h>

// for using the real time clock
#include <time.h>

#include "TrainWindow.H"
#include "TrainView.H"
#include "CallBacks.H"



//************************************************************************
//
// * Constructor
//========================================================================
TrainWindow::
TrainWindow(const int x, const int y) 
	: Fl_Double_Window(x,y,800,600,"Train and Roller Coaster")
//========================================================================
{
	// make all of the widgets
	begin();	// add to this widget
	{
		int pty=5;			// where the last widgets were drawn

		trainView = new TrainView(5,5,590,590);
		trainView->tw = this;
		trainView->m_pTrack = &m_Track;
		this->resizable(trainView);

		// to make resizing work better, put all the widgets in a group
		widgets = new Fl_Group(600,5,190,590);
		widgets->begin();

		runButton = new Fl_Button(670,pty,60,20,"Run");
		togglify(runButton,1);
		

		pty += 30;

		// camera buttons - in a radio button group
		Fl_Group* camGroup = new Fl_Group(600,pty,195,20);
		camGroup->begin();
		worldCam = new Fl_Button(605, pty, 60, 20, "World");
        worldCam->type(FL_RADIO_BUTTON);		// radio button
        worldCam->value(1);			// turned on
        worldCam->selection_color((Fl_Color)3); // yellow when pressed
		worldCam->callback((Fl_Callback*)damageCB,this);
		reflectCam = new Fl_Button(670, pty, 60, 20, "reflect");
		reflectCam->type(FL_RADIO_BUTTON);
		reflectCam->value(0);
		reflectCam->selection_color((Fl_Color)3);
		reflectCam->callback((Fl_Callback*)damageCB,this);
		refractCam = new Fl_Button(735, pty, 60, 20, "refract");
        refractCam->type(FL_RADIO_BUTTON);
        refractCam->value(0);
        refractCam->selection_color((Fl_Color)3);
		refractCam->callback((Fl_Callback*)damageCB,this);
		camGroup->end();
		pty += 30;

		// add and delete points
		Fl_Button* ap = new Fl_Button(605, pty, 80, 20, "Add Cube");
		ap->callback((Fl_Callback*)addPointCB, this);
		Fl_Button* dp = new Fl_Button(690, pty, 80, 20, "Delete Cube");
		dp->callback((Fl_Callback*)deletePointCB, this);

		pty += 30;
		// reset the points
		resetButton = new Fl_Button(735, pty, 60, 20, "Reset");
		resetButton->callback((Fl_Callback*)resetCB, this);
		Fl_Button* loadb = new Fl_Button(605, pty, 60, 20, "Load");
		loadb->callback((Fl_Callback*)loadCB, this);
		Fl_Button* saveb = new Fl_Button(670, pty, 60, 20, "Save");
		saveb->callback((Fl_Callback*)saveCB, this);

		pty += 30;
		// roll the points
		Fl_Button* rx = new Fl_Button(605, pty, 30, 20, "R+X");
		rx->callback((Fl_Callback*)rpxCB, this);
		Fl_Button* rxp = new Fl_Button(635, pty, 30, 20, "R-X");
		rxp->callback((Fl_Callback*)rmxCB, this);
		Fl_Button* rz = new Fl_Button(670, pty, 30, 20, "R+Z");
		rz->callback((Fl_Callback*)rpzCB, this);
		Fl_Button* rzp = new Fl_Button(700, pty, 30, 20, "R-Z");
		rzp->callback((Fl_Callback*)rmzCB, this);

		pty += 30;

		// camera buttons - in a radio button group
		Fl_Group* waveGroup = new Fl_Group(600,pty,195,20);
		waveGroup->begin();
		sineWave = new Fl_Button(605, pty, 60, 20, "sin wave");
        sineWave->type(FL_RADIO_BUTTON);		// radio button
        sineWave->value(0);			// turned on
        sineWave->selection_color((Fl_Color)3); // yellow when pressed
		sineWave->callback((Fl_Callback*)damageWave,this);
		rippleWave = new Fl_Button(670, pty, 60, 20, "ripple");
		rippleWave->type(FL_RADIO_BUTTON);
		rippleWave->value(1);
		rippleWave->selection_color((Fl_Color)3);
		rippleWave->callback((Fl_Callback*)damageWave,this);
		waveGroup->end();

		pty += 30;
		///////////different
		// 
		// 
		// browser to select spline types
		// TODO: make sure these choices are the same as what the code supports
		waveBrowser = new Fl_Browser(605,pty,120,75,"Mode");
		waveBrowser->type(2);		// select
		waveBrowser->callback((Fl_Callback*)damageCB,this);
		waveBrowser->add("auto generate");
		waveBrowser->add("reactive");
		waveBrowser->add("both");
		waveBrowser->add("no wave");
		waveBrowser->select(1);

		pty += 120;
		speed = new Fl_Value_Slider(630, pty, 140, 20, "wave speed");
		speed->range(1, 5);
		speed->value(2);
		speed->align(FL_ALIGN_TOP);
		speed->type(FL_HORIZONTAL);

		pty += 45;
		waveHei = new Fl_Value_Slider(630, pty, 140, 20, "wave height");
		waveHei->range(-1.5, 1.5);
		waveHei->value(0.8);
		waveHei->align(FL_ALIGN_TOP);
		waveHei->type(FL_HORIZONTAL);

		pty -= 90;//go back
		//################################


		// TODO: add widgets for all of your fancier features here
#ifdef EXAMPLE_SOLUTION
		makeExampleWidgets(this,pty);
#endif

		// we need to make a little phantom widget to have things resize correctly
		Fl_Box* resizebox = new Fl_Box(600,595,200,5);
		widgets->resizable(resizebox);

		widgets->end();
	}
	end();	// done adding to this widget

	// set up callback on idle
	Fl::add_idle((void (*)(void*))runButtonCB,this);
}

//************************************************************************
//
// * handy utility to make a button into a toggle
//========================================================================
void TrainWindow::
togglify(Fl_Button* b, int val)
//========================================================================
{
	b->type(FL_TOGGLE_BUTTON);		// toggle
	b->value(val);		// turned off
	b->selection_color((Fl_Color)3); // yellow when pressed	
	b->callback((Fl_Callback*)damageCB,this);
}

//************************************************************************
//
// *
//========================================================================
void TrainWindow::
damageMe()
//========================================================================
{
	if (trainView->selectedCube >= ((int)m_Track.points.size()))
		trainView->selectedCube = 0;
	trainView->damage(1);
}

//************************************************************************
//
// * This will get called (approximately) 30 times per second
//   if the run button is pressed
//========================================================================
void TrainWindow::
advanceTrain(float dir)
//========================================================================
{
	trainView->time+=0.1;
	trainView->updateHeightMap();
	//#####################################################################
	// TODO: make this work for your train
	//#####################################################################
#ifdef EXAMPLE_SOLUTION
	// note - we give a little bit more example code here than normal,
	// so you can see how this works

	if (arcLength->value()) {
		float vel = ew.physics->value() ? physicsSpeed(this) : dir * (float)speed->value();
		world.trainU += arclenVtoV(world.trainU, vel, this);
	} else {
		world.trainU +=  dir * ((float)speed->value() * .1f);
	}

	float nct = static_cast<float>(world.points.size());
	if (world.trainU > nct) world.trainU -= nct;
	if (world.trainU < 0) world.trainU += nct;
#endif
}