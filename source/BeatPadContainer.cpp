#include "BeatPadContainer.h"

BeatPadContainer::BeatPadContainer (PluginProcessor& p)
    : processorRef (p)
{
    for (int i = 0; i < nbrOfPads; i++)
    {
        auto pad = std::make_unique<BeatPad> (processorRef, i + 1);
        // auto adsrComponent = std::make_unique<ADSRComponent> (processorRef, i + 1);
        auto fxTab = std::make_unique<FXTab> (processorRef, i + 1);
        addAndMakeVisible (pad.get());
        addAndMakeVisible (fxTab.get());
        // addAndMakeVisible (adsrComponent.get());
        // adsrComponent.get()->setVisible (false);
        fxTab.get()->setVisible (false);
        // adsrComponents.push_back (std::move (adsrComponent));
        fxTabs.push_back (std::move (fxTab));
        // pad->onSelected = [this] (int padId) { updateADSRVisibility (padId); };
        pad->onSelected = [this] (int padId) { updateFXTabVisibility (padId); };
        pads.push_back (std::move (pad));
    }
}

BeatPadContainer::~BeatPadContainer()
{
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
        // auto adsrComponent = getADSRComponent (currentPadId);
        auto fxTab = getFXTab (currentPadId);
        std::printf ("Current pad ID: %d\n", currentPadId);
        if (fxTab)
        {
            fxTab->setBounds (area);
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

FXTab* BeatPadContainer::getFXTab (int index)
{
    if (index > 0 && index <= fxTabs.size())
    {
        std::printf ("FX Tab found\n");
        return fxTabs[index - 1].get();
    }
    return nullptr;
}

void BeatPadContainer::updateFXTabVisibility (int padId)
{
    // Hide previously visible ADSR component if it exists
    if (currentPadId != -1 && currentPadId != padId)
    {
        auto previousFXTab = getFXTab (currentPadId);
        if (previousFXTab)
        {
            previousFXTab->setVisible (false);
        }
    }

    auto newFXTab = getFXTab (padId);
    if (newFXTab)
    {
        newFXTab->setVisible (true);
        currentPadId = padId;
        for (int i = 0; i < pads.size(); ++i)
        {
            if (pads[i] != nullptr)
            {
                pads[i]->setHighlight ((i + 1) == padId);
            }
        }
        // -----------------------------

        resized();
        repaint();
    }
    else
    {
        currentPadId = -1;
        for (int i = 0; i < pads.size(); ++i)
        {
            if (pads[i] != nullptr)
                pads[i]->setHighlight (false);
        }
    }
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
    // Hide previously visible ADSR component if it exists
    if (currentPadId != -1 && currentPadId != padId)
    {
        auto previousAdsrComponent = getADSRComponent (currentPadId);
        if (previousAdsrComponent)
        {
            previousAdsrComponent->setVisible (false);
        }
    }

    auto newAdsrComponent = getADSRComponent (padId);
    if (newAdsrComponent)
    {
        newAdsrComponent->setVisible (true);
        currentPadId = padId;
        for (int i = 0; i < pads.size(); ++i)
        {
            if (pads[i] != nullptr)
            {
                pads[i]->setHighlight ((i + 1) == padId);
            }
        }
        // -----------------------------

        resized();
        repaint();
    }
    else
    {
        currentPadId = -1;
        for (int i = 0; i < pads.size(); ++i)
        {
            if (pads[i] != nullptr)
                pads[i]->setHighlight (false);
        }
    }
}
