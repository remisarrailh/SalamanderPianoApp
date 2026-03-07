#include "HeaderPanel.h"
#include "../LookAndFeel.h"

HeaderPanel::HeaderPanel()
{
    titleLabel.setText ("Salamander Grand Piano  v" SALAMANDER_VERSION, juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::accentColour);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("Alexander Holm (CC-BY 3.0) | SFZ: kinwie | App: MIT",
                           juce::dontSendNotification);
    subtitleLabel.setFont (juce::FontOptions (11.0f));
    subtitleLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    subtitleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (subtitleLabel);

    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

void HeaderPanel::paint (juce::Graphics& g)
{
    g.setColour (SalamanderLookAndFeel::panelColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f);
}

void HeaderPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    titleLabel.setBounds (area.removeFromTop (26));
    subtitleLabel.setBounds (area.removeFromTop (18));
}

void HeaderPanel::mouseUp (const juce::MouseEvent&)
{
    juce::URL ("https://sfzinstruments.github.io/pianos/salamander").launchInDefaultBrowser();
}
