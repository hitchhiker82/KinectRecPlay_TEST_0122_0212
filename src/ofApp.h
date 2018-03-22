#pragma once

#include "ofMain.h"
#include "ofxKinectForWindows2.h"
#include "ofxVideoRecorder.h"
#include "ofxOpenCv.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();
		void exit();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

		ofxKFW2::Device kinect;
		ICoordinateMapper* coordinateMapper;

		ofPixels bodyIndexImg, foregroundImg;
		vector<ofVec2f> colorCoords;
		vector<ofVec3f> cameraCoords;
		//vector<vector<ofVec3f>> cameraCoordss;
		int numBodiesTracked;
		bool bHaveAllStreams;


		int numBodiesTrackedP;
		bool isSubLoaded;
		float tStamp;

		/////
		ofxVideoRecorder    vidRecorderC;
		ofxVideoRecorder    vidRecorderDH;
		bool bRecording;
		string fileNameC;
		string fileNameDH;
		string fileExt;

		ofFbo recordFbo;
		ofPixels recordPixels;

		ofVideoPlayer mvC01;
		ofVideoPlayer mvDH01;
		bool frameByframe;

		ofMesh mesh;
		vector<ofFloatColor> colors;
		vector<ofVec3f> vertexs;

		ofCamera cam1;

		ofPixels toSaveDH;
		vector<ofShortPixels> loadedDeps;

		string dStamp;
		int rCounter;

		vector<ofVideoPlayer> cVideos;
		vector<ofVideoPlayer> dVideos;
		vector<ofVideoPlayer> cVideosSub;
		vector<ofVideoPlayer> dVideosSub;

		vector<ofVideoPlayer*> cVideosP;
		vector<ofVideoPlayer*> dVideosP;

		ofDirectory dir;

		ofSerial serial1;
		//ofSerial serial2;

		vector<unsigned char> lightsBrightness1;
		vector<unsigned char> lightsBrightness2;
		vector<ofVec3f> lightsPosition;
		vector<float> shortestLength;

		bool dInfo;

};