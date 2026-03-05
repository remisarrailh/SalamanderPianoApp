#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include <functional>
#include <vector>

class MidiDeviceManager : private juce::Timer
{
public:
    MidiDeviceManager();
    ~MidiDeviceManager() override;

    // Start/stop polling for devices (standalone mode only)
    void startListening();
    void stopListening();

    // Status
    juce::StringArray getConnectedDeviceNames() const;
    bool hasConnectedDevice() const;
    int getDeviceCount() const;

    // Listener for UI updates
    class Listener
    {
    public:
        virtual ~Listener() = default;
        virtual void midiDevicesChanged() = 0;
    };

    void addListener (Listener* l) { listeners.add (l); }
    void removeListener (Listener* l) { listeners.remove (l); }

    // Enable all MIDI inputs on a given AudioDeviceManager (standalone mode)
    static void enableAllMidiInputs (juce::AudioDeviceManager& deviceManager);

private:
    void timerCallback() override;
    void updateDeviceList();

    juce::StringArray connectedDeviceNames;
    juce::StringArray lastKnownIdentifiers;
    juce::ListenerList<Listener> listeners;
    bool listening = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiDeviceManager)
};
