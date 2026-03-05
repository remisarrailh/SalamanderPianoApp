#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <sfizz.h>
#include <atomic>

class SfizzEngine
{
public:
    SfizzEngine();
    ~SfizzEngine();

    // Lifecycle
    bool loadSfzFile (const juce::File& sfzFile);
    bool isLoaded() const { return loaded.load(); }

    // Audio configuration
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    // Real-time audio rendering (called from audio thread)
    void renderBlock (juce::AudioBuffer<float>& buffer, int numSamples);

    // MIDI input (called from audio thread via processBlock)
    void handleNoteOn (int channel, int noteNumber, int velocity);
    void handleNoteOff (int channel, int noteNumber, int velocity);
    void handleController (int channel, int ccNumber, int ccValue);
    void handleHdController (int channel, int ccNumber, float normalizedValue); // float 0.0-1.0
    void handlePitchWheel (int channel, int pitchWheelValue);

    // Status
    int getActiveVoiceCount() const;
    int getMaxVoices() const;
    juce::String getLoadError() const { return loadError; }
    int getNumRegions() const;
    int getNumSamples() const;
    float getIoLoad() const { return ioLoad.load(); }

private:
    sfizz_synth_t* synth = nullptr;
    std::atomic<bool> loaded { false };
    std::atomic<float> ioLoad { 0.0f };
    juce::String loadError;
    double currentSampleRate = 48000.0;
    int currentBlockSize = 512;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SfizzEngine)
};
