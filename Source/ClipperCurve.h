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

// A cubic Hermite knee: unity gain below the knee, a smooth transition at
// the ceiling, and a flat tangent where the signal meets the ceiling.
inline float softKnee(float sample, float ceiling, float kneeRatio) noexcept
{
    if (!std::isfinite(sample))
        return 0.0f;
    const auto limit = std::clamp(ceiling, 1.0e-5f, 1.0f);
    const auto ratio = std::clamp(kneeRatio, 1.0e-5f, 1.0f);
    const auto magnitude = std::abs(sample);
    const auto kneeStart = limit * ratio;

    if (magnitude <= kneeStart)
        return sample;

    if (magnitude >= limit || limit - kneeStart <= 1.0e-7f)
        return std::copysign(limit, sample);

    const auto width = limit - kneeStart;
    const auto t = std::clamp((magnitude - kneeStart) / width, 0.0f, 1.0f);
    const auto shaped = kneeStart + width * (t + t * t - t * t * t);
    return std::copysign(std::min(shaped, limit), sample);
}

inline float clip(float sample, float ceiling, float kneeRatio, Character character) noexcept
{
    const auto hard = hardClip(sample, ceiling);
    if (character == Character::hard)
        return hard;

    const auto soft = softKnee(sample, ceiling, kneeRatio);
    if (character == Character::soft)
        return soft;

    // Medium keeps a tighter knee than Soft while still rounding the corner.
    const auto mediumRatio = std::sqrt(std::clamp(kneeRatio, 1.0e-5f, 1.0f));
    return softKnee(sample, ceiling, mediumRatio);
}

// Blend the three characters continuously so changing the mode is click-free
// when the host automation is smoothed by the processor.
inline float clipBlend(float sample, float ceiling, float kneeRatio, float character) noexcept
{
    const auto position = std::clamp(character, 0.0f, 2.0f);
    if (position < 1.0f)
    {
        const auto soft = softKnee(sample, ceiling, kneeRatio);
        if (position <= 0.0f)
            return soft;
        const auto mediumRatio = std::sqrt(std::clamp(kneeRatio, 1.0e-5f, 1.0f));
        const auto medium = softKnee(sample, ceiling, mediumRatio);
        return soft + (medium - soft) * position;
    }

    const auto mediumRatio = std::sqrt(std::clamp(kneeRatio, 1.0e-5f, 1.0f));
    const auto medium = softKnee(sample, ceiling, mediumRatio);
    if (position >= 2.0f)
        return hardClip(sample, ceiling);
    const auto hard = hardClip(sample, ceiling);
    return medium + (hard - medium) * (position - 1.0f);
}
} // namespace kraken
