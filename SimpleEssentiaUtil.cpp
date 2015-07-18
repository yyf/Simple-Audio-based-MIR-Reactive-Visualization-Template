//
//  SimpleEssentiaUtil.cpp
//  SimpleAudioBasedMIRVisualizationTemplate
//
//  Created by Yuan-Yi Fan on 7/17/15.
//  Modified from ofxAudioAnalyzer by Leo Zimmerman https://github.com/leozimmerman
//

#include "SimpleEssentiaUtil.h"


void SimpleEssentiaUtil::setup(int bufferSize, int sampleRate){
    sr = sampleRate;
    audioBuffer.resize(bufferSize);
    essentia::init();
    
    AlgorithmFactory& factory = AlgorithmFactory::instance();
    rms = factory.create("RMS");
    energy = factory.create("Energy");
    power = factory.create("InstantPower");
    dcremoval = factory.create("DCRemoval", "sampleRate", sr);
    
    dcremoval->input("signal").set(audioBuffer);
    dcremoval->output("signal").set(audioBuffer_dc);
    rms->input("array").set(audioBuffer_dc);
    rms->output("rms").set(rmsValue);
    energy->input("array").set(audioBuffer_dc);
    energy->output("energy").set(energyValue);
    power->input("array").set(audioBuffer_dc);
    power->output("power").set(powerValue);
}

//--------------------------------------------------------------
void SimpleEssentiaUtil::exit(){
    delete dcremoval;
    delete rms;
    delete energy;
    delete power;
    essentia::shutdown();
}

//--------------------------------------------------------------
void SimpleEssentiaUtil::analyze(float * iBuffer, int bufferSize){
    
    vector <float> fBuffer;
    fBuffer.resize(bufferSize);
    memcpy(&fBuffer[0], iBuffer, sizeof(float) * bufferSize);
    for (int i=0; i<bufferSize;i++){
        audioBuffer[i] = (Real) fBuffer[i];
    }
    
    dcremoval->compute();
    rms->compute();
    energy->compute();
    power->compute();
    
    for (int i=0; i<spec.size(); i++)
        spectrum_f[i] = log10((float) spec[i]);

    rms_f = (float) rmsValue;
    
    energy_f = (float) energyValue;

    power_f = (float) powerValue;
}

