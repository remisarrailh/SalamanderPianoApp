#include "MidiStatusBar.h"
#include "../LookAndFeel.h"

MidiStatusBar::MidiStatusBar()
{
    deviceLabel.setText ("No MIDI device", juce::dontSendNotification);
    deviceLabel.setFont (juce::FontOptions (14.0f));
    deviceLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    deviceLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (deviceLabel);

    voiceLabel.setText ("Voices: 0/64", juce::dontSendNotification);
    voiceLabel.setFont (juce::FontOptions (13.0f));
    voiceLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    voiceLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (voiceLabel);

    cpuLabel.setText ("CPU: 0%", juce::dontSendNotification);
    cpuLabel.setFont (juce::FontOptions (13.0f));
    cpuLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    cpuLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (cpuLabel);

    ioLabel.setText ("RT: 0%", juce::dontSendNotification);
    ioLabel.setFont (juce::FontOptions (13.0f));
    ioLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    ioLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (ioLabel);
}

void MidiStatusBar::paint (juce::Graphics& g)
{
    auto area = getLocalBounds().reduced (2);
    g.setColour (SalamanderLookAndFeel::panelColour.darker (0.1f));
    g.fillRoundedRectangle (area.toFloat(), 4.0f);

    // LED indicator
    auto ledArea = juce::Rectangle<float> (10.0f, (static_cast<float> (getHeight()) - 10.0f) * 0.5f, 10.0f, 10.0f);
    g.setColour (isConnected ? SalamanderLookAndFeel::greenLedColour
                             : SalamanderLookAndFeel::dimTextColour.darker (0.3f));
    g.fillEllipse (ledArea);

    // LED glow when connected
    if (isConnected)
    {
        g.setColour (SalamanderLookAndFeel::greenLedColour.withAlpha (0.3f));
        g.fillEllipse (ledArea.expanded (3.0f));
    }
}

void MidiStatusBar::resized()
{
    auto area = getLocalBounds().reduced (8);
    area.removeFromLeft (18); // Space for LED

    cpuLabel.setBounds  (area.removeFromRight (68));
    ioLabel.setBounds   (area.removeFromRight (62));
    voiceLabel.setBounds (area.removeFromRight (100));
    deviceLabel.setBounds (area);
}

void MidiStatusBar::setDeviceName (const juce::String& name)
{
    deviceLabel.setText (name.isEmpty() ? "No MIDI device" : name,
                         juce::dontSendNotification);

    if (name.isNotEmpty())
        deviceLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::textColour);
    else
        deviceLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
}

void MidiStatusBar::setConnected (bool connected)
{
    isConnected = connected;
    repaint();
}

void MidiStatusBar::setVoiceCount (int count, int maxVoices)
{
    voiceLabel.setText ("Voices: " + juce::String (count) + "/" + juce::String (maxVoices),
                        juce::dontSendNotification);

    // Colour the voice count red when near the limit
    const auto colour = (count >= maxVoices * 3 / 4)
                        ? juce::Colours::orangered
                        : SalamanderLookAndFeel::dimTextColour;
    voiceLabel.setColour (juce::Label::textColourId, colour);
}

void MidiStatusBar::setCpuLoad (float load)
{
    const int pct = juce::roundToInt (load * 100.0f);
    cpuLabel.setText ("CPU: " + juce::String (pct) + "%",
                      juce::dontSendNotification);

    const auto colour = (load > 0.85f) ? juce::Colours::orangered
                      : (load > 0.60f) ? juce::Colours::orange
                                       : SalamanderLookAndFeel::dimTextColour;
    cpuLabel.setColour (juce::Label::textColourId, colour);
}

void MidiStatusBar::setIoLoad (float load)
{
    const int pct = juce::roundToInt (load * 100.0f);
    ioLabel.setText ("RT: " + juce::String (pct) + "%",
                     juce::dontSendNotification);

    const auto colour = (load > 0.50f) ? juce::Colours::orangered
                      : (load > 0.25f) ? juce::Colours::orange
                                       : SalamanderLookAndFeel::dimTextColour;
    ioLabel.setColour (juce::Label::textColourId, colour);
}
