#pragma once

#include "ofMain.h"

#include "ofxVoid/ui/ui.h"
#include "ofxVoid/ui/CellLayout.h"
#include "ofxVoid/ui/Panel.h"
#include "ofxVoid/ui/Scene2d.h"
#include "ofxVoid/ui/Button.h"
#include "ofxVoid/math/Shaping.h"
#include "ofxVoid/types/File.h"
#include "ofxVoid/utils/ParameterUtils.h"

#include "ofxPDSP.h"
#include "Modules.h"
#include "ofxTweenzor.h"

class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    
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
    
private:
    // UI
    shared_ptr<ofxVoid::ui::CellLayout> ui;
    shared_ptr<ofxVoid::ui::Scene2d> scene;
    ofParameterGroup parameters;
    
    ofVideoGrabber vidGrabber;
    ofFbo waveplot;
    int channel;
    
    void saveSettings();
    void loadSettings();
    
    // Sound
    pdsp::Engine            engine;
    
    pdsp::ParameterGain     gain;
    
    pdsp::Sampler           sampler;
    pdsp::TriggerControl    samplerTrigger;
    pdsp::Amp               sampler_amp;
    
    pdsp::SampleBufferPlotter  waveformGraphics;
    
    pdsp::SampleBuffer      sampleData;
    ofEventListener         onSampleChanged;
    pdsp::GrainCloud        cloud;
    pdsp::ParameterAmp      cloud_amp;
    pdsp::PatchNode         cloud_position;
    pdsp::Parameter         cloud_in_position;
    pdsp::Parameter         cloud_in_position_jitter;
    pdsp::Parameter         cloud_in_length;
    pdsp::Parameter         cloud_in_density;
    pdsp::Parameter         cloud_in_distance_jitter;
    pdsp::Parameter         cloud_in_pitch_jitter;
    pdsp::Parameter         cloud_in_pitch;
    pdsp::TriggerControl    cloud_trigger;
    ofEventListener         cloud_trigger_listener;
    
    pdsp::LinearCrossfader  delay_to_reverb;
    pdsp::Parameter         delay_to_reverb_control;
    
    ParameterReverb         reverb;
    ParameterDelay          delay;
    ParameterFilter         filter;
    Drone                   drone;
    PolySynth               synth;
    ParameterReverb         synthReverb;
    ofParameter<float>      synthSmoothing;
    void smoothCall( float & value );
    
    pdsp::ParameterAmp      amp_control;
    
};
