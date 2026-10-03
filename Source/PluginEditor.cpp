#include "PluginEditor.h"
#include "ClipperCurve.h"
#include "BinaryData.h"
#include <array>

namespace
{
const juce::Colour ink{0xff090612};
const juce::Colour panel{0xff151023};
const juce::Colour panelEdge{0xff8b622b};
const juce::Colour purple{0xff9d54ed};
const juce::Colour violet{0xffc488ff};
const juce::Colour gold{0xffffcf68};
const juce::Colour paper{0xfff0eaf7};
const juce::Colour muted{0xffd9c6e9};

class KrakenLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    float liquidPhase = 0.0f;
    float liquidEnergy = 0.0f;

    KrakenLookAndFeel()
    {
        setColour(juce::Slider::textBoxTextColourId, paper);
        setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0e0a18));
        setColour(juce::Slider::textBoxOutlineColourId, panelEdge);
        setColour(juce::TextButton::textColourOffId, paper);
        setColour(juce::TextButton::textColourOnId, paper);
        setColour(juce::ComboBox::textColourId, paper);
        setColour(juce::ComboBox::backgroundColourId, panel);
        setColour(juce::ComboBox::outlineColourId, panelEdge);
        setColour(juce::ComboBox::arrowColourId, gold);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                                    static_cast<float>(width), static_cast<float>(height)).reduced(9.0f);
        const auto diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const auto knob = bounds.withSizeKeepingCentre(diameter, diameter);
        const auto centre = knob.getCentre();
        const auto radius = diameter * 0.5f;

        g.setColour(juce::Colour(0x33200b35));
        g.fillEllipse(knob.expanded(4.0f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff33264a), knob.getTopLeft(),
                                               juce::Colour(0xff0a0811), knob.getBottomRight(), false));
        g.fillEllipse(knob);
        g.setColour(gold.withAlpha(0.76f));
        g.drawEllipse(knob, 1.4f);
        g.setColour(gold.withAlpha(0.32f));
        g.drawEllipse(knob.reduced(5.0f), 1.0f);

        // Decorative syrup is rendered on the message thread, never in audio processing.
        const auto cup = knob.reduced(diameter * 0.23f, diameter * 0.24f);
        juce::Path bowl;
        bowl.startNewSubPath(cup.getX(), cup.getY());
        bowl.lineTo(cup.getRight(), cup.getY());
        bowl.lineTo(cup.getRight() - cup.getWidth() * 0.13f, cup.getBottom());
        bowl.quadraticTo(cup.getCentreX(), cup.getBottom() + 4.0f,
                          cup.getX() + cup.getWidth() * 0.13f, cup.getBottom());
        bowl.closeSubPath();
        g.setGradientFill(juce::ColourGradient(paper, cup.getTopLeft(),
                                              juce::Colour(0xffae8fca), cup.getBottomRight(), false));
        g.fillPath(bowl);
        g.setColour(gold);
        g.strokePath(bowl, juce::PathStrokeType(1.2f));
        g.saveState();
        g.reduceClipRegion(bowl);
        const auto fillY = cup.getBottom() - cup.getHeight() * (0.16f + 0.70f * sliderPos);
        juce::Path liquid;
        liquid.startNewSubPath(knob.getX(), knob.getBottom());
        liquid.lineTo(knob.getX(), fillY);
        for (int point = 0; point <= 24; ++point)
        {
            const auto t = static_cast<float>(point) / 24.0f;
            liquid.lineTo(knob.getX() + t * diameter,
                          fillY + std::sin(t * 8.0f + liquidPhase) * (1.2f + liquidEnergy * 3.2f));
        }
        liquid.lineTo(knob.getRight(), knob.getBottom());
        liquid.closeSubPath();
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb25bf8), centre.x, fillY,
                                              juce::Colour(0xff3b0869), centre.x, knob.getBottom(), false));
        g.fillPath(liquid);
        g.setColour(violet.withAlpha(0.65f));
        g.strokePath(liquid, juce::PathStrokeType(1.0f));
        g.restoreState();
        g.setColour(paper);
        g.drawEllipse(cup.getX() - 1.0f, cup.getY() - 3.0f, cup.getWidth() + 2.0f, 7.0f, 2.0f);
        g.setColour(gold);
        g.drawEllipse(cup.getX() - 2.0f, cup.getY() - 6.0f, cup.getWidth() + 4.0f, 8.0f, 1.3f);
        const auto dripX = cup.getX() + cup.getWidth() * 0.22f;
        const auto dripLength = diameter * (0.08f + 0.27f * sliderPos);
        g.setColour(purple);
        g.fillRoundedRectangle(dripX, cup.getY() + 1.0f, 4.5f, dripLength, 2.2f);
        g.setColour(violet);
        g.drawLine(dripX + 1.2f, cup.getY() + 3.0f, dripX + 1.2f,
                    cup.getY() + dripLength - 2.0f, 1.0f);
        if (liquidEnergy > 0.01f)
        {
            const auto fall = std::fmod(liquidPhase * 0.22f, 1.0f);
            const auto dropY = cup.getY() + dripLength + fall * diameter * 0.18f;
            g.setColour(violet.withAlpha(liquidEnergy * (1.0f - fall)));
            g.fillEllipse(dripX, dropY, 4.5f, 6.0f);
        }

        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius - 7.0f, radius - 7.0f, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xff453953));
        g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));

        juce::Path active;
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        active.addCentredArc(centre.x, centre.y, radius - 7.0f, radius - 7.0f, 0.0f,
                             rotaryStartAngle, angle, true);
        g.setColour(gold);
        g.strokePath(active, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        const auto pointerLength = radius * 0.60f;
        const auto pointerWidth = 2.7f;
        juce::Path pointer;
        pointer.addRoundedRectangle(-pointerWidth * 0.5f, -radius * 0.70f,
                                    pointerWidth, pointerLength, 1.0f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
        g.setColour(gold);
        g.fillPath(pointer);
        g.setColour(juce::Colour(0xffd7c5ec));
        g.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool hovered, bool pressed) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        const auto selected = button.getToggleState();
        auto fill = selected ? juce::Colour(0xff321c4b) : juce::Colour(0xff100d19);
        if (hovered) fill = fill.brighter(0.12f);
        if (pressed) fill = fill.darker(0.1f);
        g.setGradientFill(juce::ColourGradient(fill.brighter(selected ? 0.4f : 0.14f), bounds.getTopLeft(),
                                              fill.darker(0.15f), bounds.getBottomRight(), false));
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(selected ? gold : panelEdge);
        g.drawRoundedRectangle(bounds, 8.0f, selected ? 1.7f : 1.0f);
        if (selected)
        {
            g.setColour(purple.withAlpha(0.34f));
            g.fillRoundedRectangle(bounds.reduced(3.0f), 6.0f);
        }
        g.setColour(gold.withAlpha(selected ? 0.56f : 0.20f));
        g.drawLine(bounds.getX() + 9.0f, bounds.getY() + 3.0f,
                    bounds.getRight() - 9.0f, bounds.getY() + 3.0f, 1.0f);
        for (int drip = 0; drip < 3; ++drip)
        {
            const auto dx = bounds.getX() + bounds.getWidth() * (0.18f + drip * 0.29f);
            g.setColour((selected ? violet : purple).withAlpha(0.64f));
            g.fillRoundedRectangle(dx, bounds.getBottom() - 5.0f, 3.0f,
                                    3.0f + (drip == 1 ? 1.0f : 0.0f), 1.5f);
        }
    }

    juce::Font getTextButtonFont(juce::TextButton&, int height) override
    {
        return juce::Font(juce::FontOptions(height > 25 ? "Segoe Print" : "Trebuchet MS", juce::jlimit(11.0f, 16.0f, height * 0.40f),
                                            juce::Font::bold)
                              .withFallbacks({"Arial Black", "Arial"}));
    }

    static juce::TextLayout helpLayout(const juce::String& text)
    {
        juce::AttributedString content;
        content.append(text, juce::Font(juce::FontOptions("Trebuchet MS", 13.0f)), paper);
        juce::TextLayout layout;
        layout.createLayout(content, 330.0f);
        return layout;
    }

    juce::Rectangle<int> getTooltipBounds(const juce::String& text, juce::Point<int> position,
                                         juce::Rectangle<int> parentArea) override
    {
        const auto layout = helpLayout(text);
        return juce::Rectangle<int>(position.x + 12, position.y + 24,
                                    static_cast<int>(std::ceil(layout.getWidth())) + 22,
                                    static_cast<int>(std::ceil(layout.getHeight())) + 18).constrainedWithin(parentArea);
    }

    void drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height) override
    {
        g.setColour(juce::Colour(0xff1d0a2f));
        g.fillRoundedRectangle(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 8.0f);
        g.setColour(gold);
        g.drawRoundedRectangle(0.5f, 0.5f, static_cast<float>(width) - 1.0f,
                                static_cast<float>(height) - 1.0f, 8.0f, 1.0f);
        auto layout = helpLayout(text);
        layout.draw(g, juce::Rectangle<float>(11.0f, 9.0f, static_cast<float>(width) - 22.0f,
                                               static_cast<float>(height) - 18.0f));
    }
};

}

