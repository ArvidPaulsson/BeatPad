#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), waveThumbnail (p), beatPadContainer (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // addAndMakeVisible (beatPad);
    addAndMakeVisible (waveThumbnail);
    addAndMakeVisible (beatPadContainer);

    // Because the BeatPadContainer creates the pads, which in their turn create the ADSR components, we need to be able to access the adsr components and set their visibility when a pad is triggered

    setResizable (true, true);
    setResizeLimits (600, 400, 1918, 1050);

    const float ratio = 1.83f;
    getConstrainer()->setFixedAspectRatio (ratio);
    setSize (800, 600);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Top half: Wave thumbnail
    auto topSection = area.removeFromTop (area.getHeight() * 0.5);
    waveThumbnail.setBounds (topSection);
    auto beatPadSection = area.removeFromLeft (area.getWidth());
    beatPadContainer.setBounds (beatPadSection);
}
