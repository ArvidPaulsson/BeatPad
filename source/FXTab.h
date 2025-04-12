#pragma once

#include "ADSRComponent.h"
#include "LoopingComponent.h"
#include "PluginProcessor.h"
#include <juce_gui_basics/juce_gui_basics.h>

class FXTab : public juce::TabbedComponent
{
public:
    FXTab (PluginProcessor& p, int padId);
    ~FXTab() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PluginProcessor& processorRef;
    int padId { -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FXTab)
};