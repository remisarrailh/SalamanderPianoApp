#include "SfizzEngine.h"

#if JUCE_ANDROID
#include <android/log.h>
#define SFZLOG(...) __android_log_print(ANDROID_LOG_ERROR, "SALTEST", __VA_ARGS__)
#else
#define SFZLOG(...) DBG(juce::String::formatted(__VA_ARGS__))
#endif

SfizzEngine::SfizzEngine()
{
    synth = sfizz_create_synth();

   #if JUCE_ANDROID
    sfizz_set_num_voices (synth, 16);
    // 96000 floats = 2s preload per sample at 48000Hz (native device rate).
    // WAVs are now 48kHz so sfizz plays directly, zero SRC overhead.
    sfizz_set_preload_size (synth, 96000);
    SFZLOG ("SfizzEngine: voices=16, preload=96000/2s@48kHz (Android)");
   #else
    sfizz_set_num_voices (synth, 32);
   #endif
}

SfizzEngine::~SfizzEngine()
{
    if (synth != nullptr)
        sfizz_free (synth);
}

bool SfizzEngine::loadSfzFile (const juce::File& sfzFile)
{
    if (synth == nullptr)
    {
        loadError = "Synth not initialized";
        return false;
    }

    SFZLOG ("loadSfzFile: path=%s", sfzFile.getFullPathName().toRawUTF8());

    if (! sfzFile.existsAsFile())
    {
        loadError = "SFZ file not found: " + sfzFile.getFullPathName();
        SFZLOG ("loadSfzFile: FILE NOT FOUND!");
        return false;
    }
    SFZLOG ("loadSfzFile: file exists, calling sfizz_load_file...");

    loaded.store (false);

    auto path = sfzFile.getFullPathName().toStdString();
    bool result = sfizz_load_file (synth, path.c_str());

    if (result)
    {
        loaded.store (true);
        loadError.clear();
        SFZLOG ("loadSfzFile: sfizz_load_file SUCCESS");
    }
    else
    {
        loadError = "Failed to load SFZ: " + sfzFile.getFullPathName();
        SFZLOG ("loadSfzFile: sfizz_load_file FAILED");
    }

    return result;
}

void SfizzEngine::setNumVoices (int numVoices)
{
    if (synth != nullptr)
        sfizz_set_num_voices (synth, juce::jlimit (1, 256, numVoices));
}

void SfizzEngine::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;

    if (synth != nullptr)
    {
        sfizz_set_sample_rate (synth, static_cast<float> (sampleRate));
        sfizz_set_samples_per_block (synth, samplesPerBlock);
    }
}

void SfizzEngine::releaseResources()
{
    // Nothing to do - sfizz handles its own memory
}

void SfizzEngine::renderBlock (juce::AudioBuffer<float>& buffer, int numSamples)
{
    if (synth == nullptr || ! loaded.load())
    {
        buffer.clear();
        return;
    }

    // sfizz expects float** (array of channel pointers)
    float* channels[2] = {
        buffer.getWritePointer (0),
        buffer.getWritePointer (1)
    };

    // Time the render call — this includes disk streaming wait time
    const auto t0 = juce::Time::getHighResolutionTicks();
    sfizz_render_block (synth, channels, 2, numSamples);
    const double elapsed = juce::Time::highResolutionTicksToSeconds (
        juce::Time::getHighResolutionTicks() - t0);

    if (currentSampleRate > 0.0 && numSamples > 0)
    {
        const double blockSec = numSamples / currentSampleRate;
        const float ratio = juce::jlimit (0.0f, 1.0f,
            static_cast<float> (elapsed / blockSec));
        ioLoad.store (ioLoad.load() * 0.9f + ratio * 0.1f);
    }
}

void SfizzEngine::handleNoteOn (int /*channel*/, int noteNumber, int velocity)
{
    if (synth != nullptr && loaded.load())
        sfizz_send_note_on (synth, 0, noteNumber, velocity);
}

void SfizzEngine::handleNoteOff (int /*channel*/, int noteNumber, int velocity)
{
    if (synth != nullptr && loaded.load())
        sfizz_send_note_off (synth, 0, noteNumber, velocity);
}

void SfizzEngine::handleController (int /*channel*/, int ccNumber, int ccValue)
{
    if (synth != nullptr && loaded.load())
        sfizz_send_cc (synth, 0, ccNumber, ccValue);
}

void SfizzEngine::handleHdController (int /*channel*/, int ccNumber, float normalizedValue)
{
    if (synth != nullptr && loaded.load())
        sfizz_send_hdcc (synth, 0, ccNumber, normalizedValue);
}

void SfizzEngine::handlePitchWheel (int /*channel*/, int pitchWheelValue)
{
    if (synth != nullptr && loaded.load())
        sfizz_send_pitch_wheel (synth, 0, pitchWheelValue);
}

int SfizzEngine::getActiveVoiceCount() const
{
    if (synth != nullptr)
        return sfizz_get_num_active_voices (synth);
    return 0;
}

int SfizzEngine::getMaxVoices() const
{
    if (synth != nullptr)
        return (int) sfizz_get_num_voices (synth);
   #if JUCE_ANDROID
    return 16;
   #else
    return 64;
   #endif
}

int SfizzEngine::getNumRegions() const
{
    if (synth != nullptr)
        return sfizz_get_num_regions (synth);
    return 0;
}

int SfizzEngine::getNumSamples() const
{
    if (synth != nullptr)
        return static_cast<int> (sfizz_get_num_preloaded_samples (synth));
    return 0;
}
