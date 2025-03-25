#pragma once

#include "BeatPad.h"
#include "PluginProcessor.h"
#include <juce_audio_devices/midi_io/juce_MidiDevices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

class BeatPadContainer : public juce::Component
{
public:
    BeatPadContainer (PluginProcessor& p);
    ~BeatPadContainer() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void setMidiNotes (const std::array<int, 9>& notes);
    void setPadColors (juce::Colour normal, juce::Colour triggered);

    BeatPad* getPad (int index);

private:
    // Initialize MIDI input
    int nbrOfPads { 9 };

    PluginProcessor& processorRef;
    juce::OwnedArray<BeatPad> pads;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatPadContainer)
};