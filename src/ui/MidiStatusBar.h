#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class MidiStatusBar : public juce::Component
{
public:
    MidiStatusBar();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setDeviceName (const juce::String& name);
    void setConnected (bool connected);
    void setVoiceCount (int count, int maxVoices = 64);
    void setCpuLoad (float load); // 0.0 – 1.0
    void setIoLoad  (float load); // 0.0 – 1.0

private:
    juce::Label deviceLabel;
    juce::Label voiceLabel;
    juce::Label cpuLabel;
    juce::Label ioLabel;
    bool isConnected = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiStatusBar)
};
