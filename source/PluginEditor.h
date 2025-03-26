#pragma once

#include "ADSRComponent.h"
#include "BeatPadContainer.h"
#include "BinaryData.h"
#include "PluginProcessor.h"
#include "WaveThumbnail.h"
#include "melatonin_inspector/melatonin_inspector.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void updateADSRVisibility (int padId);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    int activePadId { -1 };

    PluginProcessor& processorRef;
    BeatPadContainer beatPadContainer;
    WaveThumbnail waveThumbnail;
    std::unique_ptr<ADSRComponent> adsrComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
