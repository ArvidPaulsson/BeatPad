#pragma once
#include "BeatPadContainer.h"
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class WaveThumbnail : public juce::Component
{
public:
    WaveThumbnail (PluginProcessor& p);
    ~WaveThumbnail();

    void paint (juce::Graphics& g) override;
    void resized() override;

    void drawThumbnail (juce::Graphics& g, juce::AudioBuffer<float>& waveform);

private:
    std::vector<float> audioPoints;
    PluginProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveThumbnail)
};