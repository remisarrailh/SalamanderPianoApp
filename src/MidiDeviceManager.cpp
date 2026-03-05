#include "MidiDeviceManager.h"

MidiDeviceManager::MidiDeviceManager()
{
}

MidiDeviceManager::~MidiDeviceManager()
{
    stopListening();
}

void MidiDeviceManager::startListening()
{
    if (! listening)
    {
        listening = true;
        updateDeviceList();
        startTimer (1000); // Poll every second
    }
}

void MidiDeviceManager::stopListening()
{
    if (listening)
    {
        stopTimer();
        listening = false;
    }
}

juce::StringArray MidiDeviceManager::getConnectedDeviceNames() const
{
    return connectedDeviceNames;
}

bool MidiDeviceManager::hasConnectedDevice() const
{
    return connectedDeviceNames.size() > 0;
}

int MidiDeviceManager::getDeviceCount() const
{
    return connectedDeviceNames.size();
}

void MidiDeviceManager::enableAllMidiInputs (juce::AudioDeviceManager& deviceManager)
{
    auto midiInputs = juce::MidiInput::getAvailableDevices();

    for (const auto& input : midiInputs)
        deviceManager.setMidiInputDeviceEnabled (input.identifier, true);
}

void MidiDeviceManager::timerCallback()
{
    updateDeviceList();
}

void MidiDeviceManager::updateDeviceList()
{
    auto devices = juce::MidiInput::getAvailableDevices();
    juce::StringArray currentIdentifiers;
    juce::StringArray currentNames;

    for (const auto& d : devices)
    {
        currentIdentifiers.add (d.identifier);
        currentNames.add (d.name);
    }

    if (currentIdentifiers != lastKnownIdentifiers)
    {
        lastKnownIdentifiers = currentIdentifiers;
        connectedDeviceNames = currentNames;
        listeners.call ([](Listener& l) { l.midiDevicesChanged(); });
    }
}
