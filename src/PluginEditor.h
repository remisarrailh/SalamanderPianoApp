#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "LookAndFeel.h"
#include "ui/HeaderPanel.h"
#include "ui/MidiStatusBar.h"
#include "ui/ControlPanel.h"
#include "ui/PianoKeyboard.h"
#include "ui/TroubleshootPanel.h"

class SalamanderPianoEditor : public juce::AudioProcessorEditor,
                               private juce::Timer,
                               private MidiDeviceManager::Listener,
                               private TroubleshootPanel::Listener
{
public:
    explicit SalamanderPianoEditor (SalamanderPianoProcessor&);
    ~SalamanderPianoEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void midiDevicesChanged() override;
    void troubleshootPanelResized() override;

    void updateMidiStatus();
    void updateTroubleshootInfo();
    void paintLoadingOverlay (juce::Graphics& g);

    SalamanderPianoProcessor& processor;
    SalamanderLookAndFeel lookAndFeel;

    HeaderPanel headerPanel;
    MidiStatusBar midiStatusBar;
    ControlPanel controlPanel;
    PianoKeyboard pianoKeyboard;
    TroubleshootPanel troubleshootPanel;

    // Layout constants
    static constexpr int headerHeight = 50;
    static constexpr int midiBarHeight = 28;
    static constexpr int controlHeight = 144;
    static constexpr int margin = 4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalamanderPianoEditor)
};
