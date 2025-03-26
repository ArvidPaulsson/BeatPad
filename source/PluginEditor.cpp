#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), waveThumbnail (p), beatPadContainer (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // addAndMakeVisible (beatPad);
    addAndMakeVisible (waveThumbnail);
    addAndMakeVisible (beatPadContainer);

    adsrComponent = std::make_unique<ADSRComponent> (p, -1);
    addAndMakeVisible (adsrComponent.get());
    adsrComponent->setVisible (false);

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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto area = getLocalBounds();

    // Top half: Wave thumbnail
    auto topSection = area.removeFromTop (area.getHeight() * 0.5);
    waveThumbnail.setBounds (topSection);
    auto beatPadSection = area.removeFromLeft (area.getWidth() * 0.5);
    beatPadContainer.setBounds (beatPadSection);
}

void PluginEditor::updateADSRVisibility (int padId)
{
    activePadId = padId;

    if (padId != -1)
    {
        adsrComponent->setVisible (true);
        adsrComponent->setBounds (getLocalBounds());
    }
    else
    {
        adsrComponent->setVisible (false);
    }
    repaint();
}
