#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

class ControlPanel : public juce::Component
{
public:
    ControlPanel (juce::AudioParameterFloat* volumeParam,
                  juce::AudioParameterFloat* reverbMixParam,
                  juce::AudioParameterFloat* reverbSizeParam);

    void resized() override;
    void paint (juce::Graphics& g) override;

    bool isSustainOn() const { return sustainOn; }

    // Callback when sustain button is toggled (sends CC64)
    std::function<void (bool sustainOn)> onSustainChanged;
    // Callback when release slider changes (sends CC72 hdcc, value 0.0-1.0)
    std::function<void (float normalizedRelease)> onReleaseChanged;

private:
    juce::Slider volumeSlider;
    juce::Slider reverbMixSlider;
    juce::Slider reverbSizeSlider;

    juce::Label volumeLabel;
    juce::Label reverbMixLabel;
    juce::Label reverbSizeLabel;

    juce::Slider releaseSlider;
    juce::Label  releaseLabel;

    juce::TextButton sustainButton;
    bool sustainOn = false;

    std::unique_ptr<juce::SliderParameterAttachment> volumeAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> reverbMixAttachment;
    std::unique_ptr<juce::SliderParameterAttachment> reverbSizeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ControlPanel)
};