class KrakenCurveDisplay final : public juce::Component,
                                 public juce::SettableTooltipClient,
                                 private juce::Timer
{
public:
    explicit KrakenCurveDisplay(KrakenKlipperAudioProcessor& p) : processor(p) { startTimerHz(24); }

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff0b0812));
        g.fillRoundedRectangle(area, 12.0f);
        g.setColour(panelEdge);
        g.drawRoundedRectangle(area.reduced(0.5f), 12.0f, 1.0f);

        auto plot = area.reduced(30.0f, 22.0f);
        plot.removeFromTop(10.0f);
        plot.removeFromBottom(18.0f);
        g.setColour(juce::Colour(0xff241b32));
        for (int i = 0; i <= 4; ++i)
        {
            const auto x = plot.getX() + plot.getWidth() * static_cast<float>(i) / 4.0f;
            const auto y = plot.getY() + plot.getHeight() * static_cast<float>(i) / 4.0f;
            const auto isZero = i == 2;
            g.setColour(isZero ? juce::Colour(0xff625777) : juce::Colour(0xff241b32));
            const auto stroke = isZero ? 1.35f : 1.0f;
            g.drawLine(x, plot.getY(), x, plot.getBottom(), stroke);
            g.drawLine(plot.getX(), y, plot.getRight(), y, stroke);
        }
        g.setColour(juce::Colour(0xff625777));
        for (int i = 0; i < 16; ++i)
        {
            const auto t0 = static_cast<float>(i) / 16.0f;
            const auto t1 = t0 + 0.035f;
            g.drawLine(plot.getX() + t0 * plot.getWidth(), plot.getBottom() - t0 * plot.getHeight(),
                       plot.getX() + t1 * plot.getWidth(), plot.getBottom() - t1 * plot.getHeight(), 1.0f);
        }

        const auto drive = std::pow(10.0f, processor.parameters.getRawParameterValue("drive")->load() / 20.0f);
        const auto ceiling = std::pow(10.0f, processor.parameters.getRawParameterValue("ceiling")->load() / 20.0f);
        const auto kneeRatio = std::pow(10.0f, -processor.parameters.getRawParameterValue("knee")->load() / 20.0f);
        const auto styleValue = processor.parameters.getRawParameterValue("character")->load();
        const auto mix = processor.parameters.getRawParameterValue("mix")->load() * 0.01f;
        const auto trim = std::pow(10.0f, processor.parameters.getRawParameterValue("output")->load() / 20.0f);
        const auto bypass = processor.parameters.getRawParameterValue("bypass")->load() >= 0.5f;
        const auto delta = processor.parameters.getRawParameterValue("delta")->load() >= 0.5f;

        juce::Path curve;
        for (int i = 0; i <= 180; ++i)
        {
            const auto u = static_cast<float>(i) / 180.0f;
            const auto input = (u * 2.0f - 1.0f) * 1.4f;
            const auto clipped = kraken::clipBlend(input * drive, ceiling, kneeRatio, styleValue);
            const auto processed = (input + (clipped - input) * mix) * trim;
            const auto output = bypass ? input : (delta ? processed - input : processed);
            const auto normalizedY = juce::jlimit(-1.15f, 1.15f, output / 1.4f);
            const auto px = plot.getX() + u * plot.getWidth();
            const auto py = plot.getCentreY() - normalizedY * plot.getHeight() * 0.5f;
            if (i == 0) curve.startNewSubPath(px, py); else curve.lineTo(px, py);
        }
        const auto curveColour = styleValue > 1.95f ? gold : violet;
        g.setColour(curveColour.withAlpha(0.16f));
        g.strokePath(curve, juce::PathStrokeType(8.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
        g.setColour(curveColour);
        g.strokePath(curve, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
        g.setColour(gold);
        g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
        auto titleRow = getLocalBounds().removeFromTop(22).reduced(15, 0);
        g.drawText("TRANSFER CURVE", titleRow.removeFromLeft(120), juce::Justification::centredLeft);
        const auto legend = bypass ? juce::String("BYPASS: UNCHANGED INPUT")
                           : delta ? juce::String("CURVE = WHAT CHANGED")
                                   : juce::String("CURVE = SHAPED  |  DASHED = NO CLIP");
        g.drawText(legend, titleRow.removeFromLeft(titleRow.getWidth() - 76),
                   juce::Justification::centredLeft);
        g.drawText("Y: OUTPUT", titleRow.removeFromRight(64), juce::Justification::centredRight);
        g.drawText("INPUT LEVEL", juce::Rectangle<int>(static_cast<int>(plot.getX()), getHeight() - 20,
                                                       static_cast<int>(plot.getWidth()), 14),
                   juce::Justification::centred);
    }

private:
    void timerCallback() override { repaint(); }
    KrakenKlipperAudioProcessor& processor;
};

class KrakenMeterDisplay final : public juce::Component,
                                 public juce::SettableTooltipClient,
                                 private juce::Timer
{
public:
    explicit KrakenMeterDisplay(KrakenKlipperAudioProcessor& p) : processor(p) { startTimerHz(30); }

    void paint(juce::Graphics& g) override
    {
        auto box = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff0b0812));
        g.fillRoundedRectangle(box, 12.0f);
        g.setColour(panelEdge);
        g.drawRoundedRectangle(box.reduced(0.5f), 12.0f, 1.0f);

        auto inner = getLocalBounds().reduced(16, 9);
        const auto labelsWidth = 64;
        constexpr int valueWidth = 74;
        constexpr int rowGap = 27;
        auto drawMeter = [&](const juce::String& label, float db, int row, const juce::Colour& colour)
        {
            auto line = inner.withY(inner.getY() + row * rowGap).withHeight(20);
            g.setColour(muted);
            g.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
            g.drawText(label, line.removeFromLeft(labelsWidth), juce::Justification::centredLeft);
            auto bar = line.removeFromLeft(line.getWidth() - valueWidth).toFloat();
            g.setColour(juce::Colour(0xff21192c));
            g.fillRoundedRectangle(bar, 5.0f);
            const auto normalized = juce::jlimit(0.0f, 1.0f, (db + 36.0f) / 36.0f);
            const auto overZero = label == "OUTPUT" && db > 0.0f;
            g.setColour(overZero ? juce::Colour(0xfff36b8f) : colour);
            g.fillRoundedRectangle(bar.withWidth(bar.getWidth() * normalized), 5.0f);
            g.setColour(paper);
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            const auto value = db <= -90.0f ? juce::String("-INF dBFS")
                                             : juce::String(db, 1) + " dBFS";
            g.drawText(value, line.removeFromRight(valueWidth), juce::Justification::centredRight);
        };

        drawMeter("INPUT", inputHoldDb, 0, purple);
        drawMeter("OUTPUT", outputHoldDb, 1, gold);

        auto grLine = inner.withY(inner.getY() + 2 * rowGap).withHeight(20);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
        g.drawText("REDUCTION", grLine.removeFromLeft(labelsWidth), juce::Justification::centredLeft);
        auto grBar = grLine.removeFromLeft(grLine.getWidth() - valueWidth).toFloat();
        g.setColour(juce::Colour(0xff21192c));
        g.fillRoundedRectangle(grBar, 5.0f);
        const auto reduction = juce::jlimit(0.0f, 18.0f, reductionHoldDb);
        g.setColour(violet);
        g.fillRoundedRectangle(grBar.withWidth(grBar.getWidth() * reduction / 18.0f), 5.0f);
        g.setColour(paper);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::String(reduction, 1) + " dB", grLine.removeFromRight(valueWidth),
                   juce::Justification::centredRight);
    }

