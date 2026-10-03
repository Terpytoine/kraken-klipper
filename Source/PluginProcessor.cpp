#include "PluginProcessor.h"
#include "ClipperCurve.h"
#include "PluginEditor.h"

namespace
{
constexpr auto driveId = "drive";
constexpr auto ceilingId = "ceiling";
constexpr auto kneeId = "knee";
constexpr auto characterId = "character";
constexpr auto mixId = "mix";
constexpr auto outputId = "output";
constexpr auto bypassId = "bypass";
constexpr auto deltaId = "delta";
}

KrakenKlipperAudioProcessor::KrakenKlipperAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "KRAKEN_PARAMETERS", createParameterLayout())
{
    driveValue = parameters.getRawParameterValue(driveId);
    ceilingValue = parameters.getRawParameterValue(ceilingId);
    kneeValue = parameters.getRawParameterValue(kneeId);
    characterValue = parameters.getRawParameterValue(characterId);
    mixValue = parameters.getRawParameterValue(mixId);
    outputValue = parameters.getRawParameterValue(outputId);
    bypassValue = parameters.getRawParameterValue(bypassId);
    deltaValue = parameters.getRawParameterValue(deltaId);
}

juce::AudioProcessorValueTreeState::ParameterLayout KrakenKlipperAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> layout;
    const auto version = 1;

    layout.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{driveId, version}, "Drive", NormalisableRange<float>{-24.0f, 24.0f, 0.01f}, 0.0f,
        AudioParameterFloatAttributes().withLabel("dB")));
    layout.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{ceilingId, version}, "Ceiling", NormalisableRange<float>{-24.0f, 0.0f, 0.01f}, -1.0f,
        AudioParameterFloatAttributes().withLabel("dBFS")));
    layout.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{kneeId, version}, "Knee", NormalisableRange<float>{0.0f, 24.0f, 0.01f}, 12.0f,
        AudioParameterFloatAttributes().withLabel("dB")));
    layout.push_back(std::make_unique<AudioParameterChoice>(
        ParameterID{characterId, version}, "Character", StringArray{"Soft", "Medium", "Hard"}, 1));
    layout.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{mixId, version}, "Mix", NormalisableRange<float>{0.0f, 100.0f, 0.01f}, 100.0f,
        AudioParameterFloatAttributes().withLabel("%")));
    layout.push_back(std::make_unique<AudioParameterFloat>(
        ParameterID{outputId, version}, "Output", NormalisableRange<float>{-24.0f, 12.0f, 0.01f}, 0.0f,
        AudioParameterFloatAttributes().withLabel("dB")));
    layout.push_back(std::make_unique<AudioParameterBool>(ParameterID{bypassId, version}, "Bypass", false));
    layout.push_back(std::make_unique<AudioParameterBool>(ParameterID{deltaId, version}, "Delta", false));
    return {layout.begin(), layout.end()};
}

void KrakenKlipperAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    preparedChannels = juce::jlimit(1, 2, getTotalNumInputChannels());
    maximumBlockSize = juce::jmax(1, samplesPerBlock);
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        static_cast<size_t>(preparedChannels), 3,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
    oversampling->initProcessing(static_cast<size_t>(maximumBlockSize));
    oversampling->reset();
    setLatencySamples(static_cast<int>(std::ceil(oversampling->getLatencyInSamples())));

    const auto highRateCapacity = maximumBlockSize * oversamplingFactor;
    dryOversampled.setSize(preparedChannels, highRateCapacity, false, true, true);

    const auto highSampleRate = sampleRate * static_cast<double>(oversamplingFactor);
    constexpr auto smoothingSeconds = 0.025;
    driveGain.reset(highSampleRate, smoothingSeconds);
    ceilingGain.reset(highSampleRate, smoothingSeconds);
    kneeRatio.reset(highSampleRate, smoothingSeconds);
    character.reset(highSampleRate, smoothingSeconds);
    mixAmount.reset(highSampleRate, smoothingSeconds);
    outputGain.reset(highSampleRate, smoothingSeconds);
    bypassAmount.reset(highSampleRate, 0.010);
    deltaAmount.reset(highSampleRate, 0.010);
    syncSmoothers(true);
}

