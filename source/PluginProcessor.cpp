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

/* void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
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
} */

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamplesInBlock = buffer.getNumSamples(); // Store for frequent use

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, numSamplesInBlock);

    // --- Apply Parameter Updates ---
    for (int padIdx = 0; padIdx < numVoices; ++padIdx)
    {
        if (mParamUpdateFlags[padIdx].exchange (false, std::memory_order_relaxed))
        {
            updateADSRForPadOnAudioThread (padIdx + 1);
        }
    }
    // -----------------------------

    midiMessages.addEvents (midiMessageQueue, 0, numSamplesInBlock, 0);
    midiMessageQueue.clear();

    // --- Update mCurrentMidiNoteForDisplay based on LAST Note On in block ---
    int lastNoteOnThisBlock = -1;
    for (const auto metadata : midiMessages)
    {
        if (metadata.getMessage().isNoteOn())
        {
            lastNoteOnThisBlock = metadata.getMessage().getNoteNumber();
        }
    }
    if (lastNoteOnThisBlock != -1)
    {
        mCurrentMidiNoteForDisplay.store (lastNoteOnThisBlock, std::memory_order_relaxed);
    }
    // -----------------------------------------------------------------------

    // --- Update Sample Count for Playhead ---
    int displayNote = mCurrentMidiNoteForDisplay.load (std::memory_order_relaxed);

    if (displayNote != mCurrentPlayingNoteForSampleCount)
    {
        mSampleCount.store (0, std::memory_order_relaxed); // Reset counter for the new note
        mCurrentPlayingNoteForSampleCount = displayNote; // Track the new note
    }

    if (mCurrentPlayingNoteForSampleCount != -1)
    {
        bool isTrackedNotePlaying = false;
        for (int i = 0; i < mSampler.getNumVoices(); ++i)
        {
            if (auto* voice = dynamic_cast<juce::SamplerVoice*> (mSampler.getVoice (i)))
            {
                // Check if the voice is playing the note we care about
                if (voice->isKeyDown() && voice->getCurrentlyPlayingNote() == mCurrentPlayingNoteForSampleCount)
                {
                    isTrackedNotePlaying = true;
                    break;
                }
            }
        }

        if (isTrackedNotePlaying)
        {
            // --- Get the waveform for the tracked note ---
            juce::AudioBuffer<float>* waveform = getWaveformForNote (mCurrentPlayingNoteForSampleCount);

            if (waveform != nullptr && waveform->getNumSamples() > 0)
            {
                int numSamplesInWaveform = waveform->getNumSamples();
                int currentCount = mSampleCount.load (std::memory_order_relaxed);

                // --- **FIX**: Only increment if current count is less than total samples ---
                if (currentCount < numSamplesInWaveform)
                {
                    int nextSampleCount = currentCount + numSamplesInBlock;

                    // Cap the count at the waveform length
                    if (nextSampleCount >= numSamplesInWaveform)
                    {
                        mSampleCount.store (numSamplesInWaveform, std::memory_order_relaxed);
                    }
                    else
                    {
                        mSampleCount.store (nextSampleCount, std::memory_order_relaxed);
                    }
                }
                // --- End Fix ---
                // If currentCount is already >= numSamplesInWaveform, do nothing.
            }
            else
            {
                // Waveform doesn't exist for the tracked note, reset count (safety)
                if (mSampleCount.load() != 0)
                    mSampleCount.store (0, std::memory_order_relaxed);
            }
        }
        else // The tracked note is no longer "KeyDown" according to the sampler voice
        {
            // Reset the count and stop tracking this note for the counter
            if (mSampleCount.load() != 0)
                mSampleCount.store (0, std::memory_order_relaxed);
            mCurrentPlayingNoteForSampleCount = -1;
        }
    }
    else // Not tracking any note (-1)
    {
        if (mSampleCount.load() != 0)
            mSampleCount.store (0, std::memory_order_relaxed);
    }
    // --- End Sample Count Update ---

    // --- Render Audio ---
    mSampler.renderNextBlock (buffer, midiMessages, 0, numSamplesInBlock);
    // --------------------

    // --- Optional: Reset display note if no voices are playing at all ---
    bool anyVoicePlaying = false;
    for (int i = 0; i < mSampler.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<juce::SamplerVoice*> (mSampler.getVoice (i)))
        {
            if (voice->isVoiceActive() && voice->isKeyDown())
            { // isPlaying includes release phase
                anyVoicePlaying = true;
                break;
            }
        }
    }
    if (!anyVoicePlaying && mCurrentMidiNoteForDisplay.load() != -1)
    {
        // mCurrentMidiNoteForDisplay.store (-1, std::memory_order_relaxed);
        // Ensure count is also reset if missed above
        if (mCurrentPlayingNoteForSampleCount != -1 || mSampleCount.load() != 0)
        {
            mSampleCount.store (0);
            mCurrentPlayingNoteForSampleCount = -1;
        }
    }
    // --------------------------------------------------------------------
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
    // auto state = mAPVTS.copyState();
    // std::unique_ptr<juce::XmlElement> xml (state.createXml());
    // copyXmlToBinary (*xml, destData);
    // printf ("State information saved.\n");
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (mAPVTS.state.getType()))
        {
            mAPVTS.replaceState (juce::ValueTree::fromXml (*xmlState));
            printf ("State information loaded.\n");

            for (int padIdx = 0; padIdx < numVoices; ++padIdx)
            {
                mParamUpdateFlags[padIdx].store (true, std::memory_order_relaxed);
            }
            printf ("Flagged all pads for ADSR update after loading state.\n");
        }
        else
        {
            printf ("Error loading state: XML tag name mismatch.\n");
        }
    }
    else
    {
        printf ("Error loading state: Could not parse XML from binary data.\n");
    }
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
    int note = mCurrentMidiNoteForDisplay.load();
    if (note == -1)
    {
        return nullptr;
    }
    auto it = mSampleWaveforms.find (note);
    return (it != mSampleWaveforms.end()) ? it->second.get() : &waveForm;
}

