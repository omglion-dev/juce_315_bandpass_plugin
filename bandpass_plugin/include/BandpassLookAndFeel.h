#pragma once

namespace bandpass {
    // ============================================================
    // Custom LookAndFeel: implements styled glowing‑blue horizontal slider UI
    // ============================================================
    class BandpassLookAndFeel : public juce::LookAndFeel_V4 {
    public:
        enum class Colors : size_t {
            sliderBackgroundColor = 0,
            sliderTrackColor = 1,
            sliderThumbColor = 2,
            sliderTextBoxTextColor = 3,
            sliderTextBoxBackgroundColor = 4,
            sliderTextBoxHighlightColor = 5,
            sliderTextBoxOutlineColor = 6,
            sliderTrackBackgroundColor = 7,
            sliderThumbHighlightColor = 8,
            titleLabelColor = 9,
            activeLabelColor = 10,
            statusLabelTextColor = 11,
        };
        static juce::Colour getDefaultColour(const Colors colorName);

        BandpassLookAndFeel();

        // Draw horizontal slider:
        // dark track background + glowing‑blue filled track + white circular slider thumb
        void drawLinearSlider(juce::Graphics &,
            int x, int y, int width, int height,
            float sliderPos,
            float minSliderPos, float maxSliderPos,
            juce::Slider::SliderStyle,
            juce::Slider &) override;

        // Slider layout: position value text‑box on the right‑hand side
        juce::Slider::SliderLayout getSliderLayout(juce::Slider &) override;

        static juce::FontOptions getAxisFont();
        static juce::FontOptions getPeakFont();
        static juce::FontOptions getTitleFont();
        static juce::FontOptions getActiveLabelFont();
        static juce::FontOptions getStatusLabelFont();
        static juce::FontOptions getFreqAndQLabelFont();
    private:
        static juce::FontOptions interMediumFont();
        static juce::FontOptions interBoldFont();
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BandpassLookAndFeel)
    };
}