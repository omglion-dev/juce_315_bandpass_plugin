#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

namespace bandpass {
    FrequencyResponseComponent::FrequencyResponseComponent() {
        setOpaque(false);
    }

    void FrequencyResponseComponent::setParameter(float freqCenterVal, float qVal, float sampleRateVal) {
        freqCenter = freqCenterVal;
        q = qVal;
        sampleRate = sampleRateVal;
        repaint();
    }

    float FrequencyResponseComponent::getMagnitude(const float frequency) const {
        // Analog‑domain second‑order band‑pass magnitude‑frequency response
        // H(s) = (ω0/Q * s) / (s² + ω0/Q * s + ω0²)
        // For complex fraction H = Z1/Z2: magnitude |H| = |Z1| / |Z2| (
        // magnitude of numerator divided by magnitude of denominator)
        // |H(jω)| = (ω0/Q * ω) / sqrt((ω0² − ω²)² + (ω0/Q * ω)²)

        // Center angular frequency ω0 = 2π·f_center
        const auto omega0 = juce::MathConstants<float>::twoPi * freqCenter;
        // Angular frequency for current evaluation point ω = 2π·frequency
        const auto omega = juce::MathConstants<float>::twoPi * frequency;
        // Angular bandwidth BW = ω0 / Q
        const auto bw = omega0 / q;

        // Numerator term: bw * ω
        const auto numerator = bw * omega;
        // Denominator term: sqrt( (ω0²−ω²)² + (bw·ω)² )
        const auto denominator = std::sqrt(std::pow(omega0 * omega0 - omega * omega, 2.0f)
                                        + std::pow(bw * omega, 2.0f));

        if (denominator < 1e-6f) {
            return 1.0f;
        }
        return numerator / denominator;
    }

    void FrequencyResponseComponent::paint(juce::Graphics& g) {
        const auto w = static_cast<float>(getWidth());
        const auto h = static_cast<float>(getHeight());
        const auto padLeft = 50.f;
        const auto padRight = 20.f;
        const auto padTop = 30.f;
        const auto padBottom = 35.f;
        // plotW: width of plotting area
        const auto plotW = w - padLeft - padRight;
        // plotH: height of plotting area
        const auto plotH = h - padTop - padBottom;

        // --- Semi‑transparent background panel ---
        g.setColour(juce::Colour(0x33000000));
        g.fillRoundedRectangle(padLeft - 10, padTop - 10, plotW + 20, plotH + 20, 6.0f);

        // --- Grid lines ---
        g.setColour(juce::Colour(0x22ffffff));
        // Horizontal grid lines (total 3 lines)
        for (int i = 0; i < 3; ++i) {
            const auto gy = padTop + plotH * i / 3.0f;
            g.drawHorizontalLine(static_cast<int>(gy), padLeft, padLeft + plotW);
        }
        // Vertical grid lines (total 5 lines)
        for (int i = 0; i < 5; ++i) {
            const auto gx = padLeft + plotW * i / 5.0f;
            g.drawVerticalLine(static_cast<int>(gx), padTop, padTop + plotH);
        }
        // --- Axis labels ---
        g.setColour(juce::Colour(0xffaaaaaa));
        g.setFont(BandpassLookAndFeel::getAxisFont());
        g.drawText(
            "Gain",
            2,
            static_cast<int>(padTop + plotH * 0.5f - 10),
            45,
            20,
            juce::Justification::centred);

        // --- Draw frequency‑response curve ---

        // Stores frequency‑response outline path, used for strokePath outline drawing
        juce::Path curvePath;
        // Stores closed polygon: curve + bottom boundary,
        // used for fillPath solid‑color filling
        juce::Path fillPath;

        constexpr auto minFreq = Bandpass::minFreqCenter;
        const auto maxFreq = sampleRate * 0.5f;     // Nyquist frequency

        constexpr int numPoints = 200;
        // Note: Must use <= condition, to make t reach 1.0
        // so we sample the Nyquist frequency point
        // This loop actually generates 201 evaluation samples
        for (int i = 0; i <= numPoints; ++i) {
            // t: normalized horizontal coordinate, range [0.0, 1.0]
            // i = 0  → t = 0.0: maps to leftmost frequency (20Hz)
            // i = 200 → t = 1.0: maps to rightmost Nyquist frequency
            const auto t = static_cast<float>(i) / numPoints;
            // Logarithmic frequency mapping
            // Formula derivation:
            // t=0 → freq = minFreq = 20Hz
            // t=1 → freq = minFreq * (maxFreq/minFreq)^1 = maxFreq → Nyquist frequency
            const auto freq = minFreq * std::pow(maxFreq / minFreq, t);
            const auto mag = getMagnitude(freq);

            // Map normalized t ∈ [0,1] to canvas pixel X‑coordinate:
            // t=0 → x=padLeft (left plotting boundary)
            // t=1 → x=padLeft+plotW (right plotting boundary)
            const auto x = padLeft + plotW * t;
            // Map magnitude mag ∈ [0,1] to canvas pixel Y‑coordinate:
            const auto y =
                // 1.f − mag: invert value: higher gain → smaller Y value
                // (towards screen top)
                padTop + plotH * (1.f - juce::jlimit(0.f, 1.f, mag));

            // Process first sample point (leftmost, 20Hz)
            if (i == 0) {
                // Start drawing path from current point
                curvePath.startNewSubPath(x, y);
                // Fill‑path starting point is NOT on response curve!
                // It is bottom‑left corner of plotting area.
                fillPath.startNewSubPath(x, padTop + plotH);
                // Draw vertical line upwards from bottom‑left to first curve sample point
                fillPath.lineTo(x, y);
            }
            // Process remaining sample points
            else {
                // Connect previous point to current (x,y),
                // assemble continuous frequency‑response polyline
                curvePath.lineTo(x, y);
                // Append each curve sample point to fill polygon path
                fillPath.lineTo(x, y);
            }
        }
        // Close fill polygon path
        // From right‑most curve point,
        // draw vertical line down to bottom‑right plotting corner
        fillPath.lineTo(padLeft + plotW, padTop + plotH);   // 右下角
        // Automatically connect bottom‑right corner
        // back to fill‑path starting bottom‑left corner
        fillPath.closeSubPath();
        // fillPath now forms a fully‑closed polygon shape

        // --- Gradient fill area below response curve ---
        juce::ColourGradient fillGrad(
            juce::Colour(0x4400aaff),
            0, padTop,
            juce::Colour(0x0000aaff),
            0, padTop + plotH,
            false);
        g.setGradientFill(fillGrad);
        g.fillPath(fillPath);

        // --- Outer glow effect: thick semi‑transparent stroke ---
        g.setColour(juce::Colour(0x3300ccff));
        g.strokePath(curvePath, juce::PathStrokeType(6.0f));

        // --- Main sharp curve core: bright‑blue thin line ---
        g.setColour(juce::Colour(0xff00ccff));
        g.strokePath(curvePath, juce::PathStrokeType(2.0f));

        // --- Peak‑point annotation ---
        // Given physical frequency freqCenter,
        // compute inverse mapping to get normalized t_peak, then pixel peakX
        // This is inverse operation of the frequency‑to‑t calculation above
        const auto peakX = padLeft + plotW * (std::log(freqCenter / minFreq) / std::log(maxFreq / minFreq));
        const auto peakY = padTop + plotH * (1.f - 1.f); // Peak magnitude mag = 1, located at plot top

        // Peak marker filled circle
        g.setColour(juce::Colour(0xffffffff));
        g.fillEllipse(peakX - 5.0f, peakY - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0x4400aaff));
        g.fillEllipse(peakX - 9.f, peakY - 9.f, 18.0f, 18.0f);

        // Peak‑frequency text label
        g.setColour(juce::Colour(0xffffffff));
        g.setFont(BandpassLookAndFeel::getPeakFont());
        juce::String peakText;
        if (freqCenter >= 1000.0f) {
            peakText = juce::String(freqCenter / 1000.0f, 1) + " kHz";
        }
        else {
            peakText = juce::String(static_cast<int>(freqCenter)) + " Hz";
        }
        g.drawText(
            peakText,
            static_cast<int>(peakX - 40),
            static_cast<int>(peakY - 33),
            80,
            20,
            juce::Justification::centred,
            false);

        // Vertical dashed indicator line for peak
        g.setColour(juce::Colour(0x44ffffff));
        constexpr float dashLengths[2] = {4.0f, 4.0f};
        g.drawDashedLine(
            juce::Line<float>(peakX, peakY, peakX, padTop + plotH),
            dashLengths,
            2,
            1.0f);

    }
}
