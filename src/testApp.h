#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxDelaunay.h"
#include "ofxEasyFft.h"
#include "ofxFft.h"
#include "ofxFftBasic.h"
#include <algorithm>

class testApp : public ofBaseApp {
public:
	void setup();
	void update();
	void draw();
    float remap(float value, float from1, float to1, float from2, float to2);
    
    void plot(vector<float>& buffer, float scale);
	
	ofxEasyFft fft;
	ofImage img;
	ofVideoGrabber cam;
	ofxFaceTracker camTracker, imgTracker;
    string msg;
    float soundScale;
    float *readings;
    float total;
    int frame;
    int numReadings;
    bool bufferSetup;
};
