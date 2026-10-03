#include "PluginProcessor.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <array>
#include <vector>

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

    std::array<std::vector<float>, 3> renderedModes;
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
        auto& rendered = renderedModes[static_cast<size_t>(character)];
        rendered.resize(static_cast<size_t>(hostBlock));
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
                if (channel == 0)
                    rendered[static_cast<size_t>(i)] = sample;
            }

        if (peak < 1.0e-4f)
        {
            std::cerr << "Processor output was unexpectedly silent.\n";
            return false;
        }
    }

    for (size_t mode = 1; mode < renderedModes.size(); ++mode)
    {
        double squaredDifference = 0.0;
        const auto comparisonStart = static_cast<size_t>(hostBlock * 3 / 4);
        for (auto i = comparisonStart; i < static_cast<size_t>(hostBlock); ++i)
        {
            const auto difference = static_cast<double>(renderedModes[mode][i]
                                                        - renderedModes[mode - 1][i]);
            squaredDifference += difference * difference;
        }
        const auto comparedSamples = static_cast<double>(hostBlock) - static_cast<double>(comparisonStart);
        const auto differenceRms = std::sqrt(squaredDifference / comparedSamples);
        if (differenceRms < 1.0e-4)
        {
            fail("Soft, Medium, and Hard modes did not produce distinct audio.");
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
    // Both host sessions and .dcpreset files use this same parameter state.
    KrakenKlipperAudioProcessor original;
    setParameter(original, "drive", 11.0f);
    setParameter(original, "ceiling", -6.0f);
    setParameter(original, "knee", 21.0f);
    setParameter(original, "character", 0.0f);
    setParameter(original, "mix", 73.0f);
    setParameter(original, "output", -4.0f);
    juce::MemoryBlock saved;
    original.getStateInformation(saved);
    KrakenKlipperAudioProcessor restored;
    restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    for (const auto* id : {"drive", "ceiling", "knee", "character", "mix", "output", "bypass", "delta"})
        if (std::abs(original.parameters.getRawParameterValue(id)->load()
                     - restored.parameters.getRawParameterValue(id)->load()) > 0.001f)
            return fail("Saved settings did not restore correctly.");
    auto presetXml = original.parameters.copyState().createXml();
    auto parsedXml = juce::XmlDocument::parse(presetXml->toString());
    restored.parameters.replaceState(juce::ValueTree::fromXml(*parsedXml));
    if (std::abs(restored.parameters.getRawParameterValue("knee")->load() - 21.0f) > 0.001f)
        return fail("Preset XML did not restore the knee.");

    if (!runLayout(juce::AudioChannelSet::mono(), 1))
        return EXIT_FAILURE;
    if (!runLayout(juce::AudioChannelSet::stereo(), 2))
        return EXIT_FAILURE;

    std::cout << "Double Cup Clipper processor smoke checks passed.\n";
    return EXIT_SUCCESS;
}
