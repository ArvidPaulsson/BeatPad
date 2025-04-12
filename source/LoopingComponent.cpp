#include "LoopingComponent.h"

LoopingComponent::LoopingComponent (PluginProcessor& p, int padId) : processorRef (p), padId (padId)
{
    addAndMakeVisible (loopModeLabel);
    addAndMakeVisible (loopModeSelector);
    loopModeSelector.addItem ("Off", 1);
    loopModeSelector.addItem ("Forward", 2);
    loopModeSelector.addItem ("Ping-Pong", 3);
    loopModeSelector.onChange = [this] { loopModeSelectorChanged(); };
    loopModeSelector.setSelectedId (1);
}

LoopingComponent::~LoopingComponent()
{
}

void LoopingComponent::paint (juce::Graphics& g)
{
}

void LoopingComponent::resized()
{
    auto bounds = getLocalBounds().reduced (5);

    // Position loop mode controls
    loopModeLabel.setBounds (bounds.removeFromTop (20));
    loopModeSelector.setBounds (bounds.removeFromTop (25));

    bounds.removeFromTop (15); // Space between controls
}

void LoopingComponent::loopModeSelectorChanged()
{
    switch (loopModeSelector.getSelectedId())
    {
        case 1:
            loopMode = LoopingMode::NoLoop;
            break;
        case 2:
            loopMode = LoopingMode::Forward;
            break;
        case 3:
            loopMode = LoopingMode::PingPong;
            break;
        default:
            loopMode = LoopingMode::NoLoop;
            break;
    }
}

void LoopingComponent::drawLoopPointMarkers (juce::Graphics& g, int startX, int endX)
{
    g.setColour (juce::Colours::red);
    g.drawLine (startX, 0, startX, getHeight(), 2.0f);
    g.drawLine (endX, 0, endX, getHeight(), 2.0f);
}

void LoopingComponent::setLoopStart (float start)
{
    loopStartSlider.setValue (start);
}

void LoopingComponent::setLoopEnd (float end)
{
    loopEndSlider.setValue (end);
}
