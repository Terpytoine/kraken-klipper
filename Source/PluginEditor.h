#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <memory>

class KrakenCurveDisplay;
class KrakenMeterDisplay;

class KrakenKlipperAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                private juce::Timer
{
public:
    explicit KrakenKlipperAudioProcessorEditor(KrakenKlipperAudioProcessor& processor);
    ~KrakenKlipperAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void timerCallback() override;
    void setCharacter(int index);
    void applyPreset(int preset);
    void styleSlider(juce::Slider& slider, juce::Label& label, const juce::String& name);
    void setPluginParameter(const juce::String& id, float value);

    KrakenKlipperAudioProcessor& processor;
    std::unique_ptr<juce::LookAndFeel_V4> customLookAndFeel;

    juce::Slider driveSlider, ceilingSlider, kneeSlider, mixSlider, outputSlider;
    juce::Label driveLabel, ceilingLabel, kneeLabel, mixLabel, outputLabel;
    juce::TextButton softButton{"SOFT"}, mediumButton{"MEDIUM"}, hardButton{"HARD"};
    juce::ToggleButton bypassButton{"BYPASS"}, deltaButton{"DELTA"};
    juce::ComboBox presetSelector;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deltaAttachment;
    std::unique_ptr<KrakenCurveDisplay> curveDisplay;
    std::unique_ptr<KrakenMeterDisplay> meterDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrakenKlipperAudioProcessorEditor)
};
