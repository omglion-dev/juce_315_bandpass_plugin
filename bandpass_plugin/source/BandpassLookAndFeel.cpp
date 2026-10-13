#include "../include/BandpassLookAndFeel.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

#include "BandpassPluginAssets.h"

namespace bandpass {
    BandpassLookAndFeel::BandpassLookAndFeel() {
        // Assign default colour values to LookAndFeel colour‑ids
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

        // --- Track background (dark rounded rectangle bar) ---
        g.setColour(getDefaultColour(Colors::sliderTrackBackgroundColor));
        g.fillRoundedRectangle(trackX, trackY, trackW, trackH, 3.0f);

        // --- Filled active section (blue glow effect) ---
        // Outer glow: create linear gradient object for glow rendering
        juce::ColourGradient glow(
            juce::Colour(0x6600aaff), trackX, trackY + trackH * 0.5f,
            juce::Colour(0x0000aaff), sliderPos, trackY + trackH * 0.5f, false);
        g.setGradientFill(glow);
        g.fillRoundedRectangle(trackX, trackY - 2.0f, fillW, trackH + 4.0f, 3.0f);

        // Inner bright core track bar
        g.setColour(juce::Colour(0xff00ccff));
        g.fillRoundedRectangle(trackX, trackY, fillW, trackH, 3.0f);

        // --- Slider thumb (white circle with surrounding blue radial glow) ---
        constexpr auto thumbR = 12.0f;
        const auto thumbX = sliderPos;
        const auto thumbY = trackY + trackH * 0.5f;
        // Radial glow for thumb
        juce::ColourGradient thumbGlow(
            juce::Colour(0x8800aaff),   // Gradient centre colour
            thumbX, thumbY,             // Gradient centre point coordinates
            juce::Colour(0x0000aaff),   // Outer edge colour of radial gradient
            thumbX, thumbY +(thumbR + 8.0f),  // Point defining radial gradient radius
            true);
        g.setGradientFill(thumbGlow);
        g.fillEllipse(
            thumbX - thumbR - 6.0f,
            thumbY - thumbR - 6.0f,
            (thumbR + 6.0f) * 2.0f,
            (thumbR + 6.0f) * 2.0f);

        // Main white thumb circle
        g.setColour(getDefaultColour(Colors::sliderThumbColor));
        g.fillEllipse(
            thumbX - thumbR,
            thumbY - thumbR,
            thumbR * 2.0f,
            thumbR * 2.0f);

        // Inner highlight spot inside thumb circle
        g.setColour(getDefaultColour(Colors::sliderThumbHighlightColor));
        g.fillEllipse(
            thumbX - thumbR * 0.6f,
            thumbY - thumbR * 0.6f,
            thumbR * 1.2f,
            thumbR * 1.2f);
    }

    juce::Slider::SliderLayout BandpassLookAndFeel::getSliderLayout(juce::Slider & slider) {
        auto layout = juce::LookAndFeel_V4::getSliderLayout(slider);
        // Place value text‑box on slider right‑side, set width to 80 pixels
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
