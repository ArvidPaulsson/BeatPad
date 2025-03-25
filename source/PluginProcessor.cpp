#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
    : AudioProcessor (BusesProperties()
#if !JucePlugin_IsMidiEffect
    #if !JucePlugin_IsSynth
              .withInput ("Input", juce::AudioChannelSet::stereo(), true)
    #endif
              .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
{
    mFormatManager.registerBasicFormats();
    for (int i = 0; i < numVoices; i++)
    {
        mSampler.addVoice (new juce::SamplerVoice());
    }
}

PluginProcessor::~PluginProcessor()
{
    mReader = nullptr;
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return "BeatPad";
}

bool PluginProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool PluginProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1; // NB: some hosts don't cope very well if you tell them there are 0 programs,
        // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampler.setCurrentPlaybackSampleRate (sampleRate);
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    #if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
#endif
}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    // juce::ignoreUnused (midiMessages);

    midiMessages.addEvents (midiMessageQueue, 0, buffer.getNumSamples(), 0);
    midiMessageQueue.clear();

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    juce::MidiMessage m;
    juce::MidiBuffer::Iterator it { midiMessages };
    int sample;

    while (it.getNextEvent (m, sample))
    {
        if (m.isNoteOn())
        {
            mIsNotePlayed = true;
            std::printf ("Note played: %d, with velocity %d\n", m.getNoteNumber(), m.getVelocity());
        }
        else if (m.isNoteOff())
        {
            mIsNotePlayed = false;
            std::printf ("Note off: %d\n", m.getNoteNumber());
        }
    }

    mSampleCount = mIsNotePlayed ? mSampleCount += buffer.getNumSamples() : 0;

    mSampler.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

void PluginProcessor::loadFile (const juce::String& path)
{
    mSampler.clearSounds();

    auto file = juce::File (path);

    // std::unique_ptr<juce::AudioFormatReader> mReader (mFormatManager.createReaderFor (file));
    mReader = mFormatManager.createReaderFor (file);

    std::printf ("File loaded successfully: %s\n", path.toRawUTF8());
    std::printf ("Sample length: %lld\n", mReader->lengthInSamples);
    std::printf ("Sample rate: %.2f\n", mReader->sampleRate);

    auto sampleLength = static_cast<int> (mReader->lengthInSamples);
    waveForm.setSize (1, sampleLength);
    mReader->read (&waveForm, 0, sampleLength, 0, true, false);

    juce::BigInteger midiNoteRange;
    midiNoteRange.setRange (0, 128, true); // Clear all notes

    mSampler.addSound (new juce::SamplerSound ("Sample", *mReader, midiNoteRange, 60, 0.1, 0.1, 10));

    std::printf ("Sound added to sampler. Total sounds: %d\n", mSampler.getNumSounds());
}

void PluginProcessor::loadFile (const juce::String& path, int midiNote)
{
    for (int i = 0; i < mSampler.getNumSounds(); ++i)
    {
        juce::SamplerSound* sound = dynamic_cast<juce::SamplerSound*> (mSampler.getSound (i).get());
        if (sound)
        {
            // Remove any existing sound mapped to this MIDI note
            if (sound->appliesToNote (midiNote))
            {
                mSampler.removeSound (i);
                break;
            }
        }
    }

    auto file = juce::File (path);
    mReader = mFormatManager.createReaderFor (file);

    if (!mReader)
    {
        std::printf ("Error: Could not load file %s\n", path.toRawUTF8());
        return;
    }

    std::printf ("File loaded successfully: %s\n", path.toRawUTF8());
    std::printf ("Sample length: %lld\n", mReader->lengthInSamples);
    std::printf ("Sample rate: %.2f\n", mReader->sampleRate);
    std::printf ("Mapped to MIDI Note: %d\n", midiNote);

    auto sampleLength = static_cast<int> (mReader->lengthInSamples);
    auto* padWaveform = new juce::AudioBuffer<float>();
    padWaveform->setSize (1, sampleLength);
    mSampleWaveforms.add (padWaveform);
    mReader->read (padWaveform, 0, sampleLength, 0, true, false);

    juce::BigInteger midiNoteRange;
    midiNoteRange.setRange (0, 128, false);
    midiNoteRange.setBit (midiNote, true);

    mSampler.addSound (new juce::SamplerSound (
        "Sample",
        *mReader,
        midiNoteRange,
        midiNote, // Root note
        0.0, // Attack time
        0.1, // Release time
        10.0 // Maximum sample length
        ));

    std::printf ("Sound added to sampler. Total sounds: %d\n", mSampler.getNumSounds());
}

void PluginProcessor::addMidiMessage (const juce::MidiMessage& message)
{
    std::printf ("Received MIDI message: %s\n", message.getDescription().toRawUTF8());
    midiMessageQueue.addEvent (message, 0);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
