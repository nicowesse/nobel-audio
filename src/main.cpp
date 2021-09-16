#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	ofGLFWWindowSettings s;
    //s.setGLVersion(3, 2);
    s.setSize(1920, 1080);
    s.stencilBits = 8;
    
    auto mainWindow = ofCreateWindow(s);
    shared_ptr<ofApp> mainApp = make_shared<ofApp>();
    ofRunApp(mainWindow, mainApp);
    
    ofRunMainLoop();
}
