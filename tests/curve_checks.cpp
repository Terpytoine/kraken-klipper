#include "ClipperCurve.h"

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
bool near(float a, float b, float tolerance)
{
    return std::abs(a - b) <= tolerance;
}

int fail(const char* message)
{
    std::cerr << "Curve check failed: " << message << '\n';
    return EXIT_FAILURE;
}
}

int main()
{
    for (const auto ceiling : {1.0f, 0.5f, 0.1f})
    {
        for (const auto kneeRatio : {1.0f, 0.75f, 0.5f, 0.1f, 0.01f})
        {
            for (const auto character : {0.0f, 0.5f, 1.0f, 1.5f, 2.0f})
            {
                float previous = -ceiling;
                for (int i = 0; i <= 80000; ++i)
                {
                    const auto input = -4.0f + 8.0f * static_cast<float>(i) / 80000.0f;
                    const auto output = kraken::clipBlend(input, ceiling, kneeRatio, character);
                    if (!std::isfinite(output)) return fail("curve returned a non-finite value");
                    if (output < -ceiling - 1.0e-6f || output > ceiling + 1.0e-6f)
                        return fail("curve exceeded the selected ceiling");
                    if (i > 0 && output + 2.0e-6f < previous)
                        return fail("curve was not monotonic");
                    if (!near(output, -kraken::clipBlend(-input, ceiling, kneeRatio, character), 2.0e-6f))
                        return fail("curve lost positive/negative symmetry");
                    previous = output;
                }
            }
        }
    }

    constexpr auto ceiling = 1.0f;
    constexpr auto kneeRatio = 0.25f;
    constexpr auto start = ceiling * kneeRatio;
    constexpr auto epsilon = 0.0001f;
    const auto slopeAtStart = (kraken::softKnee(start + epsilon, ceiling, kneeRatio) - start) / epsilon;
    const auto slopeAtCeiling = (ceiling - kraken::softKnee(ceiling - epsilon, ceiling, kneeRatio)) / epsilon;
    if (!near(slopeAtStart, 1.0f, 0.002f)) return fail("soft knee was not tangent at its start");
    if (slopeAtCeiling > 0.002f) return fail("soft knee did not flatten at its ceiling");
    if (!near(kraken::softKnee(0.20f, ceiling, kneeRatio), 0.20f, 1.0e-6f))
        return fail("signal below the knee was not unchanged");
    if (!near(kraken::hardClip(3.0f, ceiling), ceiling, 1.0e-6f))
        return fail("hard clip did not land on the ceiling");
    if (kraken::softKnee(std::numeric_limits<float>::quiet_NaN(), ceiling, kneeRatio) != 0.0f
        || kraken::hardClip(std::numeric_limits<float>::infinity(), ceiling) != 0.0f)
        return fail("non-finite audio was not safely cleared");

    std::cout << "Double Cup Clipper transfer curve checks passed.\n";
    return EXIT_SUCCESS;
}
