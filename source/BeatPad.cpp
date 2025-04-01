#include "BeatPad.h"

BeatPad::BeatPad (PluginProcessor& p, int padId)
    : processorRef (p), padId (padId)
{
    setWantsKeyboardFocus (true);

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
    juce::Colour backgroundColor = juce::Colours::black;
    juce::Colour activeColor = juce::Colours::red;
    juce::Colour inactiveColor = juce::Colours::darkgrey;
    juce::Colour outlineColor = isTriggered ? activeColor.brighter() : juce::Colours::grey;

    juce::ColourGradient gradient;

    gradient = juce::ColourGradient (backgroundColor, 0.0f, 0.0f, inactiveColor.darker (0.7f), getWidth(), getHeight(), true);

    g.setFillType (gradient);
    g.fillRect (getLocalBounds());

    int padding = 2;
    juce::Rectangle<int> innerRect = getLocalBounds().reduced (padding);

    g.setColour (outlineColor);
    float cornerSize = 6.0f;
    g.drawRoundedRectangle (padding / 2.0f, padding / 2.0f, getWidth() - padding, getHeight() - padding, cornerSize, 2.0f);
}

void BeatPad::resized()
{
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
    for (auto file : files)
    {
        if (isInterestedInFileDrag (file))
        {
            auto padFile = std::make_unique<juce::File> (file);
            beatPadFileName = padFile->getFullPathName();
            // processorRef.loadFile (beatPadFileName);
            processorRef.loadFile (beatPadFileName, midiNote);
            sampleLoaded = true;
            printf ("File loaded on Pad %d with MIDI Note %d: %s\n", padId, midiNote, file.toRawUTF8());
        }
    }
    repaint();
}