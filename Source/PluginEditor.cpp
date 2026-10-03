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

void drawDoubleCupMark(juce::Graphics& g, juce::Rectangle<float> area)
{
    auto drawCup = [&](juce::Rectangle<float> cup, bool backCup)
    {
        const auto shift = backCup ? 0.0f : 2.0f;
        juce::Path body;
        body.startNewSubPath(cup.getX() + cup.getWidth() * 0.12f, cup.getY() + 8.0f + shift);
        body.lineTo(cup.getRight() - cup.getWidth() * 0.12f, cup.getY() + 8.0f + shift);
        body.lineTo(cup.getX() + cup.getWidth() * 0.82f, cup.getBottom() - 2.0f);
        body.quadraticTo(cup.getCentreX(), cup.getBottom() + 2.0f,
                         cup.getX() + cup.getWidth() * 0.18f, cup.getBottom() - 2.0f);
        body.closeSubPath();

        g.setColour(juce::Colour(0xff090612).withAlpha(0.88f));
        g.fillPath(body);
        g.setColour(backCup ? violet.withAlpha(0.76f) : gold.withAlpha(0.9f));
        g.strokePath(body, juce::PathStrokeType(1.8f));
        g.setColour(purple.withAlpha(0.45f));
        g.fillRect(cup.getX() + cup.getWidth() * 0.22f, cup.getY() + 13.0f + shift,
                   cup.getWidth() * 0.56f, 4.0f);

        g.setColour(paper.withAlpha(0.8f));
        g.drawLine(cup.getX() + cup.getWidth() * 0.6f, cup.getY() + 5.0f + shift,
                   cup.getX() + cup.getWidth() * 0.78f, cup.getY() - 3.0f + shift, 1.6f);
        g.setColour(violet.withAlpha(0.65f));
        g.drawLine(cup.getX() + cup.getWidth() * 0.28f, cup.getY() + 19.0f + shift,
                   cup.getX() + cup.getWidth() * 0.39f, cup.getBottom() - 7.0f, 1.0f);
    };

    const auto cupWidth = area.getWidth() * 0.43f;
    const auto cupHeight = area.getHeight() * 0.78f;
    drawCup({area.getX() + area.getWidth() * 0.08f, area.getY() + area.getHeight() * 0.12f,
             cupWidth, cupHeight}, true);
    drawCup({area.getX() + area.getWidth() * 0.46f, area.getY() + area.getHeight() * 0.20f,
             cupWidth, cupHeight}, false);
}

void drawSprayPaint(juce::Graphics& g, juce::Rectangle<float> area)
{
    const auto centre = area.getCentre();
    g.setGradientFill(juce::ColourGradient(purple.withAlpha(0.19f), centre,
                                           juce::Colour(0xff0c0811).withAlpha(0.0f),
                                           {centre.x, area.getBottom()}, true));
    g.fillRoundedRectangle(area, 9.0f);

    const std::array<juce::Point<float>, 16> flecks{{
        {0.04f, 0.35f}, {0.09f, 0.71f}, {0.15f, 0.22f}, {0.21f, 0.83f},
        {0.27f, 0.43f}, {0.34f, 0.18f}, {0.42f, 0.76f}, {0.49f, 0.31f},
        {0.57f, 0.84f}, {0.64f, 0.23f}, {0.72f, 0.64f}, {0.79f, 0.34f},
        {0.86f, 0.78f}, {0.91f, 0.18f}, {0.96f, 0.49f}, {0.53f, 0.56f}
    }};
    for (size_t i = 0; i < flecks.size(); ++i)
    {
        const auto point = juce::Point<float>(area.getX() + area.getWidth() * flecks[i].x,
                                               area.getY() + area.getHeight() * flecks[i].y);
        const auto radius = (i % 3 == 0 ? 2.6f : 1.5f);
        g.setColour((i % 4 == 0 ? gold : violet).withAlpha(i % 3 == 0 ? 0.42f : 0.27f));
        g.fillEllipse(point.x - radius, point.y - radius, radius * 2.0f, radius * 2.0f);
    }

    g.setColour(violet.withAlpha(0.25f));
    for (auto fraction : {0.025f, 0.965f})
    {
        const auto x = area.getX() + area.getWidth() * fraction;
        const auto y = area.getY() + area.getHeight() * (fraction < 0.5f ? 0.4f : 0.23f);
        g.drawLine(x, y, x + 1.5f, juce::jmin(area.getBottom() - 3.0f, y + 15.0f), 1.2f);
    }

    juce::Path flash;
    flash.startNewSubPath(area.getX() + area.getWidth() * 0.89f, area.getY() + 5.0f);
    flash.lineTo(area.getX() + area.getWidth() * 0.84f, area.getY() + area.getHeight() * 0.42f);
    flash.lineTo(area.getX() + area.getWidth() * 0.90f, area.getY() + area.getHeight() * 0.42f);
    flash.lineTo(area.getX() + area.getWidth() * 0.86f, area.getBottom() - 4.0f);
    g.setColour(purple.withAlpha(0.30f));
    g.strokePath(flash, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
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
        g.setColour(muted);
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
        const auto labelsWidth = 64;
        constexpr int valueWidth = 74;
        auto drawMeter = [&](const juce::String& label, float db, int row, const juce::Colour& colour)
        {
            auto line = inner.withY(inner.getY() + row * 35).withHeight(22);
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

        auto grLine = inner.withY(inner.getY() + 70).withHeight(22);
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
    driveSlider.setDoubleClickReturnValue(true, 0.0);
    ceilingSlider.setDoubleClickReturnValue(true, -1.0);
    kneeSlider.setDoubleClickReturnValue(true, 12.0);
    mixSlider.setDoubleClickReturnValue(true, 100.0);
    outputSlider.setDoubleClickReturnValue(true, 0.0);
    driveSlider.setTooltip("Drive: input gain before the clipper. More drive pushes the sound further into the clipping curve.");
    ceilingSlider.setTooltip("Ceiling: the level where clipping reaches its limit. Output trim is applied afterward and can raise the final level above the ceiling.");
    kneeSlider.setTooltip("Knee: how gradually Soft and Medium approach the ceiling. Hard clipping ignores this setting.");
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
    softButton.setTooltip("Soft: the widest, smoothest transition into the ceiling.");
    mediumButton.setTooltip("Medium: a tighter transition for a firmer clip.");
    hardButton.setTooltip("Hard: flat-top clipping at the selected ceiling; the Knee control has no effect.");

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
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "bypass", bypassButton);
    deltaAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.parameters, "delta", deltaButton);

    presetSelector.addItem("SELECT PRESET", 1);
    presetSelector.addItem("808 REEF", 2);
    presetSelector.addItem("DRUM SNAP", 3);
    presetSelector.addItem("VIOLET PUNCH", 4);
    presetSelector.addItem("BUS GLIDE", 5);
    presetSelector.setSelectedId(1, juce::dontSendNotification);
    presetSelector.setLookAndFeel(customLookAndFeel.get());
    presetSelector.setTooltip("Load a starting preset. It updates Drive, Ceiling, Knee, Mix, Output, and clip character.");
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
    curveDisplay->setTooltip("Read left to right: input level is along the bottom, and output level is up the side. At full scale, 1.0 equals 0 dBFS. The curve shows the processed sound; the dashed diagonal shows the same signal with no clipping. A flatter top means stronger clipping. Bypass shows the input line; Delta shows only what changed.");
    meterDisplay->setTooltip("Input and Output show sample peaks in dBFS. A red Output bar means the signal is above 0 dBFS and may clip in a later plug-in or output. Reduction shows how much level the clipping curve removes.");
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
    drawDoubleCupMark(g, juce::Rectangle<float>(getWidth() * 0.605f, 49.0f,
                                                 getWidth() * 0.055f, 45.0f));
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
    g.drawText("DC", juce::Rectangle<int>(static_cast<int>(logoTile.getX() + 28.0f),
                                            static_cast<int>(logoTile.getY() + 8.0f), 43, 50),
               juce::Justification::centred);
    g.setColour(gold.withAlpha(0.7f));
    g.drawLine(33.0f, 79.0f, 86.0f, 79.0f, 1.2f);

    const juce::String title = "DOUBLE CUP CLIPPER";
    const auto titleWidth = juce::jmax(280, static_cast<int>(getWidth() * 0.54f) - 142);
    const auto titleSize = juce::jlimit(24.0f, 29.0f, 25.0f + (getWidth() - 900) * 0.025f);
    const auto titleBounds = juce::Rectangle<int>(112, 17, titleWidth, 39);
    g.setFont(juce::Font(juce::FontOptions("Impact", titleSize, juce::Font::bold)
                             .withFallbacks({"Arial Black", "Arial"})));
    g.setColour(juce::Colour(0xff3e1a5c));
    g.drawText(title, titleBounds.translated(2, 3), juce::Justification::centredLeft);
    g.setColour(violet.withAlpha(0.80f));
    g.drawText(title, titleBounds.translated(1, 1), juce::Justification::centredLeft);
    g.setColour(paper);
    g.drawText(title, titleBounds, juce::Justification::centredLeft);
    g.setColour(violet);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("BAY AREA PLUGINS #001  |  Twon On Da Beat", 115, 56, 420, 19,
               juce::Justification::centredLeft);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("SAN FRANCISCO BAY | GOLDEN GATE TO THE DEEP", 115, 75, 440, 16,
               juce::Justification::centredLeft);

    g.setColour(gold.withAlpha(0.92f));
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("8X OVERSAMPLING", getWidth() - 180, 22, 150, 16, juce::Justification::centredRight);
    g.setColour(muted);
    g.drawText("AUTO CHANNELS: MONO / STEREO", getWidth() - 220, 43, 190, 15,
               juce::Justification::centredRight);
    g.setColour(purple.withAlpha(0.5f));
    g.drawText("FIRST RELEASE", getWidth() - 180, 65, 150, 14,
               juce::Justification::centredRight);

    auto controlsCard = juce::Rectangle<float>(22.0f, 113.0f, getWidth() - 44.0f, 258.0f);
    g.setColour(panel.withAlpha(0.90f));
    g.fillRoundedRectangle(controlsCard, 16.0f);
    g.setColour(panelEdge);
    g.drawRoundedRectangle(controlsCard, 16.0f, 1.1f);

    auto modeCard = juce::Rectangle<float>(38.0f, 125.0f, 365.0f, 60.0f);
    g.setColour(juce::Colour(0xff100c18));
    g.fillRoundedRectangle(modeCard, 10.0f);
    drawSprayPaint(g, modeCard.reduced(4.0f));
    drawBayBridge(g, modeCard.reduced(8.0f, 13.0f));
    g.setColour(gold.withAlpha(0.22f));
    g.drawRoundedRectangle(modeCard, 10.0f, 1.0f);

    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("CHARACTER", 42, 131, 94, 16, juce::Justification::centredLeft);
    const auto captionControlWidth = (getWidth() - 120) / 5;
    const auto captionFirstX = 56;
    const std::array<juce::String, 5> hints{{"PUSH INTO CLIP", "CLIP THRESHOLD", "KNEE WIDTH",
                                             "DRY / WET", "FINAL LEVEL"}};
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
    for (int i = 0; i < static_cast<int>(hints.size()); ++i)
        g.drawText(hints[static_cast<size_t>(i)], captionFirstX + i * captionControlWidth, 351,
                   captionControlWidth - 2, 13, juce::Justification::centred);

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
    g.drawText("BAY AREA PLUGINS #001  |  TODB / TWON ON DA BEAT  |  8X REAL-TIME OVERSAMPLING",
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
