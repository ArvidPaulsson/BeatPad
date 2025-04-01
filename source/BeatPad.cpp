#include "BeatPad.h"

BeatPad::BeatPad (PluginProcessor& p, int padId)
    : processorRef (p), padId (padId)
{
    setWantsKeyboardFocus (true);

    muteButton.setButtonText ("M");
    muteButton.setClickingTogglesState (true);
    addAndMakeVisible (muteButton);
    juce::String muteParamID = "MUTE_PAD_" + std::to_string (padId);
    juce::String soloParamID = "SOLO_PAD_" + std::to_string (padId);

    soloButton.setButtonText ("S");
    soloButton.setClickingTogglesState (true);
    addAndMakeVisible (soloButton);

    muteAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processorRef.getAPVST(), muteParamID, muteButton);

    soloAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processorRef.getAPVST(), soloParamID, soloButton);

    // Assign a default MIDI note based on the pad ID
    if (padMidiNotes.find (padId) != padMidiNotes.end())
    {
        midiNote = padMidiNotes.at (padId);
        std::printf ("Pad %d assigned MIDI note %d\n", padId, midiNote);
    }
}

BeatPad::~BeatPad()
{
}

void BeatPad::paint (juce::Graphics& g)
{
    bool isMuted = processorRef.isPadMuted (padId);
    bool isSoloed = processorRef.isPadSoloed (padId);

    juce::Colour basePadColour = juce::Colours::grey;
    juce::Colour triggeredColour = juce::Colours::red;
    juce::Colour mutedColour = juce::Colours::blue.withAlpha (0.6f);
    juce::Colour soloColour = juce::Colours::orange.withAlpha (0.6f);
    juce::Colour outlineColour = juce::Colours::grey;
    juce::Colour textColour = juce::Colours::white;

    if (isHighlighted)
    {
        outlineColour = juce::Colours::yellow;
    }
    else if (isTriggered)
    {
        outlineColour = triggeredColour.brighter();
    }
    else if (isSoloed)
    {
        outlineColour = soloColour.brighter();
        textColour = juce::Colours::black;
    }
    else if (isMuted)
    {
        outlineColour = mutedColour.brighter();
        textColour = juce::Colours::lightgrey;
    }

    juce::Colour background = basePadColour;
    if (isTriggered)
        background = triggeredColour.darker (0.3f);

    juce::ColourGradient gradient (background.darker (0.5f), 0.0f, 0.0f, background.brighter (0.2f), (float) getWidth(), (float) getHeight(), true);
    g.setFillType (gradient);
    float cornerSize = 6.0f;
    g.fillRoundedRectangle (getLocalBounds().toFloat(), cornerSize);

    if (isSoloed && isTriggered)
    {
        g.setColour (soloColour);
        g.fillRoundedRectangle (getLocalBounds().toFloat(), cornerSize);
    }
    else if (isMuted && isTriggered)
    {
        g.setColour (mutedColour);
        g.fillRoundedRectangle (getLocalBounds().toFloat(), cornerSize);
    }

    int padding = 1;
    g.setColour (outlineColour);
    g.drawRoundedRectangle (getLocalBounds().reduced (padding).toFloat(), cornerSize, 2.0f);

    if (sampleLoaded && !beatPadFileName.isEmpty())
    {
        auto filename = juce::File (beatPadFileName).getFileName();

        g.setColour (textColour);
        g.setFont (juce::Font (12.0f));

        // Define area for text, slightly padded, avoiding button areas
        auto textBounds = getLocalBounds().reduced (5); // General padding
        textBounds.removeFromBottom (25); // Remove space for buttons at the bottom

        // Draw text centered in the adjusted area
        g.drawFittedText (filename, textBounds, juce::Justification::centred, 1); // Max 1 line
    }
    else
    {
        // Optional: Draw placeholder text if no sample is loaded
        g.setColour (juce::Colours::lightgrey.withAlpha (0.5f)); // Dim placeholder
        g.setFont (juce::Font (10.0f));
        g.drawFittedText ("Drop Sample", getLocalBounds().reduced (5), juce::Justification::centred, 1);
    }
}

void BeatPad::resized()
{
    const int buttonSize = 25; // Adjust size as needed
    const int margin = 4; // Margin from corners

    auto bounds = getLocalBounds();

    soloButton.setBounds (margin, bounds.getHeight() - buttonSize - margin, buttonSize, buttonSize);

    muteButton.setBounds (bounds.getWidth() - buttonSize - margin, bounds.getHeight() - buttonSize - margin, buttonSize, buttonSize);
}

void BeatPad::mouseDown (const juce::MouseEvent& event)
{
    if (sampleLoaded)
    {
        isTriggered = true;
        processorRef.addMidiMessage (juce::MidiMessage::noteOn (1, midiNote, 1.0f));
        if (onSelected)
            onSelected (padId);
    }
    repaint();
}

void BeatPad::mouseUp (const juce::MouseEvent& event)
{
    if (sampleLoaded)
    {
        isTriggered = false;
        processorRef.addMidiMessage (juce::MidiMessage::noteOff (1, midiNote));
    }
    repaint();
}

bool BeatPad::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto file : files)
    {
        if (file.contains (".wav") || file.contains (".mp3") || file.contains (".aiff"))
        {
            return true;
        }
    }
    return false;
}

void BeatPad::filesDropped (const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused (x, y);
    for (auto file : files)
    {
        if (isInterestedInFileDrag (file))
        {
            auto padFile = std::make_unique<juce::File> (file);
            beatPadFileName = padFile->getFullPathName();
            processorRef.loadFile (beatPadFileName, midiNote);
            sampleLoaded = true;
            printf ("File loaded on Pad %d with MIDI Note %d: %s\n", padId, midiNote, file.toRawUTF8());
        }
    }
    if (sampleLoaded)
    {
        if (onSelected)
            onSelected (padId);
        repaint();
        // Should add here update to processorRef so that it can update the waveform
    }
}

void BeatPad::setHighlight (bool shouldHighlight)
{
    if (isHighlighted != shouldHighlight)
    {
        isHighlighted = shouldHighlight;
        repaint();
    }
}