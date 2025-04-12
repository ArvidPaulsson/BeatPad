#pragma once
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

enum class LoopingMode {
    NoLoop,
    Forward,
    PingPong,
};

class LoopingComponent : public juce::Component
{
public:
    LoopingComponent (PluginProcessor& p, int padId);
    ~LoopingComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setLoopingMode (LoopingMode mode) { loopMode = mode; }
    LoopingMode getLoopingMode() const { return loopMode; }
    void loopModeSelectorChanged();

    void drawLoopPointMarkers (juce::Graphics& g, int startX, int endX);
    void setLoopStart (float start);
    void setLoopEnd (float end);
    void setLoopPointsSeconds (const juce::Range<double>& loopPoints);

private:
    PluginProcessor& processorRef;
    int padId { -1 };

    LoopingMode loopMode;
    juce::ComboBox loopModeSelector;
    juce::Label loopModeLabel { "Select Looping Mode" };

    juce::Slider loopStartSlider, loopEndSlider;
    juce::Label loopStartLabel, loopEndLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mLoopStartAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mLoopEndAttachment;

    juce::Range<double> loopPointsSeconds { 0.0, 0.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoopingComponent)
};