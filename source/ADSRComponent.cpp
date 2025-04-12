#include "ADSRComponent.h"

ADSRComponent::ADSRComponent (PluginProcessor& p, int padId)
    : processorRef (p), padId (padId)
{
    std::printf ("Creating ADSR for pad %d\n", padId);
    std::string padStr = std::to_string (padId);

    attackSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 40, 20);
    addAndMakeVisible (attackSlider);

    attackLabel.setFont (10.0f);
    attackLabel.setText ("Attack", juce::NotificationType::dontSendNotification);
    attackLabel.setJustificationType (juce::Justification::centredTop);
    attackLabel.attachToComponent (&attackSlider, false);

    mAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.getAPVST(), "ATTACK_PAD_" + padStr, attackSlider);

    decaySlider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    decaySlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 40, 20);
    decaySlider.setRange (0.0f, 5.0f, 0.01f);
    addAndMakeVisible (decaySlider);

    decayLabel.setFont (10.0f);
    decayLabel.setText ("Decay", juce::NotificationType::dontSendNotification);
    decayLabel.setJustificationType (juce::Justification::centredTop);
    decayLabel.attachToComponent (&decaySlider, false);

    mDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.getAPVST(), "DECAY_PAD_" + padStr, decaySlider);

    sustainSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    sustainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 40, 20);
    sustainSlider.setRange (0.0f, 5.0f, 0.01f);
    addAndMakeVisible (sustainSlider);

    sustainLabel.setFont (10.0f);
    sustainLabel.setText ("Sustain", juce::NotificationType::dontSendNotification);
    sustainLabel.setJustificationType (juce::Justification::centredTop);
    sustainLabel.attachToComponent (&sustainSlider, false);

    mSustanAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.getAPVST(), "SUSTAIN_PAD_" + padStr, sustainSlider);

    releaseSlider.setSliderStyle (juce::Slider::SliderStyle::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 40, 20);
    releaseSlider.setRange (0.0f, 5.0f, 0.01f);
    addAndMakeVisible (releaseSlider);

    releaseLabel.setFont (10.0f);
    releaseLabel.setText ("Release", juce::NotificationType::dontSendNotification);
    releaseLabel.setJustificationType (juce::Justification::centredTop);
    releaseLabel.attachToComponent (&releaseSlider, false);

    mReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (processorRef.getAPVST(), "RELEASE_PAD_" + padStr, releaseSlider);
}

ADSRComponent::~ADSRComponent()
{
}

void ADSRComponent::paint (juce::Graphics& g)
{
    auto backgroundColour = getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId);
    auto bounds = getLocalBounds().toFloat();
    float cornerSize = 6.0f; // Adjust corner rounding

    // Draw a slightly darker, rounded background
    g.setColour (backgroundColour.darker (0.4f));
    g.fillRoundedRectangle (bounds, cornerSize);

    // Draw a subtle outline (optional)
    g.setColour (backgroundColour.brighter (0.2f));
    g.drawRoundedRectangle (bounds.reduced (0.5f), cornerSize, 1.0f);
}

void ADSRComponent::resized()
{
    const int kMargin = 10;
    const int kSliderSpacing = 8;
    const int kNumSliders = 4;

    const int kSpaceBelowSliders = 25;

    auto bounds = getLocalBounds();

    bounds.reduce (kMargin, kMargin);

    int totalSpacing = kSliderSpacing * (kNumSliders - 1);
    int availableWidth = bounds.getWidth() - totalSpacing;
    int sliderWidth = availableWidth / kNumSliders;

    int sliderHeight = bounds.getHeight() - kSpaceBelowSliders;
    sliderHeight = juce::jmax (10, sliderHeight);

    int sliderY = bounds.getY();

    int currentX = bounds.getX();

    int dialSize = juce::jmin (sliderWidth, sliderHeight);
    sliderY += (sliderHeight - dialSize) / 2;

    attackSlider.setBounds (currentX, sliderY, dialSize, dialSize);
    currentX += sliderWidth + kSliderSpacing;
    decaySlider.setBounds (currentX, sliderY, dialSize, dialSize);
    currentX += sliderWidth + kSliderSpacing;
    sustainSlider.setBounds (currentX, sliderY, dialSize, dialSize);
    currentX += sliderWidth + kSliderSpacing;
    releaseSlider.setBounds (currentX, sliderY, dialSize, dialSize);
}