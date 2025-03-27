#include "ADSRComponent.h"

ADSRComponent::ADSRComponent (PluginProcessor& p, int padId) : processorRef (p), padId (padId)
{
    std::printf ("Creating ADSR for pad %d\n", padId);
    std::string padStr = std::to_string (padId + 1);
}

ADSRComponent::~ADSRComponent()
{
}

void ADSRComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ADSRComponent::resized()
{
    const auto componentWidth = getWidth();
    const auto componentHeight = getHeight();
    const auto dialWidth = componentWidth * 0.1f;
    const auto dialHeight = componentHeight * 0.75f;
    const auto totalWidth = dialWidth * 4;
    const auto startX = (componentWidth - totalWidth) / 2.0f;
    const auto startY = (componentHeight - dialHeight) / 2.0f;

    attackSlider.setBounds (startX, startY, dialWidth, dialHeight);
    decaySlider.setBounds (startX + dialWidth, startY, dialWidth, dialHeight);
    sustainSlider.setBounds (startX + (dialWidth * 2), startY, dialWidth, dialHeight);
    releaseSlider.setBounds (startX + (dialWidth * 3), startY, dialWidth, dialHeight);
}