#include "WaveThumbnail.h"

WaveThumbnail::WaveThumbnail (PluginProcessor& p)
    : processorRef (p)
{
}

WaveThumbnail::~WaveThumbnail()
{
}

void WaveThumbnail::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::cadetblue.darker());
    if (processorRef.getCurrentMidiNote() != -1)
    {
        drawThumbnail (g, *processorRef.getCurrentWaveForm());
    }
    else
    {
        g.setColour (juce::Colours::white);
        g.setFont (15.0f);
        g.drawFittedText ("No sample loaded", getLocalBounds(), juce::Justification::centred, 1);
    }
    repaint();
}

void WaveThumbnail::drawThumbnail (juce::Graphics& g, juce::AudioBuffer<float>& waveform)
{
    g.fillAll (juce::Colours::cadetblue.darker());

    if (waveform.getNumSamples() > 0)
    {
        juce::Path p;
        audioPoints.clear();
        auto ratio = waveform.getNumSamples() / getWidth();
        auto buffer = waveform.getReadPointer (0);

        for (int i = 0; i < waveform.getNumSamples(); i += ratio)
        {
            audioPoints.push_back (buffer[i]);
        }

        p.startNewSubPath (0, getHeight() / 2);

        for (int i = 0; i < audioPoints.size(); ++i)
        {
            auto point = juce::jmap<float> (audioPoints[i], -1.0f, 1.0f, getHeight(), 0);
            p.lineTo (i, point);
        }

        g.strokePath (p, juce::PathStrokeType (2));
        g.setColour (juce::Colours::white);
        g.setFont (15.0f);
        auto textBounds = getLocalBounds().reduced (10, 10);
        // g.drawFittedText (mFileName, textBounds, juce::Justification::topRight, 1);

        auto playHeadPosition = juce::jmap<int> (processorRef.getSampleCount().load(), 0, processorRef.getCurrentWaveForm()->getNumSamples(), 0, getWidth());

        g.setColour (juce::Colours::white);
        g.drawLine (playHeadPosition, 0, playHeadPosition, getHeight(), 2.0f);
    }
}

void WaveThumbnail::resized()
{
}