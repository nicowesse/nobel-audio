#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup()
{
    ofxVoid::ui::init(1.0f);
    Tweenzor::init();
    
    vidGrabber.setDeviceID(0);
    vidGrabber.setDesiredFrameRate(60);
    vidGrabber.initGrabber( 320, 240 );
    waveplot.allocate(vidGrabber.getWidth() * 2 + 4, 170, GL_RGBA);
    waveplot.begin(); ofClear(0, 0, 0, 0); waveplot.end();
    
    sampleData.setVerbose(true);
    sampleData.load( ofToDataPath( "i_have_a_dream.wav" ));
    
    sampler.setSample(&sampleData, 0);
    1.0f >> sampler_amp.in_mod();
    samplerTrigger >> sampler.in_trig();
    samplerTrigger.trigger(1.0f);
    
    cloud.setWindowType(pdsp::Blackman); // available windows: Rectangular, Triangular, Hann, Hamming, Blackman, BlackmanHarris, SineWindow, Tukey, Welch
    cloud.setSample(&sampleData);
    
    // PATCH
    cloud_in_position >> cloud.in_position();
    cloud_in_position_jitter >> cloud.in_position_jitter();
    cloud_in_length >> cloud.in_length();
    cloud_in_density >> cloud.in_density();
    cloud_in_distance_jitter >> cloud.in_distance_jitter();
    cloud_in_pitch >> cloud.in_pitch();
    cloud_in_pitch_jitter >> cloud.in_pitch_jitter();
//    cloud_trigger >> cloud.in_trig();
    
    delay_to_reverb_control >> delay_to_reverb.in_fade();
    
    amp_control.enableSmoothing(50.0f);
    amp_control.set(1.0f);
    
    synth.setup(12);
    synth.datatable.setup(vidGrabber.getWidth(), vidGrabber.getHeight());
    
    for (int i = 0; i < 2; i++)
    {
        
        cloud.ch(i) >> cloud_amp;
        
        // Raw Graincloud to Engine
        cloud_amp >> filter >> gain >> amp_control >> engine.audio_out(i);
        
        // Delayed Graincloud to Engine
        cloud.ch(i) >> delay >> gain >> amp_control >> engine.audio_out(i);

        // Raw Graincloud at 0.0f and delayed Graincloud at 1.0f
        cloud.ch(i) >> delay_to_reverb.in_A();
        delay >> delay_to_reverb.in_B();
        
        // Reverbed Graincloud to Engine
        delay_to_reverb >> reverb >> gain >> amp_control >> engine.audio_out(i);
        
        synth.ch(i) >> gain >> amp_control >> engine.audio_out(i);
        synth.ch(i) >> synthReverb >> gain >> amp_control >> engine.audio_out(i);
    }
    
    // Parameters
    parameters.setName("Graincloud");
    
    ofParameter<ofxVoid::types::File> samplePath = ofxVoid::utils::ParameterUtils::addParameter<ofxVoid::types::File>("Sample", ofxVoid::types::File("Speeches_mixdown", ofToDataPath("Speeches_mixdown.wav")), parameters);
//    onSampleChanged = samplePath.newListener([this](ofxVoid::types::File & param) {
//        amp_control.set(0.0f);
//
//        sampleData.load ( param.getPath() );
//        waveformGraphics.setWaveform(sampleData, 0, ofColor(0, 100, 100, 255), scene->getWidth(), scene->getHeight());
//
//        amp_control.set(1.0f);
//    });
    
    parameters.add(gain.set("Gain", -6, -48, 12));
    
    ofxVoid::utils::ParameterUtils::addParameter<float>("Sample Factor", 0.5f, 0.0f, 1.0f, parameters);
    
    ofParameterGroup cloudParameters("Cloud");
    {
        cloudParameters.add(cloud_amp.set("Amp", 1.0f, 0.001f, 2.0f));
        cloudParameters.add(cloud_in_position.set("Position", 0.5f, 0.0f, 1.0f));
        cloudParameters.add(cloud_in_length.set("Length (ms)", 2000, 0, 20000));
        cloudParameters.add(cloud_in_position_jitter.set("Position Jitter", 0.0f, 0.0f, 1.0f));
        cloudParameters.add(cloud_in_density.set("Overlap", 0.5f, 0.0f, 1.0f));
        cloudParameters.add(cloud_in_distance_jitter.set("Distance Jitter", 2000, 0, 10000));
        cloudParameters.add(cloud_in_pitch.set("Pitch", 0, -24, 24));
        cloudParameters.add(cloud_in_pitch_jitter.set("Pitch Jitter", 0.5f, 0.0f, 1.0f));
    }
    parameters.add(cloudParameters);
    
    parameters.add(reverb.getParameters("Cloud Reverb"));
    parameters.add(delay.getParameters("Cloud Delay"));
    parameters.add(delay_to_reverb_control.set("Delay -> Reverb", 0.0f, 0.0f, 1.0f));
    parameters.add(filter.getParameters("Cloud Filter"));
//    parameters.add(drone.parameters);
    parameters.add(synth.parameters);
    parameters.add(synthReverb.getParameters("Synth Reverb"));
    parameters.add( synthSmoothing.set("Wave Smoothing", 0.0f, 0.0f, 0.95f) );
    synthSmoothing.addListener(this, &ofApp::smoothCall );
    
    this->loadSettings();
    
    ui = ofxVoid::ui::CellLayout::create(false);
    ui->makeRootObject();
    ui->disableAutoDraw();
    
    auto column = ofxVoid::ui::CellLayout::create(true);
    {
        auto panel = ofxVoid::ui::Panel::create();
        panel->addComponents(ofxVoid::ui::createComponentsForParameterGroup(parameters, parameters.getName()));
        
        column->addComponent(panel);
        
        auto savePanel = ofxVoid::ui::Panel::create();
        
        auto saveButton = ofxVoid::ui::Button::create("Save");
        saveButton->onMousePressed([this]() {
            this->saveSettings();
        });
        savePanel->addComponent(saveButton);
        
        column->addComponent(savePanel, ofxVoid::ui::RESIZE_RULE_TYPE_STATIC, 50.0f * ofxVoid::ui::scale);
    }
    ui->addComponent(column, ofxVoid::ui::RESIZE_RULE_TYPE_STATIC, ofxVoid::ui::panelWidth * ofxVoid::ui::scale);
    
    scene = ofxVoid::ui::Scene2d::create();
    scene->implementDraw([&]() {
        auto rect = scene->getHitArea().rect;
        rect.scale(0.98);
        
        ofRectangle videoRect(0, 0, vidGrabber.getWidth(), vidGrabber.getHeight());
        videoRect.alignTo(rect, OF_ALIGN_HORZ_RIGHT, OF_ALIGN_VERT_TOP, OF_ALIGN_HORZ_RIGHT, OF_ALIGN_VERT_TOP);
        
        ofRectangle waveplotRect(rect.x, rect.y, rect.width - videoRect.width, videoRect.height);
        
        ofRectangle waveformRect(rect.x, videoRect.getBottom(), rect.width, rect.height - videoRect.height);
        
        vidGrabber.draw(videoRect);
        
        const float & sampleX = ofxVoid::utils::ParameterUtils::getParameter<float>("Sample Factor", parameters).get();
        float lineX = ofMap(sampleX, 0.0f, 1.0f, videoRect.getLeft(), videoRect.getRight());
        ofDrawLine(lineX, videoRect.getTop(), lineX, videoRect.getBottom());
        
        waveplot.draw(waveplotRect);
        
        waveformGraphics.draw(waveformRect.x, waveformRect.y, waveformRect.width, waveformRect.height);
        
        ofPushStyle();
        ofSetColor(255, 200);
        ofEnableBlendMode(OF_BLENDMODE_ADD);
    
        ofSetRectMode(OF_RECTMODE_CENTER);
        int grainsY = waveformRect.getCenter().y;
        for (int i = 0; i < cloud.getVoicesNum(); i++)
        {
            float xpos = waveformRect.x + (waveformRect.width * cloud.meter_position(i));
            float dimensionX = cloud.meter_env(i) * 20;
            float dimensionY = cloud.meter_env(i) * 100;
            ofSetColor(255, 128);
            ofDrawRectRounded(xpos, grainsY, dimensionX, dimensionY, 2);
            
            ofNoFill();
            ofSetColor(255, 200);
            ofDrawRectRounded(xpos, grainsY, dimensionX, dimensionY, 2);
        }
        ofDisableBlendMode();
        
        
        ofPopStyle();
    });
    ui->addComponent(scene);
    
    waveformGraphics.setWaveform(sampleData, 0, ofColor::tomato, scene->getWidth(), scene->getHeight());
    
    
    
    // Intialize engine
//    auto soundDevices = engine.listDevices();
//    ofSoundDevice headphones = *find_if(soundDevices.begin(), soundDevices.end(), [](const ofSoundDevice & device) { return  ofIsStringInString(device.name, "hodetelefoner"); });
//    ofLogNotice("ofApp") << "Headphone device ID: " << headphones.deviceID;
    
    engine.listDevices();
    engine.setDeviceID(1); // REMEMBER TO SET THIS AT THE RIGHT INDEX!!!!
    engine.setup( 44100, 512, 3 );
    
}

//--------------------------------------------------------------
void ofApp::update()
{
    cloud_in_position.set(ofxVoid::math::Shaping::easing(ofNoise(ofGetElapsedTimef())) * 0.5);
    
    vidGrabber.update();
    
    if(vidGrabber.isFrameNew() && synth.datatable.ready() ){
        
        ofPixels & pixels = vidGrabber.getPixels();
        
        // ------------------ GENERATING THE WAVE ----------------------
        
        // a pdsp::DataTable easily convert data to a waveform in real time
        // if you don't need to generate waves in real time but
        // interpolate between already stored waves pdsp::WaveTable is a better choice
        // for example if you want to convert an image you already have to a wavetable
        
        const int camW = vidGrabber.getWidth();
        const int camH = vidGrabber.getHeight();
        const float & sampleX = ofxVoid::utils::ParameterUtils::getParameter<float>("Sample Factor", parameters).get();
        int col = sampleX * camW;
        
        switch( ofxVoid::utils::ParameterUtils::getParameter<bool>("Enable Additive Synth", synth.parameters).get() ){
            case false: // converting pixels to waveform samples
                synth.datatable.begin();
                for(int n = 0; n < vidGrabber.getHeight(); ++n){
                    float sample = ofMap(pixels.getData()[col * 3 + channel + n * 3 * camW], 0, 255, -0.5f, 0.5f);
                    synth.datatable.data(n, sample);
                }
                synth.datatable.end(false);
            break; // remember, raw waveform could have DC offsets, we have filtered them in the synth using an hpf
            
            case true: // converting pixels to partials for additive synthesis
                synth.datatable.begin();
                for(int n = 0; n < camH; ++n){
                    float partial = ofMap(pixels.getData()[col * 3 + channel + n * 3 * camW], 0, 255, 0.0f, 1.0f);
                    synth.datatable.data(n, partial);
                }
                synth.datatable.end(true);
            break;
        }
        
        float plotH = waveplot.getHeight();
        // ----------------- PLOTTING THE WAVEFORM ---------------------
        waveplot.begin();
        ofClear(0, 0, 0, 0);
        
        ofTranslate(2, 2);
        ofNoFill();
        switch( ofxVoid::utils::ParameterUtils::getParameter<bool>("Enable Additive Synth", synth.parameters).get() ){
            case true: // plot the raw waveforms
                ofBeginShape();
                for(int n = 0; n < camH; ++n){
                    float y = ofMap(pixels.getData()[col * 3 + channel + n * 3 * camW], 0, 255, plotH, 0);
                    ofVertex( n * 2, y );
                }
                ofEndShape();
            break;
            
            case false: // plot the partials
                for(int n = 0; n < camH; ++n){
                    float partial = ofMap(pixels.getData()[col * 3 + channel + n * 3 * camW], 0, 255, 0.0f, 1.0f);
                    int h = waveplot.getHeight() * partial;
                    int y = waveplot.getHeight() - h;
                    ofDrawLine(n*2, y, n*2, camH );
                }
            break;
        }
        waveplot.end();
        
    }
}

//--------------------------------------------------------------
void ofApp::draw()
{
    ofBackground(0);
    ui->draw();
}

//--------------------------------------------------------------
void ofApp::smoothCall( float & value ) {
    synth.datatable.smoothing( value  );
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
    auto mousePos = scene->getGlobalToLocalCoord(glm::vec2(x, y));
    auto rect = scene->getHitArea().rect;
    
    ofMap(mousePos.x, rect.getLeft(), rect.getRight(), 0.0f, 1.0f, true) >> cloud_position;
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y)
{

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h)
{

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg)
{

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo)
{
    
}

//--------------------------------------------------------------
void ofApp::saveSettings()
{
    ofJson json;
    
    ofSerialize(json, parameters);
    ofSavePrettyJson("settings.json", json);
    
    ofLogNotice("ofApp") << "Settings saved!";
}

//--------------------------------------------------------------
void ofApp::loadSettings()
{
    ofFile file("settings.json");
    
    if (file.exists())
    {
        ofJson json;
        file >> json;
        
        ofDeserialize(json, parameters);
        
        ofLogNotice("ofApp") << "Settings loaded!";
    }
}