int PluginProcessor::getCurrentMidiNote()
{
    // return currentMidiNote.load()
    return mCurrentMidiNoteForDisplay.load();
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

    juce::NormalisableRange<float> attackRange (0.0f, 5.0f, 0.001f, 0.3f);
    juce::NormalisableRange<float> decayRange (0.0f, 5.0f, 0.001f, 0.3f);
    juce::NormalisableRange<float> sustainRange (0.0f, 1.0f, 0.001f);
    juce::NormalisableRange<float> releaseRange (0.002f, 5.0f, 0.001f, 0.3f);

    for (int i = 0; i < 9; i++)
    {
        auto padId = i + 1;
        auto padIdStr = std::to_string (padId);
        auto attackId = "ATTACK_PAD_" + padIdStr;
        auto decayId = "DECAY_PAD_" + padIdStr;
        auto sustainId = "SUSTAIN_PAD_" + padIdStr;
        auto releaseId = "RELEASE_PAD_" + padIdStr;

        params.push_back (std::make_unique<juce::AudioParameterFloat> (attackId, attackId, attackRange, 0.01f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (decayId, decayId, decayRange, 0.1f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (sustainId, sustainId, sustainRange, 1.0f));
        params.push_back (std::make_unique<juce::AudioParameterFloat> (releaseId, releaseId, releaseRange, 0.3f));
    }

    return { params.begin(), params.end() };
}

void PluginProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    std::string idStr = parameterID.toStdString();
    size_t lastUnderscore = idStr.find_last_of ('_');

    if (lastUnderscore != std::string::npos && lastUnderscore > 0)
    {
        if (idStr.find ("ATTACK_PAD_") == 0 || idStr.find ("DECAY_PAD_") == 0 || idStr.find ("SUSTAIN_PAD_") == 0 || idStr.find ("RELEASE_PAD_") == 0)
        {
            std::string padNumStr = idStr.substr (lastUnderscore + 1);
            try
            {
                int padId = std::stoi (padNumStr);
                if (padId >= 1 && padId <= numVoices)
                {
                    mParamUpdateFlags[padId - 1].store (true, std::memory_order_relaxed);
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
    int midiNote = 60 + padId - 1;

    juce::SamplerSound* soundToUpdate = nullptr;
    for (int i = 0; i < mSampler.getNumSounds(); ++i)
    {
        if (auto* sound = dynamic_cast<juce::SamplerSound*> (mSampler.getSound (i).get()))
        {
            if (sound->appliesToNote (midiNote))
            {
                soundToUpdate = sound;
                break; // Found the sound
            }
        }
    }

    if (soundToUpdate)
    {
        // 3. Read CURRENT ADSR values from APVTS (thread-safe read)
        std::string padStr = std::to_string (padId);
        juce::ADSR::Parameters newParams;

        // Use fallback defaults if parameter somehow doesn't exist
        newParams.attack = mAPVTS.getRawParameterValue ("ATTACK_PAD_" + padStr) ? mAPVTS.getRawParameterValue ("ATTACK_PAD_" + padStr)->load() : 0.01f;
        newParams.decay = mAPVTS.getRawParameterValue ("DECAY_PAD_" + padStr) ? mAPVTS.getRawParameterValue ("DECAY_PAD_" + padStr)->load() : 0.1f;
        newParams.sustain = mAPVTS.getRawParameterValue ("SUSTAIN_PAD_" + padStr) ? mAPVTS.getRawParameterValue ("SUSTAIN_PAD_" + padStr)->load() : 1.0f;
        newParams.release = mAPVTS.getRawParameterValue ("RELEASE_PAD_" + padStr) ? mAPVTS.getRawParameterValue ("RELEASE_PAD_" + padStr)->load() : 0.3f;

        soundToUpdate->setEnvelopeParameters (newParams);
    }
    else
    {
        printf ("AudioThread: Warning - No SamplerSound found for Pad %d (MIDI %d) to update ADSR.\n", padId, midiNote);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
