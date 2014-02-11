#include "testApp.h"

using namespace ofxCv;

template<class T>
const T& constrain(const T& x, const T& a, const T& b) {
    if(x < a) {
        return a;
    }
    else if(b < x) {
        return b;
    }
    else
        return x;
}

void testApp::setup() {
	ofSetVerticalSync(true);
	cam.initGrabber(ofGetWidth(), ofGetHeight());
	camTracker.setup();
    
	//imgTracker.setup();
	
	//img.loadImage("face.jpg");
	//imgTracker.update(toCv(img));
    frame = 0;
    total = 1;
    readings = new float[60];
    for (int i = 0; i < 60; i++)
        readings[i] = 1.0;
    //memset(readings, 0, sizeof(float) * 60);
    bufferSetup = false;
    soundScale = 1;
    
    fft.setup(48000,OF_FFT_WINDOW_HAMMING);
    fft.setUseNormalization(false);
}

void testApp::update() {
	cam.update();
	if(cam.isFrameNew()) {
		camTracker.update(toCv(cam));
	}
   

    fft.update();
    vector<float>& freqs = fft.getBins();
    int bin;
    auto maxInd = std::max_element(freqs.begin(), freqs.end());
    if (freqs.size() > 0) {
        bin = std::distance(freqs.begin(),maxInd);
        cout << "val: " + ofToString(*maxInd) + "\n";
        cout << "bin: " + ofToString(bin) + "\n";
    }
    
    if (*maxInd > 0.01){
       if (bin < 310)
           soundScale += 0.01;
        else if (bin >= 310)
            soundScale -= 0.01;
    }
    else if (soundScale < 1)
        soundScale += 0.01;
    else
        soundScale -= 0.01;
    
}

void testApp::draw() {
    ofImage myFace;
    myFace.setFromPixels(cam.getPixels(), ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR, true);
	ofSetColor(255);
    myFace.draw(0,0);
	//cam.draw(0, 0);
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
    ofDrawBitmapString(ofToString(mouseX), 50, 20);
    ofDrawBitmapString(ofToString(mouseY), 90, 20);
	
	/*if(camTracker.getFound()) {
		camTracker.draw();
	}
	
	ofTranslate(0, 480);*/
	
	if(camTracker.getFound()) {
        ofMesh faceMesh = camTracker.getImageMesh();
		ofxDelaunay delaunay;
		
		// add main face points
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			delaunay.addPoint(faceMesh.getVertex(i));
		}
		
		// add boundary face points
		float scaleFactor = 1.6;
        ofPolyline outline = camTracker.getImageFeature(ofxFaceTracker::FACE_OUTLINE);
        ofVec2f position = camTracker.getPosition();
		for(int i = 0; i < outline.size(); i++) {
			ofVec2f point((outline[i] - position) * scaleFactor + position);
			delaunay.addPoint(point);
		}
        
		// add the image corners
		int w = cam.getWidth(), h = cam.getHeight();
        //int w = 640;
        //int h = 480;
		delaunay.addPoint(ofVec2f(0, 0));
		delaunay.addPoint(ofVec2f(w, 0));
		delaunay.addPoint(ofVec2f(w, h));
		delaunay.addPoint(ofVec2f(0, h));
        
		delaunay.triangulate();
        ofMesh triangulated = delaunay.triangleMesh;
		triangulated.drawWireframe();
		
		// find mapping between triangulated mesh and original
		vector<int> delaunayToFinal(triangulated.getNumVertices(), -1);
		vector<int> finalToDelaunay;
		int reindexed = 0;
		for(int i = 0; i < faceMesh.getNumVertices(); i++) {
			float minDistance = 0;
			int best = 0;
			for(int j = 0; j < triangulated.getNumVertices(); j++) {
				float distance = triangulated.getVertex(j).distance(faceMesh.getVertex(i));
				if(j == 0 || distance < minDistance) {
					minDistance = distance;
					best = j;
				}
			}
			delaunayToFinal[best] = reindexed++;
			finalToDelaunay.push_back(best);
		}
		for(int i = 0; i < delaunayToFinal.size(); i++) {
			if(delaunayToFinal[i] == -1) {
				delaunayToFinal[i] = reindexed++;
				finalToDelaunay.push_back(i);
			}
		}
        
		// construct new mesh that has tex coords, vertices, etc.
		ofMesh finalMesh;
		finalMesh.setMode(OF_PRIMITIVE_TRIANGLES);	
		for(int i = 0; i < delaunayToFinal.size(); i++) {
			int index = finalToDelaunay[i];
			finalMesh.addVertex(triangulated.getVertex(index));
			finalMesh.addTexCoord(triangulated.getVertex(index));
		}
		for(int i = 0; i < triangulated.getNumIndices(); i++) {
			finalMesh.addIndex(delaunayToFinal[triangulated.getIndex(i)]);
		}
        
		
        
		// modify mesh
        soundScale = constrain(soundScale,0.25f,1.65f);
        cout<< "scale: " + ofToString(soundScale) + "\n";
		if(camTracker.getFound()) {;
			ofVec2f camPosition = camTracker.getPosition();
			float camScale = camTracker.getScale();
			ofMesh reference = camTracker.getImageMesh();
			for(int i = 0; i < reference.getNumVertices(); i++) {
				ofVec2f point = reference.getVertices()[i];
                point -= camPosition;
                point *= soundScale;
                point += camPosition;
                finalMesh.getVertices()[i] = point;
			}
		}
		 
        myFace.bind();
		finalMesh.drawFaces();
        myFace.unbind();
	}
    
}

float testApp::remap(float value, float from1, float to1, float from2, float to2) {
    
    return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
    
}

void testApp::plot(vector<float>& buffer, float scale) {
	ofNoFill();
	int n = MIN(1024, buffer.size());
	ofRect(0, 0, n, scale);
	ofPushMatrix();
	ofTranslate(0, scale);
	ofScale(1, -scale);
	ofBeginShape();
	for (int i = 0; i < n; i++) {
		ofVertex(i, buffer[i]);
	}
	ofEndShape();
	ofPopMatrix();
}



