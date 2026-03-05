#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

class PianoKeyboard : public juce::Component
{
public:
    PianoKeyboard (juce::MidiKeyboardState& state);

    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override;

    void shiftOctaveDown();
    void shiftOctaveUp();

private:
    juce::MidiKeyboardState& keyboardState;
    juce::MidiKeyboardComponent keyboard;
    juce::TextButton octaveDownButton;
    juce::TextButton octaveUpButton;
    juce::TextButton panicButton;
    juce::Label rangeLabel;

    int currentBaseOctave = 2;

    void updateRange();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PianoKeyboard)
};
