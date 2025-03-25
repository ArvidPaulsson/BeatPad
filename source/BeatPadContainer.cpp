#include "BeatPadContainer.h"

BeatPadContainer::BeatPadContainer (PluginProcessor& p)
    : processorRef (p)
{
    for (int i = 0; i < nbrOfPads; i++)
    {
        // Weird, make safer by using a unique_ptr
        pads.add (new BeatPad (processorRef, i + 1));
        addAndMakeVisible (pads[i]);
    }
}

BeatPadContainer::~BeatPadContainer()
{
}

void BeatPadContainer::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::grey);
}

void BeatPadContainer::resized()
{
    auto area = getLocalBounds().reduced (10);
    int buttonWidth = area.getWidth() / 3;
    int buttonHeight = area.getHeight() / 3;

    // Arrange 9 pads in a 3x3 grid
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            int index = row * 3 + col;
            int x = area.getX() + col * buttonWidth;
            int y = area.getY() + row * buttonHeight;

            pads[index]->setBounds (x, y, buttonWidth - 2, buttonHeight - 2);
            // pads[index]->getTextButton().setBounds(x, y, buttonWidth - 2, buttonHeight - 2);

            // If using pads array instead
        }
    }
}

void BeatPadContainer::setMidiNotes (const std::array<int, 9>& notes)
{
}

void BeatPadContainer::setPadColors (juce::Colour normal, juce::Colour triggered)
{
}

BeatPad* BeatPadContainer::getPad (int index)
{
    return pads[index];
}
