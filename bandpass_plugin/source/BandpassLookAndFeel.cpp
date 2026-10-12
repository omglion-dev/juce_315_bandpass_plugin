#include "../include/BandpassLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

#include "BandpassPluginAssets.h"

namespace bandpass {
    BandpassLookAndFeel::BandpassLookAndFeel() {
        // 设置默认颜色
        setColour(juce::Slider::backgroundColourId, getDefaultColour(Colors::sliderBackgroundColor));
        setColour(juce::Slider::trackColourId, getDefaultColour(Colors::sliderTrackColor));
        setColour(juce::Slider::thumbColourId, getDefaultColour(Colors::sliderThumbColor));
        setColour(juce::Slider::textBoxBackgroundColourId, getDefaultColour(Colors::sliderTextBoxBackgroundColor));
        setColour(juce::Slider::textBoxHighlightColourId, getDefaultColour(Colors::sliderTextBoxHighlightColor));
        setColour(juce::Slider::textBoxOutlineColourId, getDefaultColour(Colors::sliderTextBoxOutlineColor));
    }

    void BandpassLookAndFeel::drawLinearSlider(juce::Graphics & g,
        int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        juce::Slider::SliderStyle style,
        juce::Slider & slider) {

        juce::ignoreUnused(minSliderPos, maxSliderPos, style, slider);

        const auto trackX = static_cast<float>(x);
        const auto trackY = y + height * 0.5f - 3.0f;
        const auto trackW = static_cast<float>(width);
        const auto trackH = 6.0f;
        const auto fillW = sliderPos - trackX;

        // --- 轨道背景（深色圆角条）---
        g.setColour(getDefaultColour(Colors::sliderTrackBackgroundColor));
        g.fillRoundedRectangle(trackX, trackY, trackW, trackH, 3.0f);

        // --- 已填充部分（蓝色发光）---
        // 外层光晕：创建一个线性渐变对象glow
        juce::ColourGradient glow(
            juce::Colour(0x6600aaff), trackX, trackY + trackH * 0.5f,
            juce::Colour(0x0000aaff), sliderPos, trackY + trackH * 0.5f, false);
        g.setGradientFill(glow);
        g.fillRoundedRectangle(trackX, trackY - 2.0f, fillW, trackH + 4.0f, 3.0f);

        // 核心亮条
        g.setColour(juce::Colour(0xff00ccff));
        g.fillRoundedRectangle(trackX, trackY, fillW, trackH, 3.0f);

        // --- 滑块头（白色圆形+蓝色光晕）---
        constexpr auto thumbR = 12.0f;
        const auto thumbX = sliderPos;
        const auto thumbY = trackY + trackH * 0.5f;
        // 光晕
        juce::ColourGradient thumbGlow(
            juce::Colour(0x8800aaff),   // 圆心颜色
            thumbX, thumbY,             // 圆心坐标
            juce::Colour(0x0000aaff),   // 外圈边缘颜色
            thumbX, thumbY +(thumbR + 8.0f),  // 渐变半径
            true);
        g.setGradientFill(thumbGlow);
        g.fillEllipse(
            thumbX - thumbR - 6.0f,
            thumbY - thumbR - 6.0f,
            (thumbR + 6.0f) * 2.0f,
            (thumbR + 6.0f) * 2.0f);

        // 白色圆形
        g.setColour(getDefaultColour(Colors::sliderThumbColor));
        g.fillEllipse(
            thumbX - thumbR,
            thumbY - thumbR,
            thumbR * 2.0f,
            thumbR * 2.0f);

        // 圆形内部高光
        g.setColour(getDefaultColour(Colors::sliderThumbHighlightColor));
        g.fillEllipse(
            thumbX - thumbR * 0.6f,
            thumbY - thumbR * 0.6f,
            thumbR * 1.2f,
            thumbR * 1.2f);
    }

    juce::Slider::SliderLayout BandpassLookAndFeel::getSliderLayout(juce::Slider & slider) {
        auto layout = juce::LookAndFeel_V4::getSliderLayout(slider);
        // 文本框放在滑块右侧，宽度80
        layout.textBoxBounds = juce::Rectangle<int>(
            slider.getWidth() - 90,
            0,
            80,
            slider.getHeight());
        layout.sliderBounds = juce::Rectangle<int>(
            0,0,slider.getWidth() - 100, slider.getHeight());
        return layout;
    }

    juce::Colour BandpassLookAndFeel::getDefaultColour(const Colors colorName) {
        static const std::array colors = {
            juce::Colour(0xff2a2a2a),
            juce::Colour(0xff00aaff),
            juce::Colour(0xffffffff),
            juce::Colour(0xffcccccc),
            juce::Colour(0x00000000),
            juce::Colour(0x4400aaff),
            juce::Colour(0x00000000),
            juce::Colour(0xff1e1e1e),
            juce::Colour(0x33000000),
            juce::Colour(0xffe0e0e0),
            juce::Colour(0xff88ff88),
            juce::Colour(0xff888888)
        };
        const auto idx = juce::toUnderlyingType(colorName);
        jassert(idx <= colors.size());
        return colors.at(idx);
    }

    juce::FontOptions BandpassLookAndFeel::getAxisFont() {
        return interMediumFont().withPointHeight(14.f);
    }

    juce::FontOptions BandpassLookAndFeel::getPeakFont() {
        return interBoldFont().withPointHeight(16.f);
    }

    juce::FontOptions BandpassLookAndFeel::getTitleFont() {
        return interBoldFont().withPointHeight(28.0f);
    }

    juce::FontOptions BandpassLookAndFeel::getActiveLabelFont() {
        return interBoldFont().withPointHeight(14.f);
    }

    juce::FontOptions BandpassLookAndFeel::getStatusLabelFont() {
        return interMediumFont().withPointHeight(12.f);
    }

    juce::FontOptions BandpassLookAndFeel::getFreqAndQLabelFont() {
        return interMediumFont().withPointHeight(14.f);
    }

    juce::FontOptions BandpassLookAndFeel::interMediumFont() {
        static const auto typefacePtr =
                 juce::Typeface::createSystemTypefaceFor(assets::InterMedium_ttf, assets::InterMedium_ttfSize);
        return juce::FontOptions(typefacePtr);
    }

    juce::FontOptions BandpassLookAndFeel::interBoldFont() {
        static const auto typefacePtr =
            juce::Typeface::createSystemTypefaceFor(assets::InterBold_ttf, assets::InterBold_ttfSize);
        return juce::FontOptions(typefacePtr);
    }
}