private:
    void timerCallback() override
    {
        constexpr float decayPerFrameDb = 0.8f;
        inputHoldDb = juce::jmax(processor.getInputPeakDb(), inputHoldDb - decayPerFrameDb);
        outputHoldDb = juce::jmax(processor.getOutputPeakDb(), outputHoldDb - decayPerFrameDb);
        reductionHoldDb = juce::jmax(processor.getGainReductionDb(), reductionHoldDb - decayPerFrameDb);
        repaint();
    }

    KrakenKlipperAudioProcessor& processor;
    float inputHoldDb = -180.0f;
    float outputHoldDb = -180.0f;
    float reductionHoldDb = 0.0f;
};

KrakenKlipperAudioProcessorEditor::KrakenKlipperAudioProcessorEditor(KrakenKlipperAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      customLookAndFeel(std::make_unique<KrakenLookAndFeel>()),
      curveDisplay(std::make_unique<KrakenCurveDisplay>(p)),
      meterDisplay(std::make_unique<KrakenMeterDisplay>(p)),
      panelMural(juce::ImageCache::getFromMemory(BinaryData::SyrupPool_png, BinaryData::SyrupPool_pngSize)),
      headerArtwork(juce::ImageCache::getFromMemory(BinaryData::DoubleCupHeader_png, BinaryData::DoubleCupHeader_pngSize)),
      todbTag(juce::ImageCache::getFromMemory(BinaryData::TODBTag_jpg, BinaryData::TODBTag_jpgSize))
{
    setSize(1120, 790);
    tooltips.setLookAndFeel(customLookAndFeel.get());
    setResizable(true, true);
    setResizeLimits(900, 790, 1350, 1040);

    styleSlider(driveSlider, driveLabel, "DRIVE");
    styleSlider(ceilingSlider, ceilingLabel, "CEILING");
    styleSlider(kneeSlider, kneeLabel, "KNEE");
    styleSlider(mixSlider, mixLabel, "MIX");
    styleSlider(outputSlider, outputLabel, "OUTPUT");
    driveSlider.setDoubleClickReturnValue(true, 0.0);
    ceilingSlider.setDoubleClickReturnValue(true, -1.0);
    kneeSlider.setDoubleClickReturnValue(true, 12.0);
    mixSlider.setDoubleClickReturnValue(true, 100.0);
    outputSlider.setDoubleClickReturnValue(true, 0.0);
    driveSlider.setTooltip("Drive: input gain before the clipper. More drive pushes the sound further into the clipping curve.");
    ceilingSlider.setTooltip("Ceiling: the level where clipping reaches its limit. Output trim is applied afterward and can raise the final level above the ceiling.");
    kneeSlider.setTooltip("Knee rounds the shoulder as the signal crosses the Ceiling. Raise Drive until the curve reaches the ceiling to hear it. Hard ignores Knee.");
    mixSlider.setTooltip("Mix: blend between the dry input and clipped signal. 100% is fully processed.");
    outputSlider.setTooltip("Output: final level trim after clipping. Watch the Output meter; positive gain can raise the signal above 0 dBFS.");
    driveSlider.setTextValueSuffix(" dB");
    ceilingSlider.setTextValueSuffix(" dBFS");
    kneeSlider.setTextValueSuffix(" dB");
    mixSlider.setTextValueSuffix(" %");
    outputSlider.setTextValueSuffix(" dB");
    for (auto* slider : {&driveSlider, &ceilingSlider, &kneeSlider, &outputSlider})
        slider->setNumDecimalPlacesToDisplay(1);
    mixSlider.setNumDecimalPlacesToDisplay(0);

    for (auto* button : {static_cast<juce::Button*>(&softButton),
                         static_cast<juce::Button*>(&mediumButton),
                         static_cast<juce::Button*>(&hardButton)})
    {
        addAndMakeVisible(button);
        button->setLookAndFeel(customLookAndFeel.get());
        button->setClickingTogglesState(true);
        button->setRadioGroupId(9102);
    }
    softButton.onClick = [this] { setCharacter(0); };
    mediumButton.onClick = [this] { setCharacter(1); };
    hardButton.onClick = [this] { setCharacter(2); };
    softButton.setTooltip("Soft: widest, roundest shoulder. A higher Knee starts smoothing earlier.");
    mediumButton.setTooltip("Medium: tighter shoulder with a firmer edge than Soft.");
    hardButton.setTooltip("Hard: clips straight to a flat top. Knee has no effect in this mode.");

    for (auto* button : {static_cast<juce::Button*>(&bypassButton),
                         static_cast<juce::Button*>(&deltaButton)})
    {
        addAndMakeVisible(button);
        button->setLookAndFeel(customLookAndFeel.get());
        button->setClickingTogglesState(true);
        button->setColour(juce::ToggleButton::textColourId, paper);
    }
    bypassButton.setTooltip("On: passes the input through without clipping. Off: runs the clipper.");
    deltaButton.setTooltip("On: hear only the difference between the processed signal and the dry input.");
    bypassButton.onClick = [this] { syrupEnergy = 1.0f; };
    deltaButton.onClick = [this] { syrupEnergy = 1.0f; };
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "bypass", bypassButton);
    deltaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "delta", deltaButton);

    presetSelector.addItem("SELECT PRESET", 1);
    presetSelector.addItem("808 WEIGHT", 2);
    presetSelector.addItem("DRUM PUNCH", 3);
    presetSelector.addItem("MEDIUM PUNCH", 4);
    presetSelector.addItem("BUS GLUE", 5);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetSelector.setLookAndFeel(customLookAndFeel.get());
    presetSelector.setTooltip("Choose a starting point: 808 Weight for bass, Drum Punch for drums, Medium Punch for general use, or Bus Glue for a subtle blend. Then adjust Drive and Output by ear.");
    presetSelector.onChange = [this]
    {
        const auto selectedPreset = presetSelector.getSelectedId();
        if (selectedPreset > 1)
        {
            syrupEnergy = 1.0f;
            applyPreset(selectedPreset);
            presetStatusLabel.setText("STARTING POINT LOADED  |  TWEAK IT YOUR WAY", juce::dontSendNotification);
            presetSelector.setSelectedId(1, juce::dontSendNotification);
        }
    };
    addAndMakeVisible(presetSelector);

    for (auto* button : {static_cast<juce::Button*>(&savePresetButton),
                         static_cast<juce::Button*>(&loadPresetButton)})
    {
        addAndMakeVisible(button);
        button->setLookAndFeel(customLookAndFeel.get());
    }
    savePresetButton.setTooltip("Save all current settings to a .dcpreset file in Documents\\TODB\\Double Cup Clipper\\Presets.");
    loadPresetButton.setTooltip("Load a saved Double Cup Clipper .dcpreset file.");
    savePresetButton.onClick = [this] { syrupEnergy = 1.0f; saveUserPreset(); };
    loadPresetButton.onClick = [this] { syrupEnergy = 1.0f; loadUserPreset(); };

    addAndMakeVisible(presetStatusLabel);
    presetStatusLabel.setText("HELLA SAUCE  |  YOUR SOUND, YOUR CALL", juce::dontSendNotification);
    presetStatusLabel.setJustificationType(juce::Justification::centredLeft);
    presetStatusLabel.setColour(juce::Label::textColourId, violet.withAlpha(0.9f));
    presetStatusLabel.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 10.0f,
                                                           juce::Font::bold)
                                              .withFallbacks({"Arial Black", "Arial"})));

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "drive", driveSlider);
    ceilingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "ceiling", ceilingSlider);
    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "knee", kneeSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "mix", mixSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "output", outputSlider);

    addAndMakeVisible(curveDisplay.get());
    addAndMakeVisible(meterDisplay.get());
    curveDisplay->setTooltip("Horizontal axis: signal before Drive. Vertical axis: output. 1.0 is 0 dBFS. The bright curve shows the processed signal; the dashed diagonal is the unprocessed signal. A flatter top means more clipping. Bypass passes the input; Delta plays only what changed.");
    meterDisplay->setTooltip("Input and Output show sample peaks in dBFS. A red Output bar means the signal is above 0 dBFS and may clip in a later plug-in or output. Reduction shows how much level the clipping curve removes.");
    timerCallback();
    startTimerHz(30);
}

KrakenKlipperAudioProcessorEditor::~KrakenKlipperAudioProcessorEditor()
{
    stopTimer();
    tooltips.setLookAndFeel(nullptr);
    for (auto* slider : {&driveSlider, &ceilingSlider, &kneeSlider, &mixSlider, &outputSlider})
        slider->setLookAndFeel(nullptr);
    for (auto* button : {static_cast<juce::Button*>(&softButton),
                         static_cast<juce::Button*>(&mediumButton),
                         static_cast<juce::Button*>(&hardButton),
                         static_cast<juce::Button*>(&bypassButton),
                         static_cast<juce::Button*>(&deltaButton)})
        button->setLookAndFeel(nullptr);
    presetSelector.setLookAndFeel(nullptr);
    savePresetButton.setLookAndFeel(nullptr);
    loadPresetButton.setLookAndFeel(nullptr);
}

void KrakenKlipperAudioProcessorEditor::styleSlider(juce::Slider& slider, juce::Label& label,
                                                      const juce::String& name)
{
    addAndMakeVisible(slider);
    slider.setLookAndFeel(customLookAndFeel.get());
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 88, 23);
    slider.onDragStart = [this] { syrupEnergy = 1.0f; };
    slider.setColour(juce::Slider::thumbColourId, gold);
    slider.setColour(juce::Slider::rotarySliderFillColourId, purple);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, panelEdge);
    addAndMakeVisible(label);
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, paper);
    label.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 12.0f,
                                               juce::Font::bold)
                                  .withFallbacks({"Arial Black", "Arial"})));
    label.attachToComponent(&slider, false);
}

void KrakenKlipperAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto width = static_cast<float>(getWidth());
    g.fillAll(ink);
    if (panelMural.isValid())
        g.drawImageWithin(panelMural, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);

    // The entire shell is a double foam-cup rim with glossy purple liquid.
    g.setColour(juce::Colour(0xff1b0630).withAlpha(0.30f));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(25.0f, 10.0f), 36.0f);
    if (headerArtwork.isValid())
        g.drawImageWithin(headerArtwork, 18, 0, getWidth() - 238, 238,
                          juce::RectanglePlacement::xLeft | juce::RectanglePlacement::yMid);
    g.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 12.0f, juce::Font::bold)));
    g.setColour(gold);
    g.drawText("TODB / THE BAY", getWidth() - 214, 60, 177, 25, juce::Justification::centredRight);
    g.setColour(paper);
    g.drawText("8X OVERSAMPLING", getWidth() - 214, 92, 177, 18, juce::Justification::centredRight);
    g.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 10.0f, juce::Font::bold)));
    g.drawText("AUTO MONO / STEREO", getWidth() - 214, 117, 177, 18, juce::Justification::centredRight);
    g.setColour(gold);
    g.drawText("510 / 415 / 707", getWidth() - 214, 145, 177, 18, juce::Justification::centredRight);
    g.setColour(violet);
    g.drawText("HELLA SAUCE. YADADAMEAN?", getWidth() - 224, 172, 187, 18, juce::Justification::centredRight);

    const auto controls = juce::Rectangle<float>(22.0f, 253.0f, width - 44.0f, 258.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff35124f).withAlpha(0.92f), controls.getTopLeft(),
                                          juce::Colour(0xff15081f).withAlpha(0.95f), controls.getBottomRight(), false));
    g.fillRoundedRectangle(controls, 20.0f);
    g.setColour(gold.withAlpha(0.78f));
    g.drawRoundedRectangle(controls, 20.0f, 1.5f);
    g.setColour(violet.withAlpha(0.28f));
    g.drawRoundedRectangle(controls.reduced(4.0f), 17.0f, 1.0f);

    // Small waves and ballistic droplets occupy gutters, clear of text and hit targets.
    if (syrupEnergy > 0.01f)
    {
        g.saveState();
        juce::Path clip;
        clip.addRoundedRectangle(controls.reduced(2.0f), 18.0f);
        g.reduceClipRegion(clip);
        juce::Path ripple;
        for (int point = 0; point <= 90; ++point)
        {
            const auto x = controls.getX() + static_cast<float>(point) * controls.getWidth() / 90.0f;
            const auto y = controls.getBottom() - 6.0f + std::sin(point * 0.16f + syrupPhase) * syrupEnergy * 4.0f;
            if (point == 0) ripple.startNewSubPath(x, y); else ripple.lineTo(x, y);
        }
        g.setColour(purple.withAlpha(syrupEnergy * 0.75f));
        g.strokePath(ripple, juce::PathStrokeType(6.0f));
        g.setColour(gold.withAlpha(syrupEnergy * 0.82f));
        g.strokePath(ripple, juce::PathStrokeType(1.3f));
        for (int i = 0; i < 14; ++i)
        {
            const auto progress = std::fmod(syrupPhase * 0.12f + i * 0.071f, 1.0f);
            const auto x = i % 2 == 0 ? controls.getX() + 10.0f : controls.getRight() - 10.0f;
            const auto y = controls.getBottom() - 12.0f - std::sin(progress * juce::MathConstants<float>::pi) * 135.0f;
            const auto radius = (2.0f + static_cast<float>(i % 3)) * syrupEnergy;
            g.setColour((i % 3 == 0 ? gold : violet).withAlpha(syrupEnergy * 0.80f));
            g.fillEllipse(x - radius, y - radius, radius * 2.0f, radius * 2.0f);
        }
        g.restoreState();
    }
    const auto modeCard = juce::Rectangle<float>(38.0f, 265.0f, 365.0f, 60.0f);
    g.setColour(ink.withAlpha(0.85f));
    g.fillRoundedRectangle(modeCard, 10.0f);
    g.setColour(gold.withAlpha(0.45f));
    g.drawRoundedRectangle(modeCard, 10.0f, 1.0f);
    g.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 11.0f, juce::Font::bold)));
    g.setColour(gold);
    g.drawText("CLIP STYLE / PICK YOUR SAUCE", 42, 271, 265, 16, juce::Justification::centredLeft);

    const auto controlW = (getWidth() - 120) / 5;
    const std::array<juce::String, 5> hints{{"PUSH INTO THE CLIP", "TOP LIMIT", "ROUND THE EDGE",
                                            "DRY / WET BLEND", "AFTER THE CLIP"}};
    g.setColour(paper);
    g.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 9.0f, juce::Font::bold)));
    for (int i = 0; i < 5; ++i)
        g.drawText(hints[static_cast<size_t>(i)], 56 + i * controlW, 491, controlW - 2, 13,
                   juce::Justification::centred);

    const auto lowerY = 522.0f;
    const auto lowerHeight = static_cast<float>(getHeight() - 555);
    for (const auto card : {juce::Rectangle<float>(45.0f, lowerY, width - 426.0f, lowerHeight),
                           juce::Rectangle<float>(width - 366.0f, lowerY, 334.0f, lowerHeight)})
    {
        g.setColour(juce::Colour(0xff220c37).withAlpha(0.94f));
        g.fillRoundedRectangle(card, 16.0f);
        g.setColour(gold.withAlpha(0.58f));
        g.drawRoundedRectangle(card, 16.0f, 1.2f);
    }
    const auto tagFrame = juce::Rectangle<float>(width - 253.0f, static_cast<float>(getHeight() - 140), 221.0f, 98.0f);
    g.setColour(ink);
    g.fillRoundedRectangle(tagFrame, 7.0f);
    if (todbTag.isValid())
    {
        g.saveState();
        g.reduceClipRegion(tagFrame.toNearestInt().reduced(2));
        g.drawImage(todbTag, static_cast<int>(tagFrame.getX() + 2.0f), static_cast<int>(tagFrame.getY() + 2.0f),
                    217, 94, 0, static_cast<int>(std::lround(todbTag.getHeight() * 0.211f)),
                    todbTag.getWidth(), static_cast<int>(std::lround(todbTag.getHeight() * 0.646f)), false);
        g.restoreState();
    }
    g.setColour(gold);
    g.drawRoundedRectangle(tagFrame, 7.0f, 1.2f);
    const auto tagX = getWidth() - 354;
    const auto tagY = getHeight() - 134;
    g.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 13.0f, juce::Font::bold)));
    g.drawText("THE BAY", tagX, tagY, 94, 20, juce::Justification::centredLeft);
    g.setColour(violet);
    g.drawText("HELLA SAUCE", tagX, tagY + 25, 94, 20, juce::Justification::centredLeft);
    g.setColour(paper);
    g.setFont(juce::Font(juce::FontOptions("Trebuchet MS", 9.0f, juce::Font::bold)));
    g.drawText("510 / 415 / 707", tagX, tagY + 51, 94, 18, juce::Justification::centredLeft);
    g.drawText("TWON ON DA BEAT", tagX, tagY + 72, 94, 15, juce::Justification::centredLeft);
    const auto footer = juce::Rectangle<int>(24, getHeight() - 24, getWidth() - 48, 20);
    g.setColour(ink.withAlpha(0.9f));
    g.fillRoundedRectangle(footer.toFloat(), 5.0f);
    g.setColour(gold);
    g.drawText("TODB / TWON ON DA BEAT | DOUBLE CUP CLIPPER | 8X REAL-TIME OVERSAMPLING",
               footer.reduced(12, 0), juce::Justification::centredLeft);
}

void KrakenKlipperAudioProcessorEditor::resized()
{
    const auto width = getWidth();
    softButton.setBounds(47, 293, 103, 31);
    mediumButton.setBounds(158, 293, 112, 31);
    hardButton.setBounds(278, 293, 112, 31);
    bypassButton.setBounds(width - 420, 266, 105, 37);
    deltaButton.setBounds(width - 300, 266, 94, 37);
    presetSelector.setBounds(width - 196, 266, 163, 34);
    savePresetButton.setBounds(width - 196, 304, 78, 22);
    loadPresetButton.setBounds(width - 113, 304, 80, 22);
    presetStatusLabel.setBounds(412, 304, width - 620, 22);

    const auto controlW = (width - 120) / 5;
    const auto firstX = 56;
    const auto y = 344;
    const auto h = 146;
    auto place = [&](juce::Slider& slider, int index)
    {
        slider.setBounds(firstX + index * controlW, y, controlW - 2, h);
    };
    place(driveSlider, 0);
    place(ceilingSlider, 1);
    place(kneeSlider, 2);
    place(mixSlider, 3);
    place(outputSlider, 4);

    curveDisplay->setBounds(62, 537, width - 450, getHeight() - 595);
    meterDisplay->setBounds(width - 354, 537, 322, 112);
}

void KrakenKlipperAudioProcessorEditor::timerCallback()
{
    const std::array<const char*, 8> ids{{"drive", "ceiling", "knee", "mix", "output", "character", "bypass", "delta"}};
    bool changed = false;
    for (size_t i = 0; i < ids.size(); ++i)
    {
        const auto value = processor.parameters.getRawParameterValue(ids[i])->load();
        if (visualParametersInitialised && std::abs(value - lastVisualParameters[i]) > 0.0001f)
            changed = true;
        lastVisualParameters[i] = value;
    }
    visualParametersInitialised = true;
    const auto style = static_cast<int>(std::lround(lastVisualParameters[5]));
    softButton.setToggleState(style == 0, juce::dontSendNotification);
    mediumButton.setToggleState(style == 1, juce::dontSendNotification);
    hardButton.setToggleState(style == 2, juce::dontSendNotification);
    if (changed) syrupEnergy = 1.0f;
    const bool moving = syrupEnergy > 0.01f;
    if (moving)
    {
        syrupPhase = std::fmod(syrupPhase + 0.22f, juce::MathConstants<float>::twoPi * 10.0f);
        syrupEnergy *= 0.93f;
    }
    else syrupEnergy = 0.0f;
    auto& look = static_cast<KrakenLookAndFeel&>(*customLookAndFeel);
    look.liquidPhase = syrupPhase;
    look.liquidEnergy = syrupEnergy;
    if (moving || changed) repaint();
}