void KrakenKlipperAudioProcessor::releaseResources()
{
    if (oversampling != nullptr)
        oversampling->reset();
}

bool KrakenKlipperAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto input = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    if (input != output)
        return false;
    return input == juce::AudioChannelSet::mono() || input == juce::AudioChannelSet::stereo();
}

float KrakenKlipperAudioProcessor::dbToGain(float db) noexcept
{
    return std::pow(10.0f, db * 0.05f);
}

float KrakenKlipperAudioProcessor::gainToDb(float gain) noexcept
{
    return gain > 1.0e-9f ? 20.0f * std::log10(gain) : -180.0f;
}

void KrakenKlipperAudioProcessor::syncSmoothers(bool initialize) noexcept
{
    const auto drive = dbToGain(driveValue->load(std::memory_order_relaxed));
    const auto ceiling = dbToGain(ceilingValue->load(std::memory_order_relaxed));
    const auto knee = dbToGain(-kneeValue->load(std::memory_order_relaxed));
    const auto style = characterValue->load(std::memory_order_relaxed);
    const auto mix = mixValue->load(std::memory_order_relaxed) * 0.01f;
    const auto output = dbToGain(outputValue->load(std::memory_order_relaxed));
    const auto bypass = bypassValue->load(std::memory_order_relaxed) >= 0.5f ? 1.0f : 0.0f;
    const auto delta = deltaValue->load(std::memory_order_relaxed) >= 0.5f ? 1.0f : 0.0f;
    if (initialize)
    {
        driveGain.setCurrentAndTargetValue(drive);
        ceilingGain.setCurrentAndTargetValue(ceiling);
        kneeRatio.setCurrentAndTargetValue(knee);
        character.setCurrentAndTargetValue(style);
        mixAmount.setCurrentAndTargetValue(mix);
        outputGain.setCurrentAndTargetValue(output);
        bypassAmount.setCurrentAndTargetValue(bypass);
        deltaAmount.setCurrentAndTargetValue(delta);
        return;
    }
    driveGain.setTargetValue(drive);
    ceilingGain.setTargetValue(ceiling);
    kneeRatio.setTargetValue(knee);
    character.setTargetValue(style);
    mixAmount.setTargetValue(mix);
    outputGain.setTargetValue(output);
    bypassAmount.setTargetValue(bypass);
    deltaAmount.setTargetValue(delta);
}

void KrakenKlipperAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused(midi);
    juce::ScopedNoDenormals noDenormals;

    const auto inputChannels = getTotalNumInputChannels();
    const auto outputChannels = getTotalNumOutputChannels();
    for (auto channel = inputChannels; channel < outputChannels; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (oversampling == nullptr || inputChannels < 1)
        return;

    syncSmoothers();
    const auto samples = buffer.getNumSamples();
    const auto channels = juce::jmin(inputChannels, preparedChannels, buffer.getNumChannels());
    if (samples <= 0 || channels <= 0)
    {
        inputPeak.store(0.0f, std::memory_order_relaxed);
        outputPeak.store(0.0f, std::memory_order_relaxed);
        gainReduction.store(0.0f, std::memory_order_relaxed);
        return;
    }

    // Hosts can pass occasional blocks larger than prepareToPlay's hint. Sanitize
    // input first, then process those blocks in bounded chunks so JUCE's
    // preallocated oversampling buffers are never overrun.
    for (auto channel = 0; channel < channels; ++channel)
    {
        auto* audio = buffer.getWritePointer(channel);
        for (auto sample = 0; sample < samples; ++sample)
            if (!std::isfinite(audio[sample]))
                audio[sample] = 0.0f;
    }

    float inPeak = 0.0f;
    for (auto channel = 0; channel < channels; ++channel)
        inPeak = juce::jmax(inPeak, buffer.getMagnitude(channel, 0, samples));

    auto fullInputBlock = juce::dsp::AudioBlock<float>(buffer)
                              .getSubsetChannelBlock(0, static_cast<size_t>(channels));
    float maxReductionRatio = 1.0f;
    auto sampleOffset = 0;
    while (sampleOffset < samples)
    {
        const auto chunkSamples = juce::jmin(maximumBlockSize, samples - sampleOffset);
        auto inputBlock = fullInputBlock.getSubBlock(static_cast<size_t>(sampleOffset),
                                                      static_cast<size_t>(chunkSamples));
        auto& highRateBlock = oversampling->processSamplesUp(inputBlock);
        const auto highSamples = highRateBlock.getNumSamples();
        auto dryBlock = juce::dsp::AudioBlock<float>(dryOversampled)
                            .getSubsetChannelBlock(0, static_cast<size_t>(channels))
                            .getSubBlock(0, highSamples);

        for (auto channel = 0; channel < channels; ++channel)
        {
            auto* upsampled = highRateBlock.getChannelPointer(static_cast<size_t>(channel));
            juce::FloatVectorOperations::copy(dryBlock.getChannelPointer(static_cast<size_t>(channel)),
                                              upsampled, static_cast<int>(highSamples));
        }

        for (size_t sample = 0; sample < highSamples; ++sample)
        {
            const auto drive = driveGain.getNextValue();
            const auto ceiling = ceilingGain.getNextValue();
            const auto knee = kneeRatio.getNextValue();
            const auto style = character.getNextValue();
            const auto wet = mixAmount.getNextValue();
            const auto trim = outputGain.getNextValue();
            const auto bypass = bypassAmount.getNextValue();
            const auto delta = deltaAmount.getNextValue();

            for (auto channel = 0; channel < channels; ++channel)
            {
                auto* audio = highRateBlock.getChannelPointer(static_cast<size_t>(channel));
                const auto dry = dryBlock.getChannelPointer(static_cast<size_t>(channel))[sample];
                const auto driven = dry * drive;
                const auto shaped = kraken::clipBlend(driven, ceiling, knee, style);
                // Ceiling is the clipping threshold. Output is a true post-clip
                // trim, so positive gain is allowed to raise the final level.
                const auto processed = (dry + (shaped - dry) * wet) * trim;
                const auto difference = processed - dry;
                const auto effected = difference * delta + processed * (1.0f - delta);
                audio[sample] = dry * bypass + effected * (1.0f - bypass);

                const auto drivenMagnitude = std::abs(driven);
                const auto shapedMagnitude = std::abs(shaped);
                if (drivenMagnitude > shapedMagnitude && drivenMagnitude > 1.0e-8f)
                    maxReductionRatio = juce::jmax(maxReductionRatio,
                        drivenMagnitude / juce::jmax(shapedMagnitude, 1.0e-8f));
            }
        }

        // Downsample the processed high-rate buffer back into the original
        // normal-rate host block for this chunk.
        oversampling->processSamplesDown(inputBlock);
        sampleOffset += chunkSamples;
    }

    float outPeak = 0.0f;
    for (auto channel = 0; channel < channels; ++channel)
    {
        auto* audio = buffer.getWritePointer(channel);
        for (auto sample = 0; sample < samples; ++sample)
        {
            if (!std::isfinite(audio[sample]))
                audio[sample] = 0.0f;
        }
        outPeak = juce::jmax(outPeak, buffer.getMagnitude(channel, 0, samples));
    }

    inputPeak.store(inPeak, std::memory_order_relaxed);
    outputPeak.store(outPeak, std::memory_order_relaxed);
    gainReduction.store(gainToDb(maxReductionRatio), std::memory_order_relaxed);
}

juce::AudioProcessorEditor* KrakenKlipperAudioProcessor::createEditor()
{
    return new KrakenKlipperAudioProcessorEditor(*this);
}

void KrakenKlipperAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary(*xml, destination);
}

void KrakenKlipperAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

float KrakenKlipperAudioProcessor::getInputPeakDb() const noexcept { return gainToDb(inputPeak.load(std::memory_order_relaxed)); }
float KrakenKlipperAudioProcessor::getOutputPeakDb() const noexcept { return gainToDb(outputPeak.load(std::memory_order_relaxed)); }
float KrakenKlipperAudioProcessor::getGainReductionDb() const noexcept { return gainReduction.load(std::memory_order_relaxed); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KrakenKlipperAudioProcessor();
}
