#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class SalamanderLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SalamanderLookAndFeel();

    // Colours
    static const juce::Colour backgroundColour;
    static const juce::Colour panelColour;
    static const juce::Colour accentColour;
    static const juce::Colour textColour;
    static const juce::Colour dimTextColour;
    static const juce::Colour greenLedColour;
    static const juce::Colour redLedColour;

    // Slider customization
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    // Label customization
    void drawLabel (juce::Graphics& g, juce::Label& label) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SalamanderLookAndFeel)
};
