#include "PianoKeyboard.h"
#include "../LookAndFeel.h"
#if JUCE_ANDROID
#include <android/log.h>
#endif

PianoKeyboard::PianoKeyboard (juce::MidiKeyboardState& state)
    : keyboardState (state),
      keyboard (state, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setInterceptsMouseClicks (true, true);
    addAndMakeVisible (keyboard);
    keyboard.setAvailableRange (21, 108);
    keyboard.setScrollButtonsVisible (false);

    octaveDownButton.setButtonText ("<");
    octaveDownButton.onClick = [this]() { shiftOctaveDown(); };
    addAndMakeVisible (octaveDownButton);

    octaveUpButton.setButtonText (">");
    octaveUpButton.onClick = [this]() { shiftOctaveUp(); };
    addAndMakeVisible (octaveUpButton);

    panicButton.setButtonText ("!");
    panicButton.setTooltip ("All notes off");
    panicButton.onClick = [this]()
    {
        for (int ch = 1; ch <= 16; ++ch)
            keyboardState.allNotesOff (ch);
    };
    addAndMakeVisible (panicButton);

    rangeLabel.setFont (juce::FontOptions (12.0f));
    rangeLabel.setColour (juce::Label::textColourId, SalamanderLookAndFeel::dimTextColour);
    rangeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (rangeLabel);

    updateRange();
}

void PianoKeyboard::mouseDown (const juce::MouseEvent& e)
{
   #if JUCE_ANDROID
    __android_log_print (ANDROID_LOG_ERROR, "SALTEST",
                         "=== PIANO mouseDown at %d,%d bounds=%dx%d ===",
                         e.x, e.y, getWidth(), getHeight());
   #endif
    juce::Component::mouseDown (e);
}

void PianoKeyboard::resized()
{
    auto area = getLocalBounds();
    auto topBar = area.removeFromTop (24);

    auto buttonWidth = 36;
    octaveDownButton.setBounds (topBar.removeFromLeft (buttonWidth));
    panicButton.setBounds       (topBar.removeFromLeft (buttonWidth));
    octaveUpButton.setBounds   (topBar.removeFromRight (buttonWidth));
    rangeLabel.setBounds (topBar);

    keyboard.setBounds (area);
}

void PianoKeyboard::shiftOctaveDown()
{
    if (currentBaseOctave > 0)
    {
        currentBaseOctave--;
        updateRange();
    }
}

void PianoKeyboard::shiftOctaveUp()
{
    if (currentBaseOctave < 5)
    {
        currentBaseOctave++;
        updateRange();
    }
}

void PianoKeyboard::updateRange()
{
    int lowestNote = currentBaseOctave * 12 + 12;
    int highestNote = lowestNote + 36;  // 3 octaves

    lowestNote = juce::jmax (21, lowestNote);
    highestNote = juce::jmin (108, highestNote);

    keyboard.setAvailableRange (lowestNote, highestNote);

    auto noteName = juce::MidiMessage::getMidiNoteName (lowestNote, true, true, 4);
    auto noteNameHigh = juce::MidiMessage::getMidiNoteName (highestNote, true, true, 4);
    rangeLabel.setText (noteName + " - " + noteNameHigh, juce::dontSendNotification);
}
