#include "ofxPDSP.h"
#include "ofxVoid/utils/ParameterUtils.h"

class ParameterDelay : public pdsp::Patchable {
public:
    ofParameterGroup parameters;
    
    ParameterDelay() { patch(); }
    ParameterDelay(const ParameterDelay & other) { patch(); }
    
    void patch()
    {
        addModuleInput("in_signal", delay);
        addModuleOutput("signal", amp);
        addModuleOutput("raw", delay);
        
        // Patch
        amp.enableSmoothing(50.0f);
        time_control >> delay.in_time();
        feedback_control >> delay.in_feedback();
        damping_control >> delay.in_damping();
        
        delay >> amp;
        
        parameters.setName("Delay");
        parameters.add(amp.set("Amp", 1.0f, 0.0f, 2.0f));
        parameters.add(time_control.set("Time (ms)", 60, 1, 2000));
        parameters.add(feedback_control.set("Feedback", 0.5f, 0.0f, 1.0f));
        parameters.add(damping_control.set("Damping", 0.0f, 0.0f, 1.0f));
    }
    
    pdsp::Patchable & in_signal()
    {
        return in("in_signal");
    }
    
    pdsp::Patchable & out_signal()
    {
        return out("signal");
    }
    
    pdsp::Patchable & out_raw()
    {
        return out("raw");
    }
    
    ofParameterGroup & getParameters(string label = "Delay")
    {
        parameters.setName(label);
        return parameters;
    }
    
private:
    pdsp::Delay             delay;
    pdsp::Parameter         time_control;
    pdsp::Parameter         feedback_control;
    pdsp::Parameter         damping_control;
    pdsp::ParameterAmp      amp;
};

class ParameterReverb : public pdsp::Patchable {
public:
    ofParameterGroup parameters;
    
    ParameterReverb() { patch(); }
    ParameterReverb(const ParameterReverb & other) { patch(); }
    
    void patch()
    {
        addModuleInput("in_signal", container);
        addModuleOutput("signal", amp);
        addModuleOutput("raw", reverb);
        
        // Patch
        amp.enableSmoothing(50.0f);
        1.0f >> container.in_mod();
        
        density_control.enableSmoothing(50.0f);
        
        mod_amount_control >> reverb.in_mod_amount();
        mod_freq_control >> reverb.in_mod_freq();
        time_control >> reverb.in_time();
        density_control >> reverb.in_density();
        damping_control >> reverb.in_damping();
        
        dry_wet_control >> dry_wet.in_fade();
        
        container >> dry_wet.in_A();
        container >> reverb >> dry_wet.in_B();
        dry_wet >> amp;
        
        // Parameter
        parameters.setName("Reverb");
        parameters.add(amp.set("Amp", 1.0f, 0.0f, 2.0f));
        parameters.add(mod_amount_control.set("Mod Amount (ms)", 0.8f, 0.0f, 1.0f));
        parameters.add(mod_freq_control.set("Mod Freq", 0.2f, 0.0f, 10.0f));
        parameters.add(time_control.set("Time (s)", 3.33f, 0.0f, 20.0f));
        parameters.add(density_control.set("Density", 0.5f, 0.0f, 1.0f));
        parameters.add(damping_control.set("Damping", 0.5f, 0.0f, 1.0f));
        parameters.add(dry_wet_control.set("Dry/Wet", 1.0f, 0.001f, 1.0f));
    }
    
    pdsp::Patchable & in_signal()
    {
        return in("in_signal");
    }
    
    pdsp::Patchable & out_signal()
    {
        return out("out_signal");
    }
    
    pdsp::Patchable & out_raw()
    {
        return out("raw");
    }
    
    ofParameterGroup & getParameters(string label = "Reverb")
    {
        parameters.setName(label);
        return parameters;
    }
    
private:
    pdsp::Amp               container;
    
    pdsp::BasiVerb          reverb;
    pdsp::Parameter         mod_amount_control;
    pdsp::Parameter         mod_freq_control;
    pdsp::Parameter         time_control;
    pdsp::Parameter         density_control;
    pdsp::Parameter         damping_control;
    
    pdsp::LinearCrossfader  dry_wet;
    pdsp::Parameter         dry_wet_control;
    
    pdsp::ParameterAmp      amp;
};

class ParameterFilter : public pdsp::Patchable {
public:
    ofParameterGroup parameters;
    
    ParameterFilter() { patch(); }
    ParameterFilter(const ParameterFilter & other) { patch(); }
    
    void patch()
    {
        addModuleInput("signal", container.in_signal());
        addModuleInput("cutoff", filter.in_cutoff());
        addModuleOutput("signal", amp);
        addModuleOutput("raw", container.out_signal());
        
        // Patch
        amp.enableSmoothing(50.0f);
        
        cutoff_control >> filter.in_cutoff();
        reso_control >> filter.in_reso();
        mode_control >> filter.in_mode();
        
        dry_wet_control >> dry_wet.in_fade();
        
        1.0f >> container.in_mod();
        
        container >> dry_wet.in_A();
        container >> filter >> dry_wet.in_B();
        
        dry_wet >> amp;
        
        // Parameter
        parameters.setName("Filter");
        parameters.add(amp.set("Amp", 1.0f, 0.0f, 2.0f));
        parameters.add(mode_control.set("Mode", 0, 0, 4));
        parameters.add(cutoff_control.set("Cutoff", 60.0f, 24.0f, 128.0f));
        parameters.add(reso_control.set("Reso", 0.0f, 0.0f, 1.0f));
        parameters.add(dry_wet_control.set("Dry/Wet", 0.0f, 0.001f, 1.0f));
    }
    
    pdsp::Patchable & in_signal() { return in("signal"); }
    pdsp::Patchable & in_cutoff() { return in("cutoff"); }
    pdsp::Patchable & out_signal() { return out("signal"); }
    pdsp::Patchable & raw_signal() { return out("raw"); }
    
    ofParameterGroup & getParameters(string label = "Filter")
    {
        parameters.setName(label);
        return parameters;
    }
    
private:
    pdsp::Amp               container;
    
    pdsp::VAFilter          filter;
    pdsp::Parameter         cutoff_control;
    pdsp::Parameter         pitch_control;
    pdsp::Parameter         reso_control;
    pdsp::Parameter         mode_control;
    
    pdsp::LinearCrossfader  dry_wet;
    pdsp::Parameter         dry_wet_control;
    pdsp::ParameterAmp      amp;
    
};

class ParameterLFO : public pdsp::Patchable {
public:
    
    ParameterLFO() { patch(); }
    ParameterLFO(const ParameterLFO & other) { patch(); }
    
    void patch()
    {
        addModuleInput("retrig", lfo.in_retrig());
        addModuleInput("freq", lfo.in_freq());
        addModuleOutput("signal", amp.out_signal());
        
        // Patch
        amp.enableSmoothing(50.0f);
        
        lfo_switch.resize(5);
        lfo.out_sine() >> lfo_switch.input(0);
        lfo.out_triangle() >> lfo_switch.input(1);
        lfo.out_saw() >> lfo_switch.input(2);
        lfo.out_square() >> lfo_switch.input(3);
        lfo.out_sample_and_hold() >> lfo_switch.input(4);
        
        lfo_freq_control >> lfo.in_freq();
        lfo_switch_control >> lfo_switch.in_select();
        lfo_switch >> amp;
        
        // Parameters
        parameters.setName("LFO");
        parameters.add(amp.set("Amp", 1.0f, 0.0f, 1.0f));
        parameters.add(lfo_switch_control.set("Mode", 0, 0, 4));
        parameters.add(lfo_freq_control.set("Freq", 0.5f, 0.01f, 80.0f));
        
    }
    
    pdsp::Patchable & in_retrig() { return in("retrig"); }
    pdsp::Patchable & in_freq() { return in("freq"); }
    pdsp::Patchable & out_signal() { return out("signal"); }
    
    ofParameterGroup & getParameters(string label = "LFO")
    {
        parameters.setName(label);
        return parameters;
    }
    
    ofParameterGroup parameters;
    
private:
    pdsp::ParameterAmp amp;
    pdsp::LFO lfo;
    pdsp::Parameter lfo_freq_control;
    pdsp::Switch lfo_switch;
    pdsp::Parameter lfo_switch_control;
};

class Drone : public pdsp::Patchable {
public:
    Drone() { patch(); }
    Drone(const Drone & other) { patch(); }
    
    void patch()
    {
//        addModuleInput("signal", container.in_signal());
        addModuleOutput("signal", amp);
//        addModuleOutput("raw", container.out_signal());
        
        // Patch
        amp.enableSmoothing(50.0f);
        osc_pitch_control >> osc.in_pitch();
        
        lfo_to_filter_control >> lfo_to_filter.in_mod();
        lfo.out_signal() >> lfo_to_filter >> filter.in_cutoff();
        
        osc.out_saw() >> drive >> filter >> amp;
        
        // Parameters
        parameters.setName("Drone");
        
        parameters.add(amp.set("Amp", 1.0f, 0.0f, 2.0f));
        parameters.add(osc_pitch_control.set("Pitch", 56.0f, 12.0f, 128.0f));
        parameters.add(filter.parameters);
        parameters.add(lfo.parameters);
        parameters.add(lfo_to_filter_control.set("LFO -> Filter", 0.0f, 0.0f, 4.0f));
    }
    
    pdsp::Patchable & in_signal()
    {
        return in("signal");
    }
    
    pdsp::Patchable & out_signal()
    {
        return out("signal");
    }
    
    pdsp::Patchable & raw_signal()
    {
        return out("raw");
    }
    
    ofParameterGroup parameters;
    
private:
    pdsp::VAOscillator osc;
    pdsp::Parameter osc_pitch_control;
    
    pdsp::Saturator1 drive;
    ParameterFilter filter;
    ParameterLFO    lfo;
    pdsp::Amp       lfo_to_filter;
    pdsp::Parameter lfo_to_filter_control;
    
    pdsp::ParameterAmp amp;
};

class PolySynth {

public:
    // class to rapresent each synth voice ------------
    class Voice : public pdsp::Patchable {
        friend class PolySynth;
    
    public:
        Voice(){}
        Voice(const Voice& other){}
        
        float meter_mod_env() const;
        float meter_pitch() const;

    private:
        void setup(PolySynth & m, int v);

        pdsp::PatchNode     voiceTrigger;
        
        pdsp::DataOscillator    oscillator;
        pdsp::VAFilter          filter;
        pdsp::Amp               amp;


        pdsp::ADSR          envelope;
    }; // end voice class -----------------------------


    // synth public API --------------------------------------

    void setup( int numVoice );
    
    pdsp::DataTable  datatable;

    pdsp::Patchable& ch( int index );

    vector<Voice>       voices;
    ofParameterGroup    parameters;

private: // --------------------------------------------------

    pdsp::ParameterGain gain;
    
    pdsp::Parameter     pitch_control;

    pdsp::Parameter     cutoff_ctrl;
    pdsp::Parameter     reso_ctrl;
    pdsp::Parameter     filter_mode_ctrl;

    pdsp::Parameter     env_attack_ctrl;
    pdsp::Parameter     env_decay_ctrl;
    pdsp::Parameter     env_sustain_ctrl;
    pdsp::Parameter     env_release_ctrl;
    pdsp::ParameterAmp  env_filter_amt;

    pdsp::Parameter     lfo_speed_ctrl;
    pdsp::Parameter     lfo_wave_ctrl;

    pdsp::LFO           lfo;
    pdsp::Switch        lfo_switch;
    pdsp::ParameterAmp  lfo_filter_amt;
    
    pdsp::LowCut            leakDC;
    
    // chorus ------------------------
    pdsp::DimensionChorus   chorus;
    ofParameterGroup    ui_chorus;
    pdsp::Parameter     chorus_speed_ctrl;
    pdsp::Parameter     chorus_depth_ctrl;
    
    std::vector<float> partials_vector;

};


