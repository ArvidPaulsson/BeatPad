#include "FXTab.h"

FXTab::FXTab (PluginProcessor& p, int padId)
    : juce::TabbedComponent (juce::TabbedButtonBar::TabsAtTop),
      processorRef (p),
      padId (padId)
{
    std::printf ("Creating FXTab for pad %d\n", padId);
    setTabBarDepth (30);
    addTab ("ADSR", juce::Colours::lightgrey, new ADSRComponent (processorRef, padId), true);
    addTab ("Looping", juce::Colours::lightgrey, new LoopingComponent (processorRef, padId), true);
}
FXTab::~FXTab()
{
    // Destructor
}

void FXTab::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::cadetblue.darker());
}

void FXTab::resized()
{
    juce::TabbedComponent::resized();
}