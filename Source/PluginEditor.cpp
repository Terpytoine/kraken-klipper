#include "PluginEditor.h"
#include "ClipperCurve.h"
#include <array>

namespace
{
const juce::Colour ink{0xff090612};
const juce::Colour panel{0xff151023};
const juce::Colour panelEdge{0xff3c2a56};
const juce::Colour purple{0xff9d54ed};
const juce::Colour violet{0xffc488ff};
const juce::Colour gold{0xffded0f0};
const juce::Colour paper{0xfff0eaf7};
const juce::Colour muted{0xffaaa0ba};

class KrakenLookAndFeel final : public juce::LookAndFeel_V4
{
public:
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
        g.setColour(juce::Colour(0xff5f476e));
        g.drawEllipse(knob.reduced(5.0f), 1.0f);

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
        g.setColour(purple);
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
        g.setColour(fill);
        g.fillRoundedRectangle(bounds, 8.0f);
        g.setColour(selected ? gold : panelEdge);
        g.drawRoundedRectangle(bounds, 8.0f, selected ? 1.7f : 1.0f);
        if (selected)
        {
            g.setColour(purple.withAlpha(0.34f));
            g.fillRoundedRectangle(bounds.reduced(3.0f), 6.0f);
        }
    }

    juce::Font getTextButtonFont(juce::TextButton&, int height) override
    {
        return juce::Font(juce::FontOptions(juce::jlimit(11.0f, 16.0f, height * 0.33f),
                                            juce::Font::bold));
    }
};

void drawLightning(juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::Path bolt;
    bolt.startNewSubPath(area.getX() + area.getWidth() * 0.58f, area.getY());
    bolt.lineTo(area.getX() + area.getWidth() * 0.17f, area.getY() + area.getHeight() * 0.54f);
    bolt.lineTo(area.getX() + area.getWidth() * 0.47f, area.getY() + area.getHeight() * 0.54f);
    bolt.lineTo(area.getX() + area.getWidth() * 0.31f, area.getBottom());
    bolt.lineTo(area.getX() + area.getWidth() * 0.83f, area.getY() + area.getHeight() * 0.38f);
    bolt.lineTo(area.getX() + area.getWidth() * 0.55f, area.getY() + area.getHeight() * 0.38f);
    bolt.closeSubPath();
    g.setColour(purple.withAlpha(0.27f));
    g.fillPath(bolt, juce::AffineTransform::scale(1.18f).translated(-area.getWidth() * 0.09f, -area.getHeight() * 0.03f));
    g.setColour(gold);
    g.fillPath(bolt);
}

void drawBayBridge(juce::Graphics& g, juce::Rectangle<float> area)
{
    const auto y = area.getY() + area.getHeight() * 0.61f;
    const auto leftTower = area.getX() + area.getWidth() * 0.31f;
    const auto rightTower = area.getX() + area.getWidth() * 0.70f;
    const auto towerTop = y - area.getHeight() * 0.30f;
    const auto bridgeColour = juce::Colour(0xff9c66c8).withAlpha(0.34f);

    g.setColour(bridgeColour);
    g.drawLine(area.getX(), y, area.getRight(), y, 2.0f);
    g.drawLine(area.getX(), y + 4.0f, area.getRight(), y + 4.0f, 1.0f);
    for (auto towerX : {leftTower, rightTower})
    {
        g.drawLine(towerX - 5.0f, towerTop, towerX - 5.0f, y + 3.0f, 2.4f);
        g.drawLine(towerX + 5.0f, towerTop, towerX + 5.0f, y + 3.0f, 2.4f);
        g.drawLine(towerX - 8.0f, towerTop + 14.0f, towerX + 8.0f, towerTop + 14.0f, 1.6f);
        g.drawLine(towerX - 8.0f, towerTop + 30.0f, towerX + 8.0f, towerTop + 30.0f, 1.6f);
        g.drawLine(towerX - 8.0f, towerTop, towerX + 8.0f, towerTop, 2.0f);
    }

    juce::Path cable;
    cable.startNewSubPath(area.getX(), y - 2.0f);
    cable.cubicTo(leftTower - 18.0f, towerTop + 2.0f, leftTower - 7.0f, towerTop - 3.0f,
                  leftTower, towerTop);
    cable.cubicTo(leftTower + 38.0f, towerTop + 4.0f, rightTower - 45.0f, towerTop + 8.0f,
                  rightTower, towerTop);
    cable.cubicTo(rightTower + 35.0f, towerTop + 7.0f, rightTower + 58.0f, y - 5.0f,
                  area.getRight(), y - 2.0f);
    g.strokePath(cable, juce::PathStrokeType(1.5f));

    for (auto fraction = 0.10f; fraction < 0.98f; fraction += 0.035f)
    {
        const auto px = area.getX() + area.getWidth() * fraction;
        if (std::abs(px - leftTower) < 10.0f || std::abs(px - rightTower) < 10.0f)
            continue;
        const auto d = px < leftTower ? (px - area.getX()) / (leftTower - area.getX())
                    : px < rightTower ? (px - leftTower) / (rightTower - leftTower)
                                      : (px - rightTower) / (area.getRight() - rightTower);
        const auto topY = px < leftTower ? y - d * (y - towerTop)
                         : px < rightTower ? towerTop + std::sin(d * juce::MathConstants<float>::pi) * 5.0f
                                           : towerTop + d * (y - towerTop);
        g.drawLine(px, topY, px, y, 0.55f);
    }
}

void drawKraken(juce::Graphics& g, juce::Rectangle<float> area)
{
    const auto cx = area.getCentreX();
    const auto cy = area.getCentreY();
    const auto scale = juce::jmin(area.getWidth(), area.getHeight()) / 270.0f;
    const auto inkPurple = juce::Colour(0xff401858).withAlpha(0.31f);
    const auto linePurple = juce::Colour(0xffa354dc).withAlpha(0.37f);
    const auto suckerGold = gold.withAlpha(0.55f);

    const std::array<std::array<juce::Point<float>, 4>, 8> tentacles{{
        {{{cx - 26*scale, cy + 26*scale}, {cx - 80*scale, cy + 44*scale}, {cx - 106*scale, cy + 10*scale}, {cx - 123*scale, cy + 52*scale}}},
        {{{cx - 34*scale, cy + 36*scale}, {cx - 77*scale, cy + 78*scale}, {cx - 39*scale, cy + 85*scale}, {cx - 89*scale, cy + 112*scale}}},
        {{{cx - 20*scale, cy + 43*scale}, {cx - 45*scale, cy + 96*scale}, {cx - 7*scale, cy + 105*scale}, {cx - 43*scale, cy + 129*scale}}},
        {{{cx - 8*scale, cy + 48*scale}, {cx - 16*scale, cy + 108*scale}, {cx + 15*scale, cy + 112*scale}, {cx + 2*scale, cy + 137*scale}}},
        {{{cx + 27*scale, cy + 32*scale}, {cx + 81*scale, cy + 46*scale}, {cx + 104*scale, cy + 11*scale}, {cx + 126*scale, cy + 52*scale}}},
        {{{cx + 34*scale, cy + 42*scale}, {cx + 87*scale, cy + 75*scale}, {cx + 53*scale, cy + 95*scale}, {cx + 97*scale, cy + 112*scale}}},
        {{{cx + 18*scale, cy + 45*scale}, {cx + 43*scale, cy + 97*scale}, {cx + 9*scale, cy + 107*scale}, {cx + 48*scale, cy + 130*scale}}},
        {{{cx + 4*scale, cy + 49*scale}, {cx + 16*scale, cy + 106*scale}, {cx - 14*scale, cy + 114*scale}, {cx - 1*scale, cy + 139*scale}}}
    }};

    for (size_t i = 0; i < tentacles.size(); ++i)
    {
        const auto& points = tentacles[i];
        juce::Path arm;
        arm.startNewSubPath(points[0]);
        arm.cubicTo(points[1], points[2], points[3]);
        g.setColour(juce::Colour(0x50000000));
        g.strokePath(arm, juce::PathStrokeType((16.0f - static_cast<float>(i % 3) * 1.8f) * scale,
                                               juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(inkPurple);
        g.strokePath(arm, juce::PathStrokeType((12.0f - static_cast<float>(i % 3) * 1.5f) * scale,
                                               juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(linePurple);
        g.strokePath(arm, juce::PathStrokeType(1.2f * scale, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        for (auto t = 0.30f; t < 0.91f; t += 0.12f)
        {
            const auto u = 1.0f - t;
            const auto px = u*u*u*points[0].x + 3*u*u*t*points[1].x + 3*u*t*t*points[2].x + t*t*t*points[3].x;
            const auto py = u*u*u*points[0].y + 3*u*u*t*points[1].y + 3*u*t*t*points[2].y + t*t*t*points[3].y;
            const auto radius = 1.6f * scale;
            g.setColour(suckerGold);
            g.fillEllipse(px - radius, py - radius, radius * 2.0f, radius * 2.0f);
        }
    }

    auto head = juce::Rectangle<float>(cx - 42*scale, cy - 40*scale, 84*scale, 89*scale);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff632c84).withAlpha(0.46f), head.getTopLeft(),
                                           juce::Colour(0xff160b26).withAlpha(0.58f), head.getBottomRight(), false));
    g.fillEllipse(head);
    g.setColour(juce::Colour(0xffbb82df).withAlpha(0.48f));
    g.drawEllipse(head, 1.5f * scale);

    for (auto eyeX : {cx - 17*scale, cx + 17*scale})
    {
        g.setColour(juce::Colour(0xff160a1e));
        g.fillEllipse(eyeX - 5*scale, cy - 9*scale, 10*scale, 7*scale);
        g.setColour(gold.withAlpha(0.77f));
        g.fillEllipse(eyeX - 1.5f*scale, cy - 8*scale, 3.0f*scale, 5.0f*scale);
    }

    // Small brow ridges and decorative flash-like curls keep the mascot original.
    g.setColour(juce::Colour(0xffdfb7ff).withAlpha(0.33f));
    for (auto side : {-1.0f, 1.0f})
    {
        juce::Path ridge;
        ridge.startNewSubPath(cx + side * 25*scale, cy - 24*scale);
        ridge.quadraticTo(cx + side * 7*scale, cy - 48*scale, cx - side * 3*scale, cy - 34*scale);
        ridge.quadraticTo(cx + side * 4*scale, cy - 22*scale, cx + side * 14*scale, cy - 25*scale);
        g.strokePath(ridge, juce::PathStrokeType(1.4f * scale));
    }
}
}

class KrakenCurveDisplay final : public juce::Component, private juce::Timer
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
            g.drawLine(x, plot.getY(), x, plot.getBottom(), 1.0f);
            g.drawLine(plot.getX(), y, plot.getRight(), y, 1.0f);
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

        juce::Path curve;
        for (int i = 0; i <= 180; ++i)
        {
            const auto u = static_cast<float>(i) / 180.0f;
            const auto input = (u * 2.0f - 1.0f) * 1.4f;
            const auto clipped = kraken::clipBlend(input * drive, ceiling, kneeRatio, styleValue);
            const auto output = juce::jlimit(-ceiling, ceiling,
                (input + (clipped - input) * mix) * trim);
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
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
        g.drawText("TRANSFER CURVE", area.removeFromTop(22.0f).reduced(15.0f, 0.0f).toNearestInt(),
                   juce::Justification::centredLeft);
        g.drawText("INPUT", juce::Rectangle<int>(static_cast<int>(plot.getX()), getHeight() - 20,
                                                  static_cast<int>(plot.getWidth()), 14),
                   juce::Justification::centred);
    }

private:
    void timerCallback() override { repaint(); }
    KrakenKlipperAudioProcessor& processor;
};

class KrakenMeterDisplay final : public juce::Component, private juce::Timer
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

        auto inner = getLocalBounds().reduced(16, 14);
        const auto labelsWidth = 42;
        auto drawMeter = [&](const juce::String& label, float db, int row, const juce::Colour& colour)
        {
            auto line = inner.withY(inner.getY() + row * 35).withHeight(22);
            g.setColour(muted);
            g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
            g.drawText(label, line.removeFromLeft(labelsWidth), juce::Justification::centredLeft);
            auto bar = line.removeFromLeft(line.getWidth() - 48).toFloat();
            g.setColour(juce::Colour(0xff21192c));
            g.fillRoundedRectangle(bar, 5.0f);
            const auto normalized = juce::jlimit(0.0f, 1.0f, (db + 36.0f) / 36.0f);
            g.setColour(colour);
            g.fillRoundedRectangle(bar.withWidth(bar.getWidth() * normalized), 5.0f);
            g.setColour(paper);
            g.setFont(juce::Font(juce::FontOptions(10.0f)));
            const auto value = row == 2 ? juce::String(db, 1) + " dB" : juce::String(db, 1) + " dBFS";
            g.drawText(value, line, juce::Justification::centredRight);
        };

        drawMeter("IN", processor.getInputPeakDb(), 0, purple);
        drawMeter("OUT", processor.getOutputPeakDb(), 1, gold);

        auto grLine = inner.withY(inner.getY() + 70).withHeight(22);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText("CLIP", grLine.removeFromLeft(labelsWidth), juce::Justification::centredLeft);
        auto grBar = grLine.removeFromLeft(grLine.getWidth() - 48).toFloat();
        g.setColour(juce::Colour(0xff21192c));
        g.fillRoundedRectangle(grBar, 5.0f);
        const auto reduction = juce::jlimit(0.0f, 18.0f, processor.getGainReductionDb());
        g.setColour(violet);
        g.fillRoundedRectangle(grBar.withWidth(grBar.getWidth() * reduction / 18.0f), 5.0f);
        g.setColour(paper);
        g.setFont(juce::Font(juce::FontOptions(10.0f)));
        g.drawText(juce::String(reduction, 1) + " dB", grLine, juce::Justification::centredRight);
    }

private:
    void timerCallback() override { repaint(); }
    KrakenKlipperAudioProcessor& processor;
};

KrakenKlipperAudioProcessorEditor::KrakenKlipperAudioProcessorEditor(KrakenKlipperAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p),
      customLookAndFeel(std::make_unique<KrakenLookAndFeel>()),
      curveDisplay(std::make_unique<KrakenCurveDisplay>(p)),
      meterDisplay(std::make_unique<KrakenMeterDisplay>(p))
{
    setSize(1040, 650);
    setResizable(true, true);
    setResizeLimits(900, 590, 1350, 900);

    styleSlider(driveSlider, driveLabel, "DRIVE");
    styleSlider(ceilingSlider, ceilingLabel, "CEILING");
    styleSlider(kneeSlider, kneeLabel, "KNEE");
    styleSlider(mixSlider, mixLabel, "MIX");
    styleSlider(outputSlider, outputLabel, "OUTPUT");
    driveSlider.setTextValueSuffix(" dB");
    ceilingSlider.setTextValueSuffix(" dBFS");
    kneeSlider.setTextValueSuffix(" dB");
    mixSlider.setTextValueSuffix(" %");
    outputSlider.setTextValueSuffix(" dB");

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

    for (auto* button : {static_cast<juce::Button*>(&bypassButton),
                         static_cast<juce::Button*>(&deltaButton)})
    {
        addAndMakeVisible(button);
        button->setLookAndFeel(customLookAndFeel.get());
        button->setClickingTogglesState(true);
        button->setColour(juce::ToggleButton::textColourId, paper);
    }
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "bypass", bypassButton);
    deltaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "delta", deltaButton);

    presetSelector.addItem("SELECT A STARTING POINT", 1);
    presetSelector.addItem("808 REEF", 2);
    presetSelector.addItem("DRUM SNAP", 3);
    presetSelector.addItem("VIOLET PUNCH", 4);
    presetSelector.addItem("BUS GLIDE", 5);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetSelector.setLookAndFeel(customLookAndFeel.get());
    presetSelector.onChange = [this]
    {
        if (presetSelector.getSelectedId() > 1)
            applyPreset(presetSelector.getSelectedId());
    };
    addAndMakeVisible(presetSelector);

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "drive", driveSlider);
    ceilingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "ceiling", ceilingSlider);
    kneeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "knee", kneeSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "mix", mixSlider);
    outputAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "output", outputSlider);

    addAndMakeVisible(curveDisplay.get());
    addAndMakeVisible(meterDisplay.get());
    timerCallback();
    startTimerHz(24);
}

KrakenKlipperAudioProcessorEditor::~KrakenKlipperAudioProcessorEditor()
{
    stopTimer();
    for (auto* slider : {&driveSlider, &ceilingSlider, &kneeSlider, &mixSlider, &outputSlider})
        slider->setLookAndFeel(nullptr);
    for (auto* button : {static_cast<juce::Button*>(&softButton),
                         static_cast<juce::Button*>(&mediumButton),
                         static_cast<juce::Button*>(&hardButton),
                         static_cast<juce::Button*>(&bypassButton),
                         static_cast<juce::Button*>(&deltaButton)})
        button->setLookAndFeel(nullptr);
    presetSelector.setLookAndFeel(nullptr);
}

void KrakenKlipperAudioProcessorEditor::styleSlider(juce::Slider& slider, juce::Label& label,
                                                      const juce::String& name)
{
    addAndMakeVisible(slider);
    slider.setLookAndFeel(customLookAndFeel.get());
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 88, 23);
    slider.setDoubleClickReturnValue(true, 0.0);
    slider.setColour(juce::Slider::thumbColourId, gold);
    slider.setColour(juce::Slider::rotarySliderFillColourId, purple);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, panelEdge);
    addAndMakeVisible(label);
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, paper);
    label.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    label.attachToComponent(&slider, false);
}

void KrakenKlipperAudioProcessorEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff10091b), bounds.getTopLeft(),
                                           juce::Colour(0xff07060c), bounds.getBottomRight(), false));
    g.fillAll();

    juce::ColourGradient ambient(juce::Colour(0xff51236c).withAlpha(0.32f), bounds.getWidth() * 0.78f,
                                 bounds.getHeight() * 0.14f, juce::Colour(0xff10091b).withAlpha(0.0f),
                                 bounds.getWidth() * 0.78f, bounds.getHeight() * 0.78f, true);
    g.setGradientFill(ambient);
    g.fillRect(bounds);

    auto header = bounds.removeFromTop(102.0f);
    g.setColour(juce::Colour(0xff120c1e));
    g.fillRect(header);
    g.setColour(gold.withAlpha(0.75f));
    g.drawLine(0.0f, 101.0f, static_cast<float>(getWidth()), 101.0f, 1.2f);

    auto artArea = juce::Rectangle<float>(getWidth() * 0.54f, 14.0f,
                                          getWidth() * 0.22f, 88.0f);
    drawBayBridge(g, artArea);
    drawKraken(g, juce::Rectangle<float>(getWidth() * 0.68f, 10.0f,
                                         getWidth() * 0.09f, 91.0f));

    auto logoTile = juce::Rectangle<float>(22.0f, 17.0f, 76.0f, 67.0f);
    g.setColour(juce::Colour(0xff1e112b));
    g.fillRoundedRectangle(logoTile, 12.0f);
    g.setColour(gold);
    g.drawRoundedRectangle(logoTile, 12.0f, 1.4f);
    drawLightning(g, juce::Rectangle<float>(logoTile.getX() + 8.0f, logoTile.getY() + 12.0f,
                                            19.0f, 42.0f));
    g.setColour(paper);
    g.setFont(juce::Font(juce::FontOptions(23.0f, juce::Font::bold)));
    g.drawText("KK", juce::Rectangle<int>(static_cast<int>(logoTile.getX() + 28.0f),
                                            static_cast<int>(logoTile.getY() + 8.0f), 43, 50),
               juce::Justification::centred);
    g.setColour(gold.withAlpha(0.7f));
    g.drawLine(33.0f, 79.0f, 86.0f, 79.0f, 1.2f);

    g.setColour(paper);
    g.setFont(juce::Font(juce::FontOptions(26.0f, juce::Font::bold)));
    g.drawText("KRAKEN KLIPPER", 112, 20, 440, 36, juce::Justification::centredLeft);
    g.setColour(violet);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("TODB  •  TWON ON DA BEAT", 115, 56, 400, 19, juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("SAN FRANCISCO BAY • GOLDEN GATE TO THE DEEP", 115, 75, 440, 16,
               juce::Justification::centredLeft);

    g.setColour(gold.withAlpha(0.92f));
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("8× OVERSAMPLING", getWidth() - 180, 22, 150, 16, juce::Justification::centredRight);
    g.setColour(muted);
    g.drawText("VST3  •  STEREO / MONO", getWidth() - 205, 43, 175, 15, juce::Justification::centredRight);
    g.setColour(purple.withAlpha(0.5f));
    g.drawText("BAY AREA", getWidth() - 180, 65, 150, 14, juce::Justification::centredRight);

    auto controlsCard = juce::Rectangle<float>(22.0f, 113.0f, getWidth() - 44.0f, 258.0f);
    g.setColour(panel.withAlpha(0.90f));
    g.fillRoundedRectangle(controlsCard, 16.0f);
    g.setColour(panelEdge);
    g.drawRoundedRectangle(controlsCard, 16.0f, 1.1f);

    auto modeCard = juce::Rectangle<float>(38.0f, 125.0f, 365.0f, 50.0f);
    g.setColour(juce::Colour(0xff100c18));
    g.fillRoundedRectangle(modeCard, 10.0f);
    g.setColour(gold.withAlpha(0.22f));
    g.drawRoundedRectangle(modeCard, 10.0f, 1.0f);

    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("CHARACTER", 42, 131, 94, 16, juce::Justification::centredLeft);
    g.setColour(gold.withAlpha(0.26f));
    g.drawLine(42.0f, 151.0f, 397.0f, 151.0f, 1.0f);

    g.setColour(juce::Colour(0xff1b1028).withAlpha(0.78f));
    const auto lowerCardY = 382.0f;
    const auto lowerCardHeight = 235.0f;
    const auto leftCardWidth = static_cast<float>(getWidth() - 426);
    g.fillRoundedRectangle(juce::Rectangle<float>(45.0f, lowerCardY, leftCardWidth, lowerCardHeight), 16.0f);
    g.setColour(panelEdge.withAlpha(0.8f));
    g.drawRoundedRectangle(juce::Rectangle<float>(45.0f, lowerCardY, leftCardWidth, lowerCardHeight), 16.0f, 1.0f);

    g.setColour(juce::Colour(0xff1b1028).withAlpha(0.90f));
    const auto rightCardX = static_cast<float>(getWidth() - 366);
    g.fillRoundedRectangle(juce::Rectangle<float>(rightCardX, lowerCardY, 334.0f, lowerCardHeight), 16.0f);
    g.setColour(panelEdge.withAlpha(0.8f));
    g.drawRoundedRectangle(juce::Rectangle<float>(rightCardX, lowerCardY, 334.0f, lowerCardHeight), 16.0f, 1.0f);

    auto footer = juce::Rectangle<float>(0.0f, static_cast<float>(getHeight() - 24),
                                         static_cast<float>(getWidth()), 24.0f);
    g.setColour(ink);
    g.fillRect(footer);
    g.setColour(juce::Colour(0xff68458c));
    g.drawLine(0.0f, footer.getY(), footer.getRight(), footer.getY(), 0.8f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
    g.drawText("TODB  •  ORIGINAL BAY AREA DESIGN  •  8× REAL-TIME OVERSAMPLING",
               footer.toNearestInt().reduced(20, 0), juce::Justification::centredLeft);
}

void KrakenKlipperAudioProcessorEditor::resized()
{
    const auto width = getWidth();
    softButton.setBounds(47, 153, 103, 31);
    mediumButton.setBounds(158, 153, 112, 31);
    hardButton.setBounds(278, 153, 112, 31);
    bypassButton.setBounds(width - 420, 126, 105, 37);
    deltaButton.setBounds(width - 300, 126, 94, 37);
    presetSelector.setBounds(width - 196, 126, 163, 37);

    const auto controlW = (width - 120) / 5;
    const auto firstX = 56;
    const auto y = 197;
    const auto h = 153;
    auto place = [&](juce::Slider& slider, int index)
    {
        slider.setBounds(firstX + index * controlW, y, controlW - 2, h);
    };
    place(driveSlider, 0);
    place(ceilingSlider, 1);
    place(kneeSlider, 2);
    place(mixSlider, 3);
    place(outputSlider, 4);

    curveDisplay->setBounds(62, 397, width - 450, getHeight() - 455);
    meterDisplay->setBounds(width - 354, 397, 322, 126);
}

void KrakenKlipperAudioProcessorEditor::timerCallback()
{
    const auto style = static_cast<int>(std::lround(processor.parameters.getRawParameterValue("character")->load()));
    softButton.setToggleState(style == 0, juce::dontSendNotification);
    mediumButton.setToggleState(style == 1, juce::dontSendNotification);
    hardButton.setToggleState(style == 2, juce::dontSendNotification);
}

void KrakenKlipperAudioProcessorEditor::setCharacter(int index)
{
    setPluginParameter("character", static_cast<float>(index));
    timerCallback();
}

void KrakenKlipperAudioProcessorEditor::setPluginParameter(const juce::String& id, float value)
{
    if (auto* parameter = processor.parameters.getParameter(id))
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
}

void KrakenKlipperAudioProcessorEditor::applyPreset(int preset)
{
    struct Values { float drive, ceiling, knee, mix, output, character; };
    Values values{};
    switch (preset)
    {
        case 2: values = {8.0f, -3.0f, 18.0f, 100.0f, -1.0f, 0.0f}; break; // 808 Reef
        case 3: values = {6.0f, -2.0f, 5.0f, 88.0f, -1.0f, 2.0f}; break;  // Drum Snap
        case 4: values = {5.0f, -1.0f, 10.0f, 100.0f, 0.0f, 1.0f}; break; // Violet Punch
        case 5: values = {2.0f, -4.0f, 22.0f, 62.0f, 0.0f, 0.0f}; break; // Bus Glide
        default: return;
    }
    setPluginParameter("drive", values.drive);
    setPluginParameter("ceiling", values.ceiling);
    setPluginParameter("knee", values.knee);
    setPluginParameter("mix", values.mix);
    setPluginParameter("output", values.output);
    setPluginParameter("character", values.character);
}
