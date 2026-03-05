#include "TroubleshootPanel.h"
#include "../LookAndFeel.h"

TroubleshootPanel::TroubleshootPanel()
{
    toggleButton.setButtonText ("Troubleshoot");
    toggleButton.onClick = [this]() { toggleExpanded(); };
    toggleButton.setColour (juce::TextButton::buttonColourId,
                            SalamanderLookAndFeel::panelColour.darker (0.15f));
    addAndMakeVisible (toggleButton);

    setupLabel (audioInfoLabel);
    setupLabel (latencyInfoLabel);
    setupLabel (midiInfoLabel);
    setupLabel (sfizzInfoLabel);
    setupLabel (samplePathLabel);

    audioInfoLabel.setText ("Audio: --", juce::dontSendNotification);
    latencyInfoLabel.setText ("Latency: --", juce::dontSendNotification);
    midiInfoLabel.setText ("MIDI: --", juce::dontSendNotification);
    sfizzInfoLabel.setText ("sfizz: --", juce::dontSendNotification);
    samplePathLabel.setText ("Samples: --", juce::dontSendNotification);
}

void TroubleshootPanel::setupLabel (juce::Label& label)
{
    label.setFont (juce::FontOptions (12.0f));
    label.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    label.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (label);
    label.setVisible (false);
}

void TroubleshootPanel::paint (juce::Graphics& g)
{
    g.setColour (SalamanderLookAndFeel::panelColour.darker (0.15f));
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f);
}

void TroubleshootPanel::resized()
{
    auto area = getLocalBounds().reduced (4);
    toggleButton.setBounds (area.removeFromTop (24));

    if (expanded)
    {
        area.removeFromTop (4);
        auto contentArea = area.reduced (8, 0);

        audioInfoLabel.setBounds (contentArea.removeFromTop (rowHeight));
        latencyInfoLabel.setBounds (contentArea.removeFromTop (rowHeight));
        midiInfoLabel.setBounds (contentArea.removeFromTop (rowHeight));
        sfizzInfoLabel.setBounds (contentArea.removeFromTop (rowHeight));
        samplePathLabel.setBounds (contentArea.removeFromTop (rowHeight));
    }
}

void TroubleshootPanel::toggleExpanded()
{
    expanded = ! expanded;

    toggleButton.setButtonText (expanded ? "Troubleshoot (hide)" : "Troubleshoot");

    audioInfoLabel.setVisible (expanded);
    latencyInfoLabel.setVisible (expanded);
    midiInfoLabel.setVisible (expanded);
    sfizzInfoLabel.setVisible (expanded);
    samplePathLabel.setVisible (expanded);

    listeners.call ([](Listener& l) { l.troubleshootPanelResized(); });
}

int TroubleshootPanel::getRequiredHeight() const
{
    return expanded ? expandedHeight : collapsedHeight;
}

void TroubleshootPanel::setAudioInfo (const juce::String& info)
{
    audioInfoLabel.setText ("Audio: " + info, juce::dontSendNotification);
}

void TroubleshootPanel::setLatencyInfo (const juce::String& info)
{
    latencyInfoLabel.setText ("Latency: " + info, juce::dontSendNotification);
}

void TroubleshootPanel::setMidiInfo (const juce::String& info)
{
    midiInfoLabel.setText ("MIDI: " + info, juce::dontSendNotification);
}

void TroubleshootPanel::setSfizzInfo (const juce::String& info)
{
    sfizzInfoLabel.setText ("sfizz: " + info, juce::dontSendNotification);
}

void TroubleshootPanel::setSamplePathInfo (const juce::String& info)
{
    samplePathLabel.setText ("Samples: " + info, juce::dontSendNotification);
}
