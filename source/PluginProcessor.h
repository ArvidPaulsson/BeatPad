#pragma once

#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>

#if (MSVC)
    #include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor,
                        public juce::AudioProcessorValueTreeState::Listener
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadFile (const juce::String& path);
    void loadFile (const juce::String& path, int padId);

    int getNumSamplerSounds() { return mSampler.getNumSounds(); }

    void addMidiMessage (const juce::MidiMessage& message);

    juce::AudioProcessorValueTreeState& getAPVST() { return mAPVTS; }

    juce::AudioBuffer<float>* getWaveformForNote (int midiNote);
    juce::AudioBuffer<float>* getCurrentWaveForm();
    int getCurrentMidiNote();

    void updateADSR (int padId);

    std::atomic<int>& getSampleCount() { return mSampleCount; }

private:
    juce::Synthesiser mSampler;
    juce::AudioFormatManager mFormatManager;
    juce::AudioFormatReader* mReader { nullptr };

    std::map<int, juce::ADSR::Parameters> mADSRParams;
    juce::AudioProcessorValueTreeState mAPVTS;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameters();
    void parameterChanged (const juce::String& parameterID, float newValue) override;
    void registerParameters();

    juce::AudioBuffer<float> waveForm;

    std::map<int, std::unique_ptr<juce::AudioBuffer<float>>> mSampleWaveforms;
    std::atomic<int> currentMidiNote { -1 };
    std::atomic<int> currentPadId { -1 };
    juce::AudioBuffer<float> currentWaveForm;
    const int numVoices { 9 };

    std::atomic<bool> mShouldUpdate { false };
    std::atomic<bool> mIsNotePlayed { false };
    std::atomic<int> mSampleCount { 0 };
    std::atomic<int> mCurrentMidiNoteForDisplay { -1 };
    int mCurrentPlayingNoteForSampleCount { -1 };

    std::vector<std::atomic<bool>> mParamUpdateFlags;
    void updateADSRForPadOnAudioThread (int padId);

    juce::MidiBuffer midiMessageQueue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)
};
;
