#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <memory>
#include <array>

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
    void saveUserPreset();
    void loadUserPreset();
    void styleSlider(juce::Slider& slider, juce::Label& label, const juce::String& name);
    void setPluginParameter(const juce::String& id, float value);

    KrakenKlipperAudioProcessor& processor;
    std::unique_ptr<juce::LookAndFeel_V4> customLookAndFeel;

    juce::Slider driveSlider, ceilingSlider, kneeSlider, mixSlider, outputSlider;
    juce::Label driveLabel, ceilingLabel, kneeLabel, mixLabel, outputLabel;
    juce::TextButton softButton{"SOFT"}, mediumButton{"MEDIUM"}, hardButton{"HARD"};
    juce::ToggleButton bypassButton{"BYPASS"}, deltaButton{"DELTA"};
    juce::TextButton savePresetButton{"SAVE PRESET"}, loadPresetButton{"LOAD PRESET"};
    juce::ComboBox presetSelector;
    juce::Label presetStatusLabel;
    std::shared_ptr<juce::FileChooser> activePresetChooser;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ceilingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deltaAttachment;
    std::unique_ptr<KrakenCurveDisplay> curveDisplay;
    std::unique_ptr<KrakenMeterDisplay> meterDisplay;
    juce::Image panelMural;
    juce::Image headerArtwork;
    juce::Image scaledPool, scaledHeader;
    juce::Image todbTag;
    std::array<float, 8> lastVisualParameters{};
    float syrupEnergy = 0.0f;
    float syrupPhase = 0.0f;
    bool visualParametersInitialised = false;
    juce::TooltipWindow tooltips{this, 500};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrakenKlipperAudioProcessorEditor)
};
