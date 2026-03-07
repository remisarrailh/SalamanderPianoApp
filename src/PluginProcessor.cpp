#include "PluginProcessor.h"
#include "PluginEditor.h"
#if JUCE_ANDROID
#include <android/log.h>
#define PLOG(...) __android_log_print(ANDROID_LOG_ERROR, "SALTEST", __VA_ARGS__)
#else
#define PLOG(...) DBG(juce::String::formatted(__VA_ARGS__))
#endif

SalamanderPianoProcessor::SalamanderPianoProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
#if JUCE_ANDROID
    PLOG ("=== PROCESSOR CONSTRUCTOR START ===");
#endif
    // Create parameters
    addParameter (volumeParam = new juce::AudioParameterFloat (
        juce::ParameterID { "volume", 1 }, "Volume", 0.0f, 1.0f, 0.8f));

    addParameter (reverbMixParam = new juce::AudioParameterFloat (
        juce::ParameterID { "reverbMix", 1 }, "Reverb Mix", 0.0f, 1.0f, 0.15f));

    addParameter (reverbSizeParam = new juce::AudioParameterFloat (
        juce::ParameterID { "reverbSize", 1 }, "Reverb Size", 0.0f, 1.0f, 0.5f));

    // Start MIDI device polling (standalone mode)
    midiDeviceManager.startListening();
#if JUCE_ANDROID
    PLOG ("=== PROCESSOR CONSTRUCTOR COMPLETE ===");
#endif
    DBG ("SalamanderPianoProcessor constructor complete");
}

SalamanderPianoProcessor::~SalamanderPianoProcessor()
{
    midiDeviceManager.stopListening();
}

void SalamanderPianoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    PLOG ("prepareToPlay called, sampleRate=%.1f blockSize=%d", sampleRate, samplesPerBlock);
    blockDurationSeconds = samplesPerBlock / sampleRate;
    sfizzEngine.prepareToPlay (sampleRate, samplesPerBlock);

    // Configure reverb
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;
    reverb.prepare (spec);

    dryBuffer.setSize (2, samplesPerBlock);

    // initializeEngine() runs on a background thread started in the constructor.
    // Launch exactly once — prepareToPlay can be called multiple times.
    if (! initializationStarted.exchange (true))
        juce::Thread::launch ([this] { initializeEngine(); });
}

void SalamanderPianoProcessor::releaseResources()
{
    sfizzEngine.releaseResources();
    reverb.reset();
}

void SalamanderPianoProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    const auto blockStart = juce::Time::getHighResolutionTicks();

   #if JUCE_ANDROID
    // One-time log to confirm audio device is calling processBlock
    static bool firstBlock = true;
    if (firstBlock)
    {
        firstBlock = false;
        PLOG ("processBlock FIRST CALL: channels=%d samples=%d engineReady=%d",
              buffer.getNumChannels(), buffer.getNumSamples(), (int) engineReady.load());
    }
   #endif

    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any extra output channels
    for (auto i = 2; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Process MIDI from keyboard state (on-screen keyboard)
    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);

    if (! engineReady.load())
    {
        buffer.clear();
        return;
    }

    // Forward MIDI events to sfizz
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();

        if (msg.isNoteOn())
        {
           #if JUCE_ANDROID
            PLOG ("MIDI noteOn: channel=%d note=%d vel=%d",
                  msg.getChannel(), msg.getNoteNumber(), msg.getVelocity());
           #endif
            sfizzEngine.handleNoteOn (msg.getChannel(), msg.getNoteNumber(),
                                      msg.getVelocity());
        }
        else if (msg.isNoteOff())
            sfizzEngine.handleNoteOff (msg.getChannel(), msg.getNoteNumber(),
                                       msg.getVelocity());
        else if (msg.isController())
            sfizzEngine.handleController (msg.getChannel(), msg.getControllerNumber(),
                                          msg.getControllerValue());
        else if (msg.isPitchWheel())
            sfizzEngine.handlePitchWheel (msg.getChannel(), msg.getPitchWheelValue());
    }

    // Render audio from sfizz
    sfizzEngine.renderBlock (buffer, buffer.getNumSamples());

    // Measure CPU load: elapsed / available block time (smoothed with leaky average)
    {
        const double elapsed = juce::Time::highResolutionTicksToSeconds (
            juce::Time::getHighResolutionTicks() - blockStart);
        const float load = (blockDurationSeconds > 0.0)
                           ? static_cast<float> (elapsed / blockDurationSeconds)
                           : 0.0f;
        // Smooth: 90% old + 10% new  (≈100ms time-constant at 10Hz display)
        cpuLoad.store (cpuLoad.load() * 0.9f + load * 0.1f);
    }

    // Apply volume
    float vol = volumeParam->get();
    buffer.applyGain (vol);

    // Apply reverb (wet/dry mix)
    float mix = reverbMixParam->get();

    if (mix > 0.001f)
    {
        // Save dry signal
        dryBuffer.makeCopyOf (buffer, true);

        // Update reverb parameters
        reverbParams.roomSize = reverbSizeParam->get();
        reverbParams.damping = 0.5f;
        reverbParams.wetLevel = 1.0f;
        reverbParams.dryLevel = 0.0f;
        reverbParams.width = 1.0f;
        reverb.setParameters (reverbParams);

        // Process reverb (buffer now contains wet signal)
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> context (block);
        reverb.process (context);

        // Mix dry and wet
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* dry = dryBuffer.getReadPointer (ch);
            auto* wet = buffer.getWritePointer (ch);

            for (int i = 0; i < buffer.getNumSamples(); ++i)
                wet[i] = dry[i] * (1.0f - mix) + wet[i] * mix;
        }
    }
}

juce::AudioProcessorEditor* SalamanderPianoProcessor::createEditor()
{
    return new SalamanderPianoEditor (*this);
}

void SalamanderPianoProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, true);
    stream.writeFloat (*volumeParam);
    stream.writeFloat (*reverbMixParam);
    stream.writeFloat (*reverbSizeParam);
    stream.writeFloat (savedRelease);
    stream.writeInt   (savedVoices);
}

void SalamanderPianoProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, static_cast<size_t> (sizeInBytes), false);

    if (stream.getNumBytesRemaining() >= 12)
    {
        *volumeParam    = stream.readFloat();
        *reverbMixParam = stream.readFloat();
        *reverbSizeParam = stream.readFloat();
    }
    if (stream.getNumBytesRemaining() >= 4)
        savedRelease = stream.readFloat();
    if (stream.getNumBytesRemaining() >= 4)
        savedVoices = stream.readInt();

    // Re-apply engine state that isn't driven by audio parameters
    sfizzEngine.setNumVoices (savedVoices);
    sfizzEngine.handleHdController (1, 72, savedRelease / 8.0f);
}

void SalamanderPianoProcessor::initializeEngine()
{
    PLOG ("initializeEngine START");
    loadProgress.store (0.1f);
    loadStatusMessage = "Searching for samples...";

    auto sfzFile = sampleManager.locateSfzFile();
    PLOG ("locateSfzFile returned: %s", sfzFile.getFullPathName().toRawUTF8());

    if (sfzFile == juce::File())
    {
        loadStatusMessage = sampleManager.getStatusMessage();
        PLOG ("locateSfzFile FAILED: %s", loadStatusMessage.toRawUTF8());
        loadProgress.store (0.0f);
        return;
    }

    loadProgress.store (0.3f);
    loadStatusMessage = "Loading samples...";

    const bool ok = sfizzEngine.loadSfzFile (sfzFile);

    if (ok)
    {
        PLOG ("sfizz load SUCCESS, engine ready — all samples in RAM, IO=0");
        engineReady.store (true);
        loadProgress.store (1.0f);
        loadStatusMessage = "Ready";
    }
    else
    {
        loadStatusMessage = sfizzEngine.getLoadError();
        PLOG ("sfizz load FAILED: %s", loadStatusMessage.toRawUTF8());
        loadProgress.store (0.0f);
    }
}

// This creates the plugin instance (entry point)
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SalamanderPianoProcessor();
}
