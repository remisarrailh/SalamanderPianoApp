#include "PluginEditor.h"
#if JUCE_ANDROID
#include <android/log.h>
#endif

SalamanderPianoEditor::SalamanderPianoEditor (SalamanderPianoProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      controlPanel (p.volumeParam, p.reverbMixParam, p.reverbSizeParam),
      pianoKeyboard (p.getKeyboardState())
{
    DBG ("SalamanderPianoEditor constructor START");
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (headerPanel);
    addAndMakeVisible (midiStatusBar);
    addAndMakeVisible (controlPanel);
    addAndMakeVisible (pianoKeyboard);
    addAndMakeVisible (troubleshootPanel);

    troubleshootPanel.addListener (this);
    processor.getMidiDeviceManager().addListener (this);

    // Sustain button (in ControlPanel) sends CC64 to sfizz engine
    controlPanel.onSustainChanged = [this] (bool on)
    {
        processor.getSfizzEngine().handleController (1, 64, on ? 127 : 0);
    };

    // Release slider sends CC72 hdcc (0.0-1.0 normalized = 0-8s)
    controlPanel.onReleaseChanged = [this] (float v)
    {
        processor.getSfizzEngine().handleHdController (1, 72, v);
        processor.setSavedRelease (v * 8.0f);
    };

    // Voice count ComboBox
    controlPanel.onVoicesChanged = [this] (int n)
    {
        processor.getSfizzEngine().setNumVoices (n);
        processor.setSavedVoices (n);
    };

    // Restore non-parameter UI state (release, voices) from saved processor state
    controlPanel.initState (processor.getSavedRelease(), processor.getSavedVoices());

    // Initial UI state
    updateMidiStatus();
    updateTroubleshootInfo();

    // Update timer for voice count, loading progress, etc.
    startTimerHz (10);

    // Default window size (landscape for comfortable keyboard)
    setSize (700, 500);
   #if ! JUCE_ANDROID
    // On Android the StandaloneFilterWindow drives fullscreen sizing; adding
    // a ResizableCornerComponent + min-size constraints causes conflicts.
    setResizable (true, true);
    setResizeLimits (400, 300, 1600, 1200);
   #endif
    DBG ("SalamanderPianoEditor constructor END, size=" + juce::String (getWidth()) + "x" + juce::String (getHeight()));
}

SalamanderPianoEditor::~SalamanderPianoEditor()
{
    processor.getMidiDeviceManager().removeListener (this);
    troubleshootPanel.removeListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

void SalamanderPianoEditor::paint (juce::Graphics& g)
{
    g.fillAll (SalamanderLookAndFeel::backgroundColour);

   #if JUCE_ANDROID
    // Diagnostic: show current editor size (remove once clicks work)
    g.setColour (juce::Colours::white.withAlpha (0.6f));
    g.setFont (juce::FontOptions (11.0f));
    g.drawText ("Editor: " + juce::String (getWidth()) + "x" + juce::String (getHeight()),
                4, 4, 200, 16, juce::Justification::left);
   #endif
}

void SalamanderPianoEditor::paintOverChildren (juce::Graphics& g)
{
    if (! processor.isEngineReady())
        paintLoadingOverlay (g);
}

void SalamanderPianoEditor::resized()
{
   #if JUCE_ANDROID
    __android_log_print (ANDROID_LOG_ERROR, "SALTEST",
                         "=== EDITOR resized: %d x %d ===", getWidth(), getHeight());
   #endif

    auto area = getLocalBounds().reduced (margin);

   #if JUCE_ANDROID
    const bool isLandscape = getWidth() > getHeight();
    if (isLandscape)
    {
        // Landscape: status bar shrinks/hides, nav bar moves to right side,
        // possible camera cutout on left. Use symmetric side insets.
        area.removeFromTop    (28);   // thin status bar in landscape
        area.removeFromBottom (8);
        area.removeFromLeft   (64);   // camera cutout / left nav area
        area.removeFromRight  (64);   // navigation bar on the right
    }
    else
    {
        // Portrait: status bar at top, gesture bar at bottom
        area.removeFromTop    (48);   // status bar + possible notch
        area.removeFromBottom (24);   // gesture navigation bar
    }
   #endif

    // Calculate total content height
    auto troubleshootHeight = troubleshootPanel.getRequiredHeight();
    auto pianoHeight = juce::jmin (180, area.getHeight() - headerHeight - midiBarHeight - controlHeight - troubleshootHeight);
    if (pianoHeight < 60) pianoHeight = 60;
    int totalContentHeight = headerHeight + midiBarHeight + controlHeight + troubleshootHeight + pianoHeight;

    // Center content vertically if there's extra space
    if (totalContentHeight < area.getHeight())
    {
        int extraSpace = area.getHeight() - totalContentHeight;
        area.removeFromTop (extraSpace / 2);
    }

    headerPanel.setBounds (area.removeFromTop (headerHeight));
    midiStatusBar.setBounds (area.removeFromTop (midiBarHeight));
    controlPanel.setBounds (area.removeFromTop (controlHeight));

    // Troubleshoot panel sits between controls and keyboard
    troubleshootPanel.setBounds (area.removeFromTop (troubleshootHeight));

    // Piano keyboard fills remaining allocated space
    auto pianoArea = area.withSizeKeepingCentre (area.getWidth(), pianoHeight);
    pianoKeyboard.setBounds (pianoArea);
}

void SalamanderPianoEditor::timerCallback()
{
    auto& engine = processor.getSfizzEngine();
    midiStatusBar.setVoiceCount (engine.getActiveVoiceCount(), engine.getMaxVoices());
    midiStatusBar.setCpuLoad (processor.getCpuLoad());
    midiStatusBar.setIoLoad  (processor.getIoLoad());

    // Repaint while loading, and once more on the frame it becomes ready
    bool engineReady = processor.isEngineReady();
    if (! engineReady || ! wasEngineReady)
        repaint();
    wasEngineReady = engineReady;
}

void SalamanderPianoEditor::mouseDown (const juce::MouseEvent& e)
{
   #if JUCE_ANDROID
    __android_log_print (ANDROID_LOG_ERROR, "SALTEST",
                         "=== EDITOR mouseDown at %d,%d source=%d ===",
                         e.x, e.y, (int) e.source.getType());
   #endif
    juce::AudioProcessorEditor::mouseDown (e);
}

void SalamanderPianoEditor::midiDevicesChanged()
{
    // Called from MidiDeviceManager when device list changes
    juce::MessageManager::callAsync ([this]()
    {
        updateMidiStatus();
    });
}

void SalamanderPianoEditor::troubleshootPanelResized()
{
    resized();
}

void SalamanderPianoEditor::updateMidiStatus()
{
    auto& midiMgr = processor.getMidiDeviceManager();
    auto names = midiMgr.getConnectedDeviceNames();

    if (names.size() > 0)
    {
        midiStatusBar.setConnected (true);
        midiStatusBar.setDeviceName (names.joinIntoString (", "));
    }
    else
    {
        midiStatusBar.setConnected (false);
        midiStatusBar.setDeviceName ("");
    }
}

void SalamanderPianoEditor::updateTroubleshootInfo()
{
    auto& engine = processor.getSfizzEngine();
    auto& sampleMgr = processor.getSampleManager();

    juce::String sfizzInfo;
    if (engine.isLoaded())
        sfizzInfo = juce::String (engine.getNumSamples()) + " samples, "
                  + juce::String (engine.getNumRegions()) + " regions";
    else if (processor.isEngineReady() || processor.getLoadProgress() > 0.05f)
        sfizzInfo = "Loading...";
    else
        sfizzInfo = "Not loaded";
    troubleshootPanel.setSfizzInfo (sfizzInfo);

    troubleshootPanel.setSamplePathInfo (
        sampleMgr.areSamplesReady()
            ? sampleMgr.getSfzDirectory().getFullPathName()
            : sampleMgr.getStatusMessage());

    // Audio device info from processor
    auto sampleRate = processor.getSampleRate();
    auto blockSize = processor.getBlockSize();

    if (sampleRate > 0)
    {
        auto latencyMs = (static_cast<double> (blockSize) / sampleRate) * 1000.0;

        troubleshootPanel.setAudioInfo (
            juce::String (static_cast<int> (sampleRate)) + "Hz, buffer "
            + juce::String (blockSize));

        troubleshootPanel.setLatencyInfo (
            "~" + juce::String (latencyMs, 1) + "ms");
    }

    auto& midiMgr = processor.getMidiDeviceManager();
    troubleshootPanel.setMidiInfo (
        juce::String (midiMgr.getDeviceCount()) + " device(s) detected");
}

void SalamanderPianoEditor::paintLoadingOverlay (juce::Graphics& g)
{
    // Semi-transparent overlay
    g.setColour (SalamanderLookAndFeel::backgroundColour.withAlpha (0.85f));
    g.fillAll();

    auto area = getLocalBounds();
    auto center = area.getCentre();

    // Loading text
    g.setColour (SalamanderLookAndFeel::textColour);
    g.setFont (juce::FontOptions (18.0f));
    g.drawText (processor.getLoadStatusMessage(),
                area.withSizeKeepingCentre (300, 30).translated (0, -20),
                juce::Justification::centred);

    // Progress bar
    auto barWidth = 250.0f;
    auto barHeight = 8.0f;
    auto barX = static_cast<float> (center.x) - barWidth * 0.5f;
    auto barY = static_cast<float> (center.y) + 10.0f;

    g.setColour (SalamanderLookAndFeel::panelColour);
    g.fillRoundedRectangle (barX, barY, barWidth, barHeight, 4.0f);

    g.setColour (SalamanderLookAndFeel::accentColour);
    g.fillRoundedRectangle (barX, barY, barWidth * processor.getLoadProgress(), barHeight, 4.0f);
}
