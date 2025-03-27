#pragma once

#include "ADSRComponent.h"
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

    std::function<void (int)> onSelected;

    // Getters
    int getPadId() { return padId; }
    int getMidiNote() { return midiNote; }
    juce::String getPadText() { return padText; }
    bool getSampleLoaded() { return sampleLoaded; }
    bool getIsTriggered() { return isTriggered; }
    juce::String getBeatPadFileName() { return beatPadFileName; }

private:
    PluginProcessor& processorRef;

    const std::map<int, int> padMidiNotes = {
        { 1, 60 },
        { 2, 61 },
        { 3, 62 },
        { 4, 63 },
        { 5, 64 },
        { 6, 65 },
        { 7, 66 },
        { 8, 67 },
        { 9, 68 }
    };

    int padId { -1 };
    int midiNote { -1 };
    juce::String padText = "Drop a sample here";
    bool sampleLoaded { false };
    bool isTriggered { false };
    juce::String beatPadFileName;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeatPad)
};