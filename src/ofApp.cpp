#include "ofApp.h"

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424
#define DEPTH_SIZE DEPTH_WIDTH * DEPTH_HEIGHT

#define COLOR_WIDTH 1920
#define COLOR_HEIGHT 1080

//int previewWidth = 640;
//int previewHeight = 480;

//--------------------------------------------------------------
void ofApp::setup(){

	ofBackground(0);

	serial1.enumerateDevices();

	serial1.setup("com4", 9600);
	//serial2.setup("com3", 9600);
	lightsBrightness1.assign(6, 0);
	//lightsBrightness2.assign(1, 0);

	lightsPosition.push_back(ofVec3f(1.30, -0.61, 2.36));
	lightsPosition.push_back(ofVec3f(0.48, -0.52, 2.49));
	lightsPosition.push_back(ofVec3f(-0.087, -0.78, 1.77));
	lightsPosition.push_back(ofVec3f(1.05, 0.09, 3.60));
	lightsPosition.push_back(ofVec3f(-0.12, -0.01, 3.57));
	lightsPosition.push_back(ofVec3f(-0.75, -0.57, 2.52));

	//lightsPosition.assign(points);
	shortestLength.assign(7, 10);

	kinect.open();
	kinect.initDepthSource();
	kinect.initColorSource();
	kinect.initInfraredSource();
	kinect.initBodySource();
	kinect.initBodyIndexSource();

	ofSetWindowShape(DEPTH_WIDTH * 2, DEPTH_HEIGHT*2);

	if (kinect.getSensor()->get_CoordinateMapper(&coordinateMapper) < 0) {
		ofLogError() << "Could not acquire CoordinateMapper!";
	}

	numBodiesTracked = 0;
	bHaveAllStreams = false;

	bodyIndexImg.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, OF_PIXELS_RGB);
	foregroundImg.allocate(DEPTH_WIDTH, DEPTH_HEIGHT, OF_PIXELS_RGB);

	colorCoords.resize(DEPTH_SIZE);

	toSaveDH.allocate(DEPTH_WIDTH*2, DEPTH_HEIGHT, OF_PIXELS_GRAY);

	cameraCoords.resize(DEPTH_SIZE);
	loadedDeps.assign(5, ofShortPixels());

	for (int i = 0; i < 5; i++) {
		loadedDeps[i].allocate(DEPTH_WIDTH, DEPTH_HEIGHT, OF_IMAGE_GRAYSCALE);	
		//vector<ofVec3f> cameraCoords;
		//cameraCoords.resize(DEPTH_SIZE);
		//cameraCoordss.push_back(cameraCoords);
	}

	/////////////////////////////////////////////

	//colors.assign(DEPTH_SIZE * (5+1), ofFloatColor());
	//vertexs.assign(DEPTH_SIZE * (5+1), ofVec3f());
	mesh.setMode(OF_PRIMITIVE_POINTS);
	ofSetFrameRate(60);
	ofSetLogLevel(OF_LOG_VERBOSE);

	
#ifdef TARGET_WIN32
	vidRecorderC.setFfmpegLocation("C:\\ffmpeg\\bin\\ffmpeg"); // use this is you have ffmpeg installed in your data folder
	vidRecorderDH.setFfmpegLocation("C:\\ffmpeg\\bin\\ffmpeg"); // use this is you have ffmpeg installed in your data folder
#endif
	fileNameC = "color";
	fileNameDH = "depth";
	fileExt = ".mp4";
					 
	vidRecorderC.setVideoCodec("mpeg4");
	vidRecorderC.setVideoBitrate("5000k");
	vidRecorderC.setPixelFormat("rgb24");

	vidRecorderDH.setVideoCodec("libx264");
	vidRecorderDH.setVideoBitrate("30000k"); //800k 20000k
	vidRecorderDH.setPixelFormat("gray");

	
	bRecording = false;
	ofEnableAlphaBlending();

	cVideos.assign(5, ofVideoPlayer());
	dVideos.assign(5, ofVideoPlayer());
	cVideosSub.assign(5, ofVideoPlayer());
	dVideosSub.assign(5, ofVideoPlayer());

	cVideosP.assign(5, &ofVideoPlayer());
	dVideosP.assign(5, &ofVideoPlayer());



	/////// load video///
	dir.listDir("video");
	dir.sort();
	for (int i = 0; i < cVideos.size(); i++) {

		string fn; // = "/video/" + dir[ofRandom(dir.size() / 2)].getFileName();
		bool isOverlaped = true;

		while (isOverlaped) {
			isOverlaped = false;
			fn = "/video/" + dir[ofRandom(dir.size() / 2)].getFileName();
			for (int j = 0; j < cVideos.size(); j++) {
				if (cVideos[j].getMoviePath() == fn) {
					isOverlaped = true;
				}
			}
		}

		cVideos[i].load(fn);

		fn.replace(7, 5, "depth");
		dVideos[i].load(fn);

		cVideos[i].setLoopState(OF_LOOP_NONE);
		dVideos[i].setLoopState(OF_LOOP_NONE);

		cVideos[i].play();
		dVideos[i].play();

		cVideosP[i] = &cVideos[i];
		dVideosP[i] = &dVideos[i];
	}

	for (int i = 0; i < cVideosSub.size(); i++) {

		string fn; // = "/video/" + dir[ofRandom(dir.size() / 2)].getFileName();
		bool isOverlaped = true;

		while (isOverlaped) {
			isOverlaped = false;
			fn = "/video/" + dir[ofRandom(dir.size() / 2)].getFileName();
			for (int j = 0; j < cVideos.size(); j++) {
				if (cVideos[j].getMoviePath() == fn || cVideosSub[j].getMoviePath()==fn) {
					isOverlaped = true;
				}
			}
		}

		cVideosSub[i].load(fn);
		fn.replace(7, 5, "depth");
		dVideosSub[i].load(fn);

		cVideosSub[i].setLoopState(OF_LOOP_NONE);
		dVideosSub[i].setLoopState(OF_LOOP_NONE);
	}

	isSubLoaded = true;

	//mvC01.load("testMovieC.mp4");
	//mvC01.setLoopState(OF_LOOP_NORMAL);
	//mvC01.play();


	//mvDH01.load("testMovieDH.mp4");
	//mvDH01.setLoopState(OF_LOOP_NORMAL);
	//mvDH01.play();
	/////////////////////////////////////

	ofSetVerticalSync(true);

	ofEnableDepthTest();
	glEnable(GL_POINT_SMOOTH); // use circular points instead of square points
	glPointSize(3); // make the points bigger

	dStamp = ofGetTimestampString("%d%H%M%S");
	rCounter = 0;

	//cam1.setPosition(0, -5000, 935);
	//cam1.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, 1, 1));

	dInfo = false;

}

void ofApp::exit() {
	vidRecorderC.close();
	vidRecorderDH.close();

	for (int i = 0; i < cVideos.size(); i++) {
		cVideos[i].close();
		dVideos[i].close();
		cVideosSub[i].close();
		dVideosSub[i].close();
	}
	//mvC01.close();
	//mvDH01.close();

}

//--------------------------------------------------------------
void ofApp::update(){

	kinect.update();


	for (int i = 0; i < cVideosP.size(); i++) {

		cVideosP[i]->update();
		dVideosP[i]->update();
		cVideosP[i]->setPosition(dVideosP[i]->getPosition());

		if (cVideosP[i]->getIsMovieDone() || dVideosP[i]->getIsMovieDone()) {

			

				cVideosP[i]->stop();
				dVideosP[i]->stop();
				
				if (cVideosP[i] == &cVideos[i]) {
					cVideosP[i] = &cVideosSub[i];
					dVideosP[i] = &dVideosSub[i];
				}
				else if (cVideosP[i] == &cVideosSub[i]) {
					cVideosP[i] = &cVideos[i];
					dVideosP[i] = &dVideos[i];
				}
				

				cVideosP[i]->play();
				dVideosP[i]->play();

		}
	}


	auto& depthPix = kinect.getDepthSource()->getPixels();
	auto& bodyIndexPix = kinect.getBodyIndexSource()->getPixels();
	auto& colorPix = kinect.getColorSource()->getPixels();


	if (!depthPix.size() || !bodyIndexPix.size() || !colorPix.size()) {
		bHaveAllStreams = false;
		return;
	}
	else {
		bHaveAllStreams = true;
	}



	shortestLength.assign(7, 10);

	numBodiesTracked = 0;
	auto& bodies = kinect.getBodySource()->getBodies();
	for (auto& body : bodies) {
		
		if (body.tracked) {
			
			auto& joints = body.joints;

			for(auto& joint :  joints)
			{
				auto& cPoint = joint.second.getPosition();
				for (int i = 0; i < lightsPosition.size(); i++) {

					float cDist = lightsPosition[i].distance(cPoint);
					if (shortestLength[i] > cDist)
						shortestLength[i] = cDist;
				}
			}
			cout << endl;
			numBodiesTracked++;
		}
	}

	if (numBodiesTrackedP > 0 && numBodiesTracked == 0) {
		isSubLoaded = false;
		tStamp = ofGetElapsedTimef();
	}

	numBodiesTrackedP = numBodiesTracked;

	if (numBodiesTracked == 0 && !isSubLoaded && ofGetElapsedTimef() - tStamp > 3) {

		for (int i = 0; i < cVideosSub.size(); i++) {

			string fn;
			bool isOverlaped = true;

			while (isOverlaped) {
				isOverlaped = false;
				fn = "/video/" + dir[ofRandom(dir.size() / 2)].getFileName();
				for (int j = 0; j < cVideos.size(); j++) {
					if (cVideos[j].getMoviePath() == fn || cVideosSub[j].getMoviePath() == fn) {
						isOverlaped = true;
					}
				}
			}
			//////////////////////////////////////////////////////////////

			if (cVideosP[i] == &cVideos[i]) {
				cVideosSub[i].close();
				dVideosSub[i].close();
				cVideosSub[i].load(fn);
				fn.replace(7, 5, "depth");
				dVideosSub[i].load(fn);
				cVideosSub[i].setLoopState(OF_LOOP_NONE);
				dVideosSub[i].setLoopState(OF_LOOP_NONE);
			}
			else if (cVideosP[i] == &cVideosSub[i]) {
				cVideos[i].close();
				dVideos[i].close();
				cVideos[i].load(fn);
				fn.replace(7, 5, "depth");
				dVideos[i].load(fn);
				cVideos[i].setLoopState(OF_LOOP_NONE);
				dVideos[i].setLoopState(OF_LOOP_NONE);
			}
		}
		isSubLoaded = true;
	}


	int tMax = lightsBrightness1.size() + lightsBrightness2.size();
	for (int i = 0; i < tMax; i++) {

		//int tBrightness = 255 - shortestLength[i] * 1000;
		int tBrightness = (500 - shortestLength[i] * 1000)/2;

		if(tBrightness < 0) {
			tBrightness = 0;
		}

		if (i < lightsBrightness1.size()) {
			lightsBrightness1[i] = tBrightness;
		}
		else {
			lightsBrightness2[i - lightsBrightness1.size()] = tBrightness;
		}
	}


	char msg1 = 'a';
	//char msg2 = 'a';

	if (serial1.available()) {
		msg1 = serial1.readByte();
		serial1.flush();
	}
	//if (serial2.available()) {
	//	msg2 = serial2.readByte();
	//	serial2.flush();
	//}

	if (msg1 == 'f') {
		serial1.writeBytes(&lightsBrightness1[0], 6);
	}

	//if (msg2 == 'f') {
	//	serial2.writeBytes(&lightsBrightness2[0], 1);
	//}


	// https://msdn.microsoft.com/en-us/library/windowspreview.kinect.coordinatemapper.mapdepthframetocolorspace.aspx
	// https://msdn.microsoft.com/en-us/library/dn785530.aspx
	coordinateMapper->MapDepthFrameToColorSpace(DEPTH_SIZE, (UINT16*)depthPix.getPixels(), DEPTH_SIZE, (ColorSpacePoint*)colorCoords.data());
	

	vector<unsigned char*> cursors;
	for (int mv = 0; mv < 5; mv++) {
		unsigned char* cursor = dVideosP[mv]->getPixels().getData();
		cursors.push_back(cursor);
	}


	if (numBodiesTracked > 0) {

		unsigned char* fCursor = foregroundImg.getData();
		unsigned char* iCursor = bodyIndexImg.getData();

		for (int y = 0; y < DEPTH_HEIGHT; y++) {
			for (int x = 0; x < DEPTH_WIDTH; x++) {
				int index = (y * DEPTH_WIDTH) + x;

				ofVec2f mappedCoord = colorCoords[index];

				mappedCoord.x = floor(mappedCoord.x);
				mappedCoord.y = floor(mappedCoord.y);



				// https://msdn.microsoft.com/en-us/library/windowspreview.kinect.bodyindexframe.aspx
				float val = bodyIndexPix[index];

				if (val < bodies.size()) {

					if (mappedCoord.x < 0 || mappedCoord.y < 0 || mappedCoord.x >= COLOR_WIDTH || mappedCoord.y >= COLOR_HEIGHT) {
						fCursor += 3;
						iCursor += 3;
						continue;
					}

					*fCursor = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4];
					*(fCursor+1) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4+1];
					*(fCursor+2) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4+2];

					*iCursor = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4];
					*(iCursor + 1) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 1];
					*(iCursor + 2) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 2];


					//foregroundImg[y*DEPTH_WIDTH * 3 + x * 3] = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4];
					//foregroundImg[y*DEPTH_WIDTH * 3 + x * 3 + 1] = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 1];
					//foregroundImg[y*DEPTH_WIDTH * 3 + x * 3 + 2] = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 2];

					////
					unsigned short dPix = depthPix[index];

					byte dH = ((dPix >> 8) & 0xff);
					byte dL = ((dPix >> 0) & 0xff);


					toSaveDH[(y * DEPTH_WIDTH * 2) + x] = dH;
					toSaveDH[(y * DEPTH_WIDTH * 2) + x + DEPTH_WIDTH] = dL;
					////

				}
				else {

					*fCursor = 0;
					*(fCursor+1) = 0;
					*(fCursor+2) = 0;

					toSaveDH[(y * DEPTH_WIDTH * 2) + x] = 0;
					toSaveDH[(y * DEPTH_WIDTH * 2) + x + DEPTH_WIDTH] = 0;

					if (mappedCoord.x < 0 || mappedCoord.y < 0 || mappedCoord.x >= COLOR_WIDTH || mappedCoord.y >= COLOR_HEIGHT) {
						fCursor += 3;
						iCursor += 3;
						continue;
					}
					*iCursor = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4];
					*(iCursor + 1) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 1];
					*(iCursor + 2) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 2];


				}

				fCursor += 3;
				iCursor += 3;
				


				for (int i = 0; i < 5; i++) {
					unsigned char* byteH = cursors[i] + (y * DEPTH_WIDTH * 3 * 2) + x * 3;
					unsigned char* byteL = cursors[i] + (y * DEPTH_WIDTH * 3 * 2) + (x + DEPTH_WIDTH) * 3;
					//
					unsigned short ts = 0;
					ts = *byteH;
					ts <<= 8;
					ts += *byteL;

					loadedDeps[i][index] = ts;
				}
			}
		}
		bRecording = true;
		if (bRecording && !vidRecorderC.isInitialized() && !vidRecorderDH.isInitialized()) {

			////delete file + is playing?
			string as = ofSystem("fsutil volume diskfree c:");

			for (int i = as.length() - 1; i >= 0; i--) {
				if (as[i] == ' ') {
					as.erase(0, i + 1);
					break;
				}
			}
			long long fSpace = ofToInt64(as);

			while (fSpace < 9000000000) {

				
	
				string fn;
				bool isOverlaped = true;

				while (isOverlaped) {

					isOverlaped = false;

					dir.open("video");
					dir.sort();

					fn = "/video/" + dir[ofRandom(dir.size() / 2 - 1)].getFileName();

					for (int j = 0; j < cVideos.size(); j++) {
						if (cVideos[j].getMoviePath() == fn || cVideosSub[j].getMoviePath() == fn) {
							isOverlaped = true;
						}
					}
				}

				ofFile::removeFile(fn);
				fn.replace(7, 5, "depth");
				ofFile::removeFile(fn);
			}



			rCounter++;
			vidRecorderC.setup("video\\" + fileNameC + "_" + dStamp + "_" + ofToString(rCounter,2,'0'), foregroundImg.getWidth(), foregroundImg.getHeight(), 30, 0, 3, true); // no audio
			vidRecorderDH.setup("video\\" + fileNameDH + "_" + dStamp + "_" + ofToString(rCounter,2, '0'), toSaveDH.getWidth(), toSaveDH.getHeight(), 30, 0, 1, true); // no audio
																										// Start recording
			vidRecorderC.start();
			vidRecorderDH.start();
		}
		if (bRecording && vidRecorderC.isInitialized() && vidRecorderDH.isInitialized()) {
			if (vidRecorderC.getNumVideoFramesRecorded() > 18000) {
				bRecording = false;
				vidRecorderC.close();
				vidRecorderDH.close();
			}
		}
	}
	else {

		unsigned char* iCursor = bodyIndexImg.getData();

		for (int y = 0; y < DEPTH_HEIGHT; y++) {
			for (int x = 0; x < DEPTH_WIDTH; x++) {
				int index = (y * DEPTH_WIDTH) + x;

				ofVec2f mappedCoord = colorCoords[index];

				mappedCoord.x = floor(mappedCoord.x);
				mappedCoord.y = floor(mappedCoord.y);

					if (mappedCoord.x < 0 || mappedCoord.y < 0 || mappedCoord.x >= COLOR_WIDTH || mappedCoord.y >= COLOR_HEIGHT) {
						iCursor += 3;
						continue;
					}

					*iCursor = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4];
					*(iCursor + 1) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 1];
					*(iCursor + 2) = colorPix[mappedCoord.y*COLOR_WIDTH * 4 + mappedCoord.x * 4 + 2];

					iCursor+=3;

				for (int i = 0; i < 5; i++) {
					unsigned char* byteH = cursors[i] + (y * DEPTH_WIDTH * 3 * 2) + x * 3;
					unsigned char* byteL = cursors[i] + (y * DEPTH_WIDTH * 3 * 2) + (x + DEPTH_WIDTH) * 3;
					//
					unsigned short ts = 0;
					ts = *byteH;
					ts <<= 8;
					ts += *byteL;

					loadedDeps[i][index] = ts;
				}
			}
		}

		bRecording = false;
		vidRecorderC.close();
		vidRecorderDH.close();
	}


	mesh.clear();

	
	colors.clear();
	vertexs.clear();

	/////////////////////////////////////////////////////////////////////
	//colors.assign(DEPTH_SIZE * (5 + 1), ofFloatColor(0,0,0));
	//vertexs.assign(DEPTH_SIZE * (5 + 1), ofVec3f(0,0,0));

	//ofFloatColor *cCursor = &colors[0];
	//ofVec3f *vCursor = &vertexs[0];

	for (int mv = 0; mv < 6; mv++) {
		//coordinateMapper->MapDepthFrameToCameraSpace(DEPTH_SIZE, (UINT16*)loadedDeps[mv].getPixels(), DEPTH_SIZE, (CameraSpacePoint*)cameraCoordss[mv].data());
		unsigned char* cursorC;
		if (mv == 5) {
			coordinateMapper->MapDepthFrameToCameraSpace(DEPTH_SIZE, (UINT16*)depthPix.getPixels(), DEPTH_SIZE, (CameraSpacePoint*)cameraCoords.data());
			cursorC = bodyIndexImg.getData();
		}
		else {
			coordinateMapper->MapDepthFrameToCameraSpace(DEPTH_SIZE, (UINT16*)loadedDeps[mv].getPixels(), DEPTH_SIZE, (CameraSpacePoint*)cameraCoords.data());
			cursorC = cVideosP[mv]->getPixels().getData();
		}

		ofVec3f* cursorD = cameraCoords.data();

		//if (cVideos[mv].isPlaying()) {
			for (int i = 0; i < DEPTH_WIDTH; i++) {
				for (int j = 0; j < DEPTH_HEIGHT; j++) {

					if (cursorD->z < 5000 && cursorD->z >0) {

						unsigned char* colorR = cursorC;
						unsigned char* colorG = cursorC + 1;
						unsigned char* colorB = cursorC + 2;

						colors.push_back(ofFloatColor(*colorR / 255., *colorG / 255., *colorB / 255.));
						vertexs.push_back(*cursorD);
						//*cCursor = ofFloatColor(*colorR / 255., *colorG / 255., *colorB / 255.);
						//*vCursor = *cursorD;
						
						//cCursor++;
						//vCursor++;
					}
					cursorC += 3;
					cursorD++;

				}
			}
	}


	mesh.addColors(colors);
	mesh.addVertices(vertexs);

