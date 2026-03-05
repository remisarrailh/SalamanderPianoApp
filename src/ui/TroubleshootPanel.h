#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class TroubleshootPanel : public juce::Component
{
public:
    TroubleshootPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Toggle expanded/collapsed
    void toggleExpanded();
    bool isExpanded() const { return expanded; }

    // Update displayed info
    void setAudioInfo (const juce::String& info);
    void setLatencyInfo (const juce::String& info);
    void setMidiInfo (const juce::String& info);
    void setSfizzInfo (const juce::String& info);
    void setSamplePathInfo (const juce::String& info);

    // Returns the required height
    int getRequiredHeight() const;

    // Listener for expand/collapse
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void troubleshootPanelResized() = 0;
    };

    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

private:
    bool expanded = false;
    juce::TextButton toggleButton;

    juce::Label audioInfoLabel;
    juce::Label latencyInfoLabel;
    juce::Label midiInfoLabel;
    juce::Label sfizzInfoLabel;
    juce::Label samplePathLabel;

    juce::ListenerList<Listener> listeners;

    static constexpr int collapsedHeight = 32;
    static constexpr int expandedHeight = 160;
    static constexpr int rowHeight = 20;

    void setupLabel (juce::Label& label);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TroubleshootPanel)
};
