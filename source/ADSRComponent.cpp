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
    const int kMargin = 10; // Margin around the edges
    const int kSliderSpacing = 8; // Space between sliders horizontally
    const int kNumSliders = 4;
    // Increase this value to push labels further down by making sliders shorter
    const int kSpaceBelowSliders = 25; // << ADJUST THIS VALUE (pixels)

    auto bounds = getLocalBounds(); // Get total area

    // Reduce bounds by margin (on all sides)
    bounds.reduce (kMargin, kMargin);

    // Calculate available width for sliders (total width - spacing between them)
    int totalSpacing = kSliderSpacing * (kNumSliders - 1);
    int availableWidth = bounds.getWidth() - totalSpacing;
    int sliderWidth = availableWidth / kNumSliders;

    // Calculate slider height, ensuring space is left below them
    int sliderHeight = bounds.getHeight() - kSpaceBelowSliders;
    // Ensure height is not negative if component is too small
    sliderHeight = juce::jmax (10, sliderHeight); // Minimum height of 10 pixels

    // Calculate starting Y position for sliders (at the top of the reduced bounds)
    int sliderY = bounds.getY();

    // Calculate starting X position for the first slider
    int currentX = bounds.getX();

    // Set bounds for each slider
    // Using sliderWidth for both width and height to aim for square-ish dials
    int dialSize = juce::jmin (sliderWidth, sliderHeight);
    // Recenter vertically if making them square
    sliderY += (sliderHeight - dialSize) / 2;

    attackSlider.setBounds (currentX, sliderY, dialSize, dialSize);
    currentX += sliderWidth + kSliderSpacing; // Still advance by original width slot + spacing
    decaySlider.setBounds (currentX, sliderY, dialSize, dialSize);
    currentX += sliderWidth + kSliderSpacing;
    sustainSlider.setBounds (currentX, sliderY, dialSize, dialSize);
    currentX += sliderWidth + kSliderSpacing;
    releaseSlider.setBounds (currentX, sliderY, dialSize, dialSize);
}