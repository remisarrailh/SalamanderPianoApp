#include "ControlPanel.h"
#include "../LookAndFeel.h"

ControlPanel::ControlPanel (juce::AudioParameterFloat* volumeParam,
                            juce::AudioParameterFloat* reverbMixParam,
                            juce::AudioParameterFloat* reverbSizeParam)
{
    auto setupSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& text)
    {
        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
        slider.setRange (0.0, 1.0, 0.01);
        addAndMakeVisible (slider);

        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::FontOptions (13.0f));
        label.setColour (juce::Label::textColourId, SalamanderLookAndFeel::textColour);
        label.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (label);
    };

    setupSlider (volumeSlider, volumeLabel, "Volume");
    setupSlider (reverbMixSlider, reverbMixLabel, "Reverb");
    setupSlider (reverbSizeSlider, reverbSizeLabel, "Room");

    // Release slider: range 0-8s displayed, normalized 0-1 sent as CC72 hdcc.
    // Default 0.5 = 4s. Set high for natural piano sustain without pedal.
    releaseSlider.setSliderStyle (juce::Slider::LinearHorizontal);
    releaseSlider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 50, 20);
    releaseSlider.setRange (0.0, 8.0, 0.1);
    releaseSlider.setValue (4.0, juce::dontSendNotification);  // matches SFZ set_hdcc72=0.5 -> 4s
    releaseSlider.setTextValueSuffix ("s");
    releaseSlider.onValueChange = [this]()
    {
        if (onReleaseChanged)
            onReleaseChanged ((float) (releaseSlider.getValue() / 8.0));
    };
    addAndMakeVisible (releaseSlider);

    releaseLabel.setText ("Release", juce::dontSendNotification);
    releaseLabel.setFont (juce::FontOptions (13.0f));
    releaseLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::textColour);
    releaseLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (releaseLabel);

    // Wide sustain toggle button
    sustainButton.setButtonText ("Sustain");
    sustainButton.setClickingTogglesState (true);
    sustainButton.setColour (juce::TextButton::buttonOnColourId, SalamanderLookAndFeel::accentColour);
    sustainButton.onClick = [this]()
    {
        sustainOn = sustainButton.getToggleState();
        if (onSustainChanged)
            onSustainChanged (sustainOn);
    };
    addAndMakeVisible (sustainButton);

    // Parameter attachments
    volumeAttachment = std::make_unique<juce::SliderParameterAttachment> (*volumeParam, volumeSlider);
    reverbMixAttachment = std::make_unique<juce::SliderParameterAttachment> (*reverbMixParam, reverbMixSlider);
    reverbSizeAttachment = std::make_unique<juce::SliderParameterAttachment> (*reverbSizeParam, reverbSizeSlider);
}

void ControlPanel::paint (juce::Graphics& g)
{
    g.setColour (SalamanderLookAndFeel::panelColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat().reduced (2.0f), 6.0f);
}

void ControlPanel::resized()
{
    auto area = getLocalBounds().reduced (8, 4);
    auto labelWidth = 60;
    auto sliderRowHeight = 24;

    auto row1 = area.removeFromTop (sliderRowHeight);
    volumeLabel.setBounds (row1.removeFromLeft (labelWidth));
    volumeSlider.setBounds (row1);

    auto row2 = area.removeFromTop (sliderRowHeight);
    reverbMixLabel.setBounds (row2.removeFromLeft (labelWidth));
    reverbMixSlider.setBounds (row2);

    auto row3 = area.removeFromTop (sliderRowHeight);
    reverbSizeLabel.setBounds (row3.removeFromLeft (labelWidth));
    reverbSizeSlider.setBounds (row3);

    auto row4 = area.removeFromTop (sliderRowHeight);
    releaseLabel.setBounds (row4.removeFromLeft (labelWidth));
    releaseSlider.setBounds (row4);

    area.removeFromTop (4);
    sustainButton.setBounds (area);
}
