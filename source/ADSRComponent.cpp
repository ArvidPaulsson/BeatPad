#include "ADSRComponent.h"

ADSRComponent::ADSRComponent (PluginProcessor& p, int padId) : processorRef (p), padId (padId)
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