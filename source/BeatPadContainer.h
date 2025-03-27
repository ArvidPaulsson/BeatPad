#pragma once

#include "ADSRComponent.h"
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

    void updateADSRVisibility (int padId);

    BeatPad getPad (int index);
    ADSRComponent* getADSRComponent (int padId);

private:
    int nbrOfPads { 9 };
    int currentPadId { -1 };

    PluginProcessor& processorRef;
    // juce::OwnedArray<BeatPad> pads;
    std::vector<std::unique_ptr<BeatPad>> pads;
    std::vector<std::unique_ptr<ADSRComponent>> adsrComponents;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatPadContainer)
};