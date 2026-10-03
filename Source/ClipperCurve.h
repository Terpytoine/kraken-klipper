#pragma once

#include <algorithm>
#include <cmath>

namespace kraken
{
enum class Character
{
    soft,
    medium,
    hard
};

inline float hardClip(float sample, float ceiling) noexcept
{
    if (!std::isfinite(sample))
        return 0.0f;
    const auto limit = std::clamp(ceiling, 1.0e-5f, 1.0f);
    return std::clamp(sample, -limit, limit);
}

// A cubic Hermite shoulder: unity gain below the knee, then a smooth, bounded
// transition to the ceiling. inputSpanInKneeWidths controls how far the curve
// rounds beyond the ceiling before it reaches the flat top.
inline float softKnee(float sample, float ceiling, float kneeRatio,
                      float inputSpanInKneeWidths = 2.6f) noexcept
{
    if (!std::isfinite(sample))
        return 0.0f;
    const auto limit = std::clamp(ceiling, 1.0e-5f, 1.0f);
    const auto ratio = std::clamp(kneeRatio, 1.0e-5f, 1.0f);
    const auto magnitude = std::abs(sample);
    const auto kneeStart = limit * ratio;

    if (magnitude <= kneeStart)
        return sample;

    const auto outputSpan = limit - kneeStart;
    if (outputSpan <= 1.0e-7f)
        return std::copysign(limit, sample);

    const auto spanMultiplier = std::clamp(inputSpanInKneeWidths, 1.0f, 3.0f);
    const auto inputSpan = outputSpan * spanMultiplier;
    const auto kneeEnd = kneeStart + inputSpan;
    if (magnitude >= kneeEnd)
        return std::copysign(limit, sample);

    const auto t = std::clamp((magnitude - kneeStart) / inputSpan, 0.0f, 1.0f);
    const auto t2 = t * t;
    const auto t3 = t2 * t;
    const auto hermiteStart = t - 2.0f * t2 + t3;
    const auto hermiteEnd = 3.0f * t2 - 2.0f * t3;
    const auto shaped = kneeStart + outputSpan *
        (spanMultiplier * hermiteStart + hermiteEnd);
    return std::copysign(std::min(shaped, limit), sample);
}

inline float clip(float sample, float ceiling, float kneeRatio, Character character) noexcept
{
    const auto hard = hardClip(sample, ceiling);
    if (character == Character::hard)
        return hard;

    const auto soft = softKnee(sample, ceiling, kneeRatio, 2.6f);
    if (character == Character::soft)
        return soft;

    // Medium keeps a tighter knee than Soft while still rounding the corner.
    const auto mediumRatio = std::sqrt(std::clamp(kneeRatio, 1.0e-5f, 1.0f));
    return softKnee(sample, ceiling, mediumRatio, 2.0f);
}

// Blend the three characters continuously so changing the mode is click-free
// when the host automation is smoothed by the processor.
inline float clipBlend(float sample, float ceiling, float kneeRatio, float character) noexcept
{
    const auto position = std::clamp(character, 0.0f, 2.0f);
    if (position < 1.0f)
    {
        const auto soft = softKnee(sample, ceiling, kneeRatio, 2.6f);
        if (position <= 0.0f)
            return soft;
        const auto mediumRatio = std::sqrt(std::clamp(kneeRatio, 1.0e-5f, 1.0f));
        const auto medium = softKnee(sample, ceiling, mediumRatio, 2.0f);
        return soft + (medium - soft) * position;
    }

    const auto mediumRatio = std::sqrt(std::clamp(kneeRatio, 1.0e-5f, 1.0f));
    const auto medium = softKnee(sample, ceiling, mediumRatio, 2.0f);
    if (position >= 2.0f)
        return hardClip(sample, ceiling);
    const auto hard = hardClip(sample, ceiling);
    return medium + (hard - medium) * (position - 1.0f);
}
} // namespace kraken
