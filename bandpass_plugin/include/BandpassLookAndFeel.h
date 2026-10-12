#pragma once

namespace bandpass {
    // ============================================================
    // 自定义 LookAndFeel：实现设计图中的蓝色发光水平滑块
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

        // 绘制水平滑块：深色轨道+蓝色发光填充+白色圆形滑块头
        void drawLinearSlider(juce::Graphics &,
            int x, int y, int width, int height,
            float sliderPos,
            float minSliderPos, float maxSliderPos,
            juce::Slider::SliderStyle,
            juce::Slider &) override;

        // 滑块布局：文本框在右侧
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