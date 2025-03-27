#include "BeatPadContainer.h"

BeatPadContainer::BeatPadContainer (PluginProcessor& p)
    : processorRef (p)
{
    for (int i = 0; i < nbrOfPads; i++)
    {
        // Weird, make safer by using a unique_ptr
        // pads.add (new BeatPad (processorRef, i + 1));
        auto pad = std::make_unique<BeatPad> (processorRef, i + 1);
        auto adsrComponent = std::make_unique<ADSRComponent> (processorRef, i + 1);
        addAndMakeVisible (pad.get());
        addAndMakeVisible (adsrComponent.get());
        adsrComponent.get()->setVisible (false);
        adsrComponents.push_back (std::move (adsrComponent));
        pad->onSelected = [this] (int padId) { updateADSRVisibility (padId); };
        pads.push_back (std::move (pad));
    }
}

BeatPadContainer::~BeatPadContainer()
{
    // clear the pads array
    pads.clear();
    adsrComponents.clear();
}

void BeatPadContainer::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::grey);
}

void BeatPadContainer::resized()
{
    auto area = getLocalBounds();
    auto beatPadArea = area.removeFromLeft (area.getWidth() * 0.5);
    int buttonWidth = beatPadArea.getWidth() / 3;
    int buttonHeight = beatPadArea.getHeight() / 3;

    // Arrange 9 pads in a 3x3 grid
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            int index = row * 3 + col;
            int x = beatPadArea.getX() + col * buttonWidth;
            int y = beatPadArea.getY() + row * buttonHeight;

            pads[index]->setBounds (x, y, buttonWidth - 2, buttonHeight - 2);
        }
    }
    if (currentPadId != -1)
    {
        auto adsrComponent = getADSRComponent (currentPadId);
        std::printf ("Current pad ID: %d\n", currentPadId);
        if (adsrComponent)
        {
            adsrComponent->setBounds (area);
        }
    }
}

void BeatPadContainer::setMidiNotes (const std::array<int, 9>& notes)
{
}

void BeatPadContainer::setPadColors (juce::Colour normal, juce::Colour triggered)
{
}

BeatPad BeatPadContainer::getPad (int index)
{
}

ADSRComponent* BeatPadContainer::getADSRComponent (int index)
{
    if (index > 0 && index <= adsrComponents.size())
    {
        std::printf ("ADSR component found\n");
        return adsrComponents[index - 1].get();
    }
    return nullptr;
}

void BeatPadContainer::updateADSRVisibility (int padId)
{
    std::printf ("Pad ID: %d\n", padId);
    if (padId != -1)
    {
        auto adsrComponent = getADSRComponent (padId);
        if (adsrComponent)
        {
            adsrComponent->setVisible (true);
            currentPadId = padId;
        }
    }
    // repaint();
    resized();
}
