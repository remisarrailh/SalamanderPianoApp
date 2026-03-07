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
    keyboard.setAvailableRange (kMinNote, kMaxNote);
    keyboard.setScrollButtonsVisible (false);
    keyboard.setLowestVisibleKey (36); // start at C2

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

    updateRangeLabel();
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

    // Use a fixed key width large enough to be clickable/touchable.
    // At this size not all 88 keys fit at once, so setLowestVisibleKey
    // can actually scroll — making the arrow buttons effective.
    // On a wide window (~1400 px) all 52 white keys become visible naturally.
    keyboard.setKeyWidth (18.0f);
    keyboard.setBounds (area);
}

void PianoKeyboard::shiftOctaveDown()
{
    int lowest = keyboard.getLowestVisibleKey();
    int newLowest = lowest - 12;
    // Wrap: below A0 → jump to the last full octave before C8
    if (newLowest < kMinNote)
        newLowest = kMaxNote - ((kMaxNote - kMinNote) % 12) - 12;
    keyboard.setLowestVisibleKey (newLowest);
    updateRangeLabel();
}

void PianoKeyboard::shiftOctaveUp()
{
    int lowest = keyboard.getLowestVisibleKey();
    int newLowest = lowest + 12;
    // Wrap: past C8 → back to A0
    if (newLowest >= kMaxNote)
        newLowest = kMinNote;
    keyboard.setLowestVisibleKey (newLowest);
    updateRangeLabel();
}

void PianoKeyboard::updateRangeLabel()
{
    int lowest = keyboard.getLowestVisibleKey();
    auto noteName = juce::MidiMessage::getMidiNoteName (lowest, true, true, 4);
    rangeLabel.setText (noteName, juce::dontSendNotification);
}