void KrakenKlipperAudioProcessorEditor::setCharacter(int index)
{
    syrupEnergy = 1.0f;
    setPluginParameter("character", static_cast<float>(index));
    timerCallback();
}

void KrakenKlipperAudioProcessorEditor::setPluginParameter(const juce::String& id, float value)
{
    if (auto* parameter = processor.parameters.getParameter(id))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

void KrakenKlipperAudioProcessorEditor::applyPreset(int preset)
{
    struct Values { float drive, ceiling, knee, mix, output, character; };
    Values values{};
    switch (preset)
    {
        case 2: values = {8.0f, -3.0f, 18.0f, 100.0f, -1.0f, 0.0f}; break; // 808 Weight
        case 3: values = {6.0f, -2.0f, 5.0f, 88.0f, -1.0f, 2.0f}; break;  // Drum Punch
        case 4: values = {5.0f, -1.0f, 10.0f, 100.0f, 0.0f, 1.0f}; break; // Medium Punch
        case 5: values = {2.0f, -4.0f, 22.0f, 62.0f, 0.0f, 0.0f}; break; // Bus Glue
        default: return;
    }
    setPluginParameter("drive", values.drive);
    setPluginParameter("ceiling", values.ceiling);
    setPluginParameter("knee", values.knee);
    setPluginParameter("mix", values.mix);
    setPluginParameter("output", values.output);
    setPluginParameter("character", values.character);
    setPluginParameter("bypass", 0.0f);
    setPluginParameter("delta", 0.0f);
}

void KrakenKlipperAudioProcessorEditor::saveUserPreset()
{
    auto presetFolder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                            .getChildFile("TODB")
                            .getChildFile("Double Cup Clipper")
                            .getChildFile("Presets");
    if (presetFolder.createDirectory().failed())
    {
        presetStatusLabel.setText("COULDN'T CREATE THE PRESET FOLDER", juce::dontSendNotification);
        return;
    }

    activePresetChooser = std::make_shared<juce::FileChooser>(
        "Save a Double Cup Clipper preset", presetFolder.getChildFile("My-Double-Cup-Preset.dcpreset"),
        "*.dcpreset");
    auto chooser = activePresetChooser;
    juce::Component::SafePointer<KrakenKlipperAudioProcessorEditor> safeThis(this);
    chooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis](const juce::FileChooser& fileChooser)
        {
            if (safeThis == nullptr)
                return;

            const auto selected = fileChooser.getResult();
            if (selected == juce::File{})
            {
                safeThis->presetStatusLabel.setText("SAVE CANCELLED", juce::dontSendNotification);
                return;
            }

            auto outputFile = selected;
            if (!outputFile.getFileExtension().equalsIgnoreCase(".dcpreset"))
                outputFile = outputFile.withFileExtension("dcpreset");

            if (auto xml = safeThis->processor.parameters.copyState().createXml())
            {
                if (outputFile.replaceWithText(xml->toString()))
                    safeThis->presetStatusLabel.setText("SAVED  |  " + outputFile.getFileName(),
                                                        juce::dontSendNotification);
                else
                    safeThis->presetStatusLabel.setText("COULDN'T WRITE THAT PRESET FILE",
                                                        juce::dontSendNotification);
            }
            else
            {
                safeThis->presetStatusLabel.setText("COULDN'T BUILD THE PRESET FILE",
                                                    juce::dontSendNotification);
            }
        });
}

void KrakenKlipperAudioProcessorEditor::loadUserPreset()
{
    const auto presetFolder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                  .getChildFile("TODB")
                                  .getChildFile("Double Cup Clipper")
                                  .getChildFile("Presets");
    activePresetChooser = std::make_shared<juce::FileChooser>(
        "Load a Double Cup Clipper preset", presetFolder, "*.dcpreset");
    auto chooser = activePresetChooser;
    juce::Component::SafePointer<KrakenKlipperAudioProcessorEditor> safeThis(this);
    chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis](const juce::FileChooser& fileChooser)
        {
            if (safeThis == nullptr)
                return;

            const auto selected = fileChooser.getResult();
            if (selected == juce::File{})
            {
                safeThis->presetStatusLabel.setText("LOAD CANCELLED", juce::dontSendNotification);
                return;
            }

            const auto xml = juce::XmlDocument::parse(selected);
            if (xml == nullptr || !xml->hasTagName(safeThis->processor.parameters.state.getType()))
            {
                safeThis->presetStatusLabel.setText("THAT FILE ISN'T A DOUBLE CUP PRESET",
                                                    juce::dontSendNotification);
            }
            else
            {
                auto state = juce::ValueTree::fromXml(*xml);
                safeThis->processor.parameters.replaceState(state);
                safeThis->presetSelector.setSelectedId(1, juce::dontSendNotification);
                safeThis->presetStatusLabel.setText("PRESET LOADED  |  YADADAMEAN?",
                                                    juce::dontSendNotification);
            }
        });
}

