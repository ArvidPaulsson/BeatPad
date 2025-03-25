#pragma once

#include "PluginProcessor.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

class BeatPad : public juce::Component, public juce::FileDragAndDropTarget
{
public:
    BeatPad (PluginProcessor& p, int padId);
    ~BeatPad() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void filesDropped (const juce::StringArray& files, int x, int y) override;
    bool isInterestedInFileDrag (const juce::StringArray& files) override;

    void mouseDown (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

private:
    PluginProcessor& processorRef;

    const std::map<int, int> padMidiNotes = {
        { 1, 60 }, // C2
        { 2, 61 }, // C#2
        { 3, 63 }, // D2
        { 4, 64 }, // D#2
        { 5, 65 }, // E2
        { 6, 66 }, // F2
        { 7, 67 }, // F#2
        { 8, 68 }, // G2
        { 9, 69 } // G#2
    };

    int padId;
    int midiNote { -1 };
    juce::String padText = "Drop a sample here";
    bool sampleLoaded { false };
    bool isTriggered { false };
    juce::String beatPadFileName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatPad)
};