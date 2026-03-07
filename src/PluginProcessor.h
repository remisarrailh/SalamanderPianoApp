#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "SfizzEngine.h"
#include "SampleManager.h"
#include "MidiDeviceManager.h"

class SalamanderPianoProcessor : public juce::AudioProcessor
{
public:
    SalamanderPianoProcessor();
    ~SalamanderPianoProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Salamander Piano"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Access for editor
    SfizzEngine& getSfizzEngine() { return sfizzEngine; }
    SampleManager& getSampleManager() { return sampleManager; }
    MidiDeviceManager& getMidiDeviceManager() { return midiDeviceManager; }
    juce::MidiKeyboardState& getKeyboardState() { return keyboardState; }

    // Parameters
    juce::AudioParameterFloat* volumeParam = nullptr;
    juce::AudioParameterFloat* reverbMixParam = nullptr;
    juce::AudioParameterFloat* reverbSizeParam = nullptr;

    // Loading state
    bool isEngineReady() const { return engineReady.load(); }
    float getLoadProgress() const { return loadProgress.load(); }
    juce::String getLoadStatusMessage() const { return loadStatusMessage; }

    // CPU load (0.0 – 1.0) measured in processBlock
    float getCpuLoad() const { return cpuLoad.load(); }
    float getIoLoad()  const { return sfizzEngine.getIoLoad(); }

    // Non-parameter state (release in seconds, voices count)
    float getSavedRelease() const { return savedRelease; }
    int   getSavedVoices()  const { return savedVoices; }
    void  setSavedRelease (float v) { savedRelease = v; }
    void  setSavedVoices  (int v)   { savedVoices = v; }

private:
    void initializeEngine();

    SfizzEngine sfizzEngine;
    SampleManager sampleManager;
    MidiDeviceManager midiDeviceManager;

    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    juce::MidiKeyboardState keyboardState;

    std::atomic<bool> engineReady { false };
    std::atomic<bool> initializationStarted { false };
    std::atomic<float> loadProgress { 0.0f };
    juce::String loadStatusMessage { "Initializing..." };

    // CPU load measurement
    std::atomic<float> cpuLoad { 0.0f };
    double blockDurationSeconds = 0.0; // updated in prepareToPlay

    float savedRelease { 4.0f }; // seconds (0-8)
    int   savedVoices  { 32 };

    // Dry buffer for reverb wet/dry mix
    juce::AudioBuffer<float> dryBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalamanderPianoProcessor)
};
