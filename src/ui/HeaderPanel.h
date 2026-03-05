#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class HeaderPanel : public juce::Component
{
public:
    HeaderPanel();

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent& e) override;

private:
    juce::Label titleLabel;
    juce::Label subtitleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderPanel)
};
