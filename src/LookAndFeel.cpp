#include "LookAndFeel.h"
#include <juce_audio_utils/juce_audio_utils.h>

const juce::Colour SalamanderLookAndFeel::backgroundColour { 0xFF1E1E2E };
const juce::Colour SalamanderLookAndFeel::panelColour { 0xFF2A2A3D };
const juce::Colour SalamanderLookAndFeel::accentColour { 0xFF6C8EBF };
const juce::Colour SalamanderLookAndFeel::textColour { 0xFFE0E0E0 };
const juce::Colour SalamanderLookAndFeel::dimTextColour { 0xFF808090 };
const juce::Colour SalamanderLookAndFeel::greenLedColour { 0xFF4CAF50 };
const juce::Colour SalamanderLookAndFeel::redLedColour { 0xFFE05050 };

SalamanderLookAndFeel::SalamanderLookAndFeel()
{
    // Set the colour scheme
    setColour (juce::ResizableWindow::backgroundColourId, backgroundColour);
    setColour (juce::Label::textColourId, textColour);
    setColour (juce::Slider::thumbColourId, accentColour);
    setColour (juce::Slider::trackColourId, panelColour.brighter (0.2f));
    setColour (juce::Slider::backgroundColourId, panelColour);
    setColour (juce::TextButton::buttonColourId, panelColour);
    setColour (juce::TextButton::textColourOnId, textColour);
    setColour (juce::TextButton::textColourOffId, textColour);

    // MidiKeyboardComponent colours
    setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xFFDDDDE0));
    setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xFF2A2A2A));
    setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xFF888888));
    setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, accentColour.withAlpha (0.3f));
    setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, accentColour.withAlpha (0.5f));
}

void SalamanderLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                               const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style != juce::Slider::LinearHorizontal)
    {
        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, 0, 0, style, slider);
        return;
    }

    auto trackHeight = 6.0f;
    auto trackY = static_cast<float> (y) + (static_cast<float> (height) - trackHeight) * 0.5f;

    // Background track
    g.setColour (panelColour.brighter (0.1f));
    g.fillRoundedRectangle (static_cast<float> (x), trackY,
                            static_cast<float> (width), trackHeight, 3.0f);

    // Active track
    auto fillWidth = sliderPos - static_cast<float> (x);
    g.setColour (accentColour);
    g.fillRoundedRectangle (static_cast<float> (x), trackY, fillWidth, trackHeight, 3.0f);

    // Thumb
    auto thumbSize = 16.0f;
    auto thumbY = static_cast<float> (y) + (static_cast<float> (height) - thumbSize) * 0.5f;
    g.setColour (accentColour.brighter (0.3f));
    g.fillEllipse (sliderPos - thumbSize * 0.5f, thumbY, thumbSize, thumbSize);

    // Thumb border
    g.setColour (accentColour.darker (0.2f));
    g.drawEllipse (sliderPos - thumbSize * 0.5f, thumbY, thumbSize, thumbSize, 1.5f);
}

void SalamanderLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                               juce::Slider& slider)
{
    // Use default V4 rotary for now
    LookAndFeel_V4::drawRotarySlider (g, x, y, width, height, sliderPos,
                                       rotaryStartAngle, rotaryEndAngle, slider);
}

void SalamanderLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    g.fillAll (label.findColour (juce::Label::backgroundColourId));

    auto textArea = getLabelBorderSize (label).subtractedFrom (label.getLocalBounds());

    g.setColour (label.findColour (juce::Label::textColourId));
    g.setFont (label.getFont());
    g.drawFittedText (label.getText(), textArea, label.getJustificationType(),
                      juce::jmax (1, (int) ((float) textArea.getHeight() / label.getFont().getHeight())),
                      label.getMinimumHorizontalScale());
}
