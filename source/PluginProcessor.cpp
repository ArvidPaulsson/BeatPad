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
              ),
      mAPVTS (*(this), nullptr, "PARAMETERS", createParameters()),
      mParamUpdateFlags (numVoices)
{
    for (int i = 0; i < numVoices; i++)
    {
        mParamUpdateFlags[i].store (false);
    }

    registerParameters();
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
            int midiNote = m.getNoteNumber();
            currentMidiNote.store (midiNote);
            auto it = mSampleWaveforms.find (midiNote);
            if (it != mSampleWaveforms.end())
            {
                currentWaveForm = *it->second;
            }
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
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
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
    auto padWaveform = std::make_unique<juce::AudioBuffer<float>> (1, sampleLength);

    mReader->read (padWaveform.get(), 0, sampleLength, 0, true, false);

    mSampleWaveforms[midiNote] = std::move (padWaveform);

    juce::BigInteger midiNoteRange;
    midiNoteRange.setRange (0, 128, false);
    midiNoteRange.setBit (midiNote, true);

    mSampler.addSound (new juce::SamplerSound (
        "Sample",
        *mReader,
        midiNoteRange,
        midiNote,
        0.0,
        0.1,
        10.0));

    std::printf ("Sound added to sampler. Total sounds: %d\n", mSampler.getNumSounds());
}

void PluginProcessor::addMidiMessage (const juce::MidiMessage& message)
{
    std::printf ("Received MIDI message: %s\n", message.getDescription().toRawUTF8());
    midiMessageQueue.addEvent (message, 0);
}

juce::AudioBuffer<float>* PluginProcessor::getWaveformForNote (int midiNote)
{
    auto it = mSampleWaveforms.find (midiNote);
    return (it != mSampleWaveforms.end()) ? it->second.get() : &waveForm;
}

juce::AudioBuffer<float>* PluginProcessor::getCurrentWaveForm()
{
    int note = currentMidiNote.load();
    if (note == -1)
    {
        return nullptr;
    }
    auto it = mSampleWaveforms.find (note);
    return (it != mSampleWaveforms.end()) ? it->second.get() : &waveForm;
}

int PluginProcessor::getCurrentMidiNote()
{
    return currentMidiNote.load();
}

void PluginProcessor::registerParameters()
{
    for (int i = 0; i < 9; i++)
    {
        auto padId = i + 1;
        auto padIdStr = std::to_string (padId);
        auto attackId = "ATTACK_PAD_" + padIdStr;
        auto decayId = "DECAY_PAD_" + padIdStr;
        auto sustainId = "SUSTAIN_PAD_" + padIdStr;
        auto releaseId = "RELEASE_PAD_" + padIdStr;

        mAPVTS.addParameterListener (attackId, this);
        mAPVTS.addParameterListener (decayId, this);
        mAPVTS.addParameterListener (sustainId, this);
        mAPVTS.addParameterListener (releaseId, this);
    }
}

void PluginProcessor::updateADSR (int padId)
{
    std::string padStr = std::to_string (padId);
    auto adsrParameters = mADSRParams[padId];

    adsrParameters.attack = mAPVTS.getRawParameterValue ("ATTACK_PAD_" + padStr)->load();
    adsrParameters.decay = mAPVTS.getRawParameterValue ("DECAY_PAD_" + padStr)->load();
    adsrParameters.sustain = mAPVTS.getRawParameterValue ("SUSTAIN_PAD_" + padStr)->load();
    adsrParameters.release = mAPVTS.getRawParameterValue ("RELEASE_PAD_" + padStr)->load();

    if (padId < mSampler.getNumSounds())
    {
        if (auto sound = dynamic_cast<juce::SamplerSound*> (mSampler.getSound (padId).get()))
        {
            sound->setEnvelopeParameters (adsrParameters);
        }
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    for (int i = 0; i < 9; i++)
    {
        auto padId = i + 1;
        auto padIdStr = std::to_string (padId);
        auto attackId = "ATTACK_PAD_" + padIdStr;
        auto decayId = "DECAY_PAD_" + padIdStr;
        auto sustainId = "SUSTAIN_PAD_" + padIdStr;
        auto releaseId = "RELEASE_PAD_" + padIdStr;

        params.push_back (std::make_unique<juce::AudioParameterFloat> (attackId, attackId, 0.0f, 5.0f, 0.1f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (decayId, decayId, 0.0f, 5.0f, 0.1f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (sustainId, sustainId, 0.0f, 5.0f, 0.1f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (releaseId, releaseId, 0.0f, 5.0f, 0.1f));
    }

    return { params.begin(), params.end() };
}

void PluginProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    std::string idStr = parameterID.toStdString();
    size_t lastUnderscore = idStr.find_last_of ('_');

    if (lastUnderscore != std::string::npos && lastUnderscore > 0)
    {
        // Check if it's one of the ADSR parameters we care about
        if (idStr.find ("ATTACK_PAD_") == 0 || idStr.find ("DECAY_PAD_") == 0 || idStr.find ("SUSTAIN_PAD_") == 0 || idStr.find ("RELEASE_PAD_") == 0)
        {
            std::string padNumStr = idStr.substr (lastUnderscore + 1);
            try
            {
                int padId = std::stoi (padNumStr);
                if (padId >= 1 && padId <= numVoices)
                {
                    // Just set the flag to true for this pad
                    mParamUpdateFlags[padId - 1].store (true, std::memory_order_relaxed);
                    // printf("Flagging Pad %d for ADSR update.\n", padId);
                }
            } catch (const std::exception& e)
            { // Catch base exception
                // std::cerr << "Error parsing pad ID from parameter: " << parameterID.toStdString() << ": " << e.what() << std::endl;
            }
        }
    }
}

void PluginProcessor::updateADSRForPadOnAudioThread (int padId)
{
    juce::ignoreUnused (padId);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
