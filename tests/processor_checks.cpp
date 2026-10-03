#include "PluginProcessor.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
int fail(const char* message)
{
    std::cerr << "Processor check failed: " << message << '\n';
    return EXIT_FAILURE;
}

void setParameter(KrakenKlipperAudioProcessor& processor, const juce::String& id, float value)
{
    if (auto* parameter = processor.parameters.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

bool runLayout(const juce::AudioChannelSet& layout, int channelCount)
{
    KrakenKlipperAudioProcessor processor;
    auto buses = processor.getBusesLayout();
    buses.inputBuses.getReference(0) = layout;
    buses.outputBuses.getReference(0) = layout;
    if (!processor.setBusesLayout(buses))
    {
        std::cerr << "Processor rejected a supported channel layout.\n";
        return false;
    }

    constexpr int preparedBlock = 64;
    constexpr int hostBlock = 5003; // Deliberately larger than the prepare hint.
    processor.prepareToPlay(48000.0, preparedBlock);
    if (processor.getLatencySamples() < 0)
    {
        std::cerr << "Processor reported a negative latency.\n";
        return false;
    }

    for (int character = 0; character != 3; ++character)
    {
        setParameter(processor, "character", static_cast<float>(character));
        setParameter(processor, "drive", 18.0f);
        setParameter(processor, "mix", 100.0f);
        juce::AudioBuffer<float> audio(channelCount, hostBlock);
        for (int channel = 0; channel < channelCount; ++channel)
        {
            auto* samples = audio.getWritePointer(channel);
            for (int i = 0; i < hostBlock; ++i)
            {
                const auto phase = static_cast<float>(i) * 0.031f + channel * 0.7f;
                samples[i] = 1.4f * std::sin(phase);
            }
            samples[7] = std::numeric_limits<float>::quiet_NaN();
            samples[9] = std::numeric_limits<float>::infinity();
        }

        juce::MidiBuffer midi;
        processor.processBlock(audio, midi);

        float peak = 0.0f;
        for (int channel = 0; channel < channelCount; ++channel)
            for (int i = 0; i < hostBlock; ++i)
            {
                const auto sample = audio.getSample(channel, i);
                if (!std::isfinite(sample))
                {
                    std::cerr << "Processor emitted a non-finite sample.\n";
                    return false;
                }
                peak = juce::jmax(peak, std::abs(sample));
            }

        if (peak < 1.0e-4f)
        {
            std::cerr << "Processor output was unexpectedly silent.\n";
            return false;
        }
    }

    // Exercise a one-sample block after the oversized blocks to catch edge cases
    // in the oversampler's retained state and chunk boundaries.
    juce::AudioBuffer<float> oneSample(channelCount, 1);
    oneSample.clear();
    oneSample.setSample(0, 0, 0.25f);
    juce::MidiBuffer midi;
    processor.processBlock(oneSample, midi);
    for (int channel = 0; channel < channelCount; ++channel)
        if (!std::isfinite(oneSample.getSample(channel, 0)))
        {
            std::cerr << "Processor failed on a one-sample block.\n";
            return false;
        }

    processor.releaseResources();
    return true;
}
}

int main()
{
    if (!runLayout(juce::AudioChannelSet::mono(), 1))
        return EXIT_FAILURE;
    if (!runLayout(juce::AudioChannelSet::stereo(), 2))
        return EXIT_FAILURE;

    std::cout << "Double Cup Clipper processor smoke checks passed.\n";
    return EXIT_SUCCESS;
}