//
	//--


	if (kinect.isFrameNew() && bRecording) {

		bool success1 = vidRecorderC.addFrame(foregroundImg);
		bool success2 = vidRecorderDH.addFrame(toSaveDH);
		if (!success1 || !success2) {
			ofLogWarning("This frame was not added!");
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw(){

	//ofShortImage aa(loadedDep);
	//aa.draw(0,0);

	if (dInfo) {

		stringstream ss;
		ss << "fps : " << ofGetFrameRate() << endl;
		ss << "Tracked bodies: " << numBodiesTracked;
		if (!bHaveAllStreams) ss << endl << "Not all streams detected!";

		ss << endl << cam1.getPosition() << endl << cam1.getLookAtDir();
		ofDrawBitmapStringHighlight(ss.str(), 60, 20);
	}
	//ofPushStyle();

	//ofSetColor(255, 255, 255);
	//
	//stringstream ss2;
	//ss2 << "video queue size: " << vidRecorderC.getVideoQueueSize() << endl
	//	<< "audio queue size: " << vidRecorderC.getAudioQueueSize() << endl
	//	<< "FPS: " << ofGetFrameRate() << endl
	//	<< (bRecording ? "pause" : "start") << " recording: r" << endl
	//	<< (bRecording ? "close current video file: c" : "") << endl;

	//ofSetColor(0, 0, 0, 100);
	//ofRect(0, 0, 260, 75);
	//ofSetColor(255, 255, 255);
	//ofDrawBitmapString(ss2.str(), 15, 15);



	//stringstream ss3;
	//ss3 << ofToString(shortestLength[6]) << ", " << lightsBrightness2[0] << endl;
	//ofDrawBitmapStringHighlight(ss3.str(), 200, 200);


	if (bRecording) {
		ofSetColor(255, 0, 0);
		ofCircle(ofGetWidth() - 20, 20, 5);
	}
	ofPopStyle();


	//mvC01.draw(0, DEPTH_HEIGHT);

	 //even points can overlap with each other, let's avoid that
	//cam1.begin();
	ofTranslate(1500, -1000, 0);
	ofRotateX(20);
	ofScale(1100, -1100, -1100); // flip the y axis and zoom in a bit
	//ofRotateX(20);
	mesh.draw();
	//cam1.end();

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	//if (key == 'r') {
	//	bRecording = !bRecording;
	//	if (bRecording && !vidRecorderC.isInitialized() && !vidRecorderDH.isInitialized()) {
	//		vidRecorderC.setup(dStamp + fileNameC + ofToString(rCounter), foregroundImg.getWidth(), foregroundImg.getHeight(), 30, 0, 3, true); // no audio
	//		vidRecorderDH.setup(dStamp + fileNameDH + ofToString(rCounter), toSaveDH.getWidth(), toSaveDH.getHeight(), 30, 0, 1, true); // no audio
	//																											   // Start recording
	//		vidRecorderC.start();
	//		vidRecorderDH.start();
	//	}
	//	else if (!bRecording && vidRecorderC.isInitialized() && vidRecorderDH.isInitialized()) { // && vidRecorderDL.isInitialized()) {
	//		vidRecorderC.setPaused(true);
	//		vidRecorderDH.setPaused(true);
	//	}
	//	else if (bRecording && vidRecorderC.isInitialized() && vidRecorderDH.isInitialized()) { // && vidRecorderDL.isInitialized()) {
	//		vidRecorderC.setPaused(false);
	//		vidRecorderDH.setPaused(false);
	//	}
	//}
	//if (key == 'c') {
	//	bRecording = false;
	//	vidRecorderC.close();
	//	vidRecorderDH.close();
	//}

	if (key == 'u') {
		for (int i = 0; i < cVideosSub.size(); i++) {

			string fn;
			bool isOverlaped = true;

			while (isOverlaped) {
				isOverlaped = false;
				fn = "/video/" + dir[ofRandom(dir.size() / 2)].getFileName();
				for (int j = 0; j < cVideos.size(); j++) {
					if (cVideos[j].getMoviePath() == fn || cVideosSub[j].getMoviePath() == fn) {
						isOverlaped = true;
					}
				}
			}
			//////////////////////////////////////////////////////////////

			if (cVideosP[i] == &cVideos[i]) {
				cVideosSub[i].close();
				dVideosSub[i].close();
				cVideosSub[i].load(fn);
				fn.replace(7, 5, "depth");
				dVideosSub[i].load(fn);
				cVideosSub[i].setLoopState(OF_LOOP_NONE);
				dVideosSub[i].setLoopState(OF_LOOP_NONE);
			}
			else if (cVideosP[i] == &cVideosSub[i]) {
				cVideos[i].close();
				dVideos[i].close();
				cVideos[i].load(fn);
				fn.replace(7, 5, "depth");
				dVideos[i].load(fn);
				cVideos[i].setLoopState(OF_LOOP_NONE);
				dVideos[i].setLoopState(OF_LOOP_NONE);
			}
		}
		isSubLoaded = true;
	}

	if (key == 'f')
		ofToggleFullscreen();
	if (key == 'i')
		dInfo = !dInfo;

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
