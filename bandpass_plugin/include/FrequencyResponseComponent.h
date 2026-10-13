#pragma once

namespace bandpass {
    // ============================================================
    // Frequency‑response plot component:
    // draws real‑time band‑pass response using current fc / Q values
    // ============================================================
    class FrequencyResponseComponent : public juce::Component {
    public:
        FrequencyResponseComponent();

        void paint(juce::Graphics &g) override;

        // Update internal parameters and trigger component repaint
        void setParameter(float freqCenter, float q, float sampleRate);

    private:
        float freqCenter = Bandpass::defaultFreqCenter;
        float q = Bandpass::defaultQ;
        float sampleRate = Bandpass::defaultSampleRate;

        // Compute normalized magnitude (range: 0‑1) of
        // band‑pass filter at given frequency
        float getMagnitude(const float frequency) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponseComponent)
    };
}