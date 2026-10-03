#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <memory>

class KrakenKlipperAudioProcessor final : public juce::AudioProcessor
{
public:
    KrakenKlipperAudioProcessor();
    ~KrakenKlipperAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock& destination) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters;

    float getInputPeakDb() const noexcept;
    float getOutputPeakDb() const noexcept;
    float getGainReductionDb() const noexcept;
    static constexpr int oversamplingFactor = 8;

private:
    void syncSmoothers(bool initialize = false) noexcept;
    static float dbToGain(float db) noexcept;
    static float gainToDb(float gain) noexcept;

    std::atomic<float>* driveValue = nullptr;
    std::atomic<float>* ceilingValue = nullptr;
    std::atomic<float>* kneeValue = nullptr;
    std::atomic<float>* characterValue = nullptr;
    std::atomic<float>* mixValue = nullptr;
    std::atomic<float>* outputValue = nullptr;
    std::atomic<float>* bypassValue = nullptr;
    std::atomic<float>* deltaValue = nullptr;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ceilingGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> kneeRatio;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> character;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixAmount;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGain;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> bypassAmount;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> deltaAmount;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::AudioBuffer<float> dryOversampled;
    int maximumBlockSize = 1;
    int preparedChannels = 2;
    std::atomic<float> inputPeak = 0.0f;
    std::atomic<float> outputPeak = 0.0f;
    std::atomic<float> gainReduction = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrakenKlipperAudioProcessor)
};
