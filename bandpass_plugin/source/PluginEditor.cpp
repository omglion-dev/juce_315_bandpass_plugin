#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../include/PluginProcessor.h"
#include "bandpass_plugin/include/BandpassLookAndFeel.h"
#include "../include/PluginEditor.h"

namespace bandpass {
    PluginEditor::PluginEditor(PluginProcessor& processorRef)
        : juce::AudioProcessorEditor(processorRef),
          processor(processorRef)
    {
        // Assign custom LookAndFeel styling
        setLookAndFeel(&lookAndFeel);

        // --- Title text label ---
        titleLabel.setFont(BandpassLookAndFeel::getTitleFont());
        titleLabel.setColour(
            juce::Label::textColourId,
            BandpassLookAndFeel::getDefaultColour(BandpassLookAndFeel::Colors::titleLabelColor));
        addAndMakeVisible(titleLabel);

        // --- Active status indicator label ---
        activeLabel.setFont(BandpassLookAndFeel::getActiveLabelFont());
        activeLabel.setColour(
            juce::Label::textColourId,
            BandpassLookAndFeel::getDefaultColour(BandpassLookAndFeel::Colors::activeLabelColor));
        addAndMakeVisible(activeLabel);

        // --- Frequency‑response plot component ---
        addAndMakeVisible(responseComponent);

        freqLabel.setFont(BandpassLookAndFeel::getFreqAndQLabelFont());
        addAndMakeVisible(freqLabel);
        // --- Frequency‑Center slider control ---
        freqSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        freqSlider.setTextBoxStyle(
            juce::Slider::TextBoxRight,
            false,
            80 ,
            24);
        freqSlider.setTextValueSuffix(" Hz");
        freqSlider.setRange(
            Bandpass::minFreqCenter,
            Bandpass::maxFreqCenter,
            1.0f);
        freqSlider.setValue(
            processor.getParameterRefs().freqCenter.get(),
            juce::NotificationType::dontSendNotification);
        freqSlider.addListener(this);
        addAndMakeVisible(freqSlider);

        qLabel.setFont(BandpassLookAndFeel::getFreqAndQLabelFont());
        addAndMakeVisible(qLabel);
        // --- Q‑Factor slider control ---
        qSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        qSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
        qSlider.setRange(Bandpass::minQ, Bandpass::maxQ, 0.1f);
        qSlider.setValue(
            processor.getParameterRefs().q.get(),
            juce::NotificationType::dontSendNotification);
        qSlider.addListener(this);
        addAndMakeVisible(qSlider);

        // --- Bottom status bar label ---
        statusLabel.setFont(BandpassLookAndFeel::getStatusLabelFont());
        statusLabel.setColour(
            juce::Label::textColourId,
            BandpassLookAndFeel::getDefaultColour(BandpassLookAndFeel::Colors::statusLabelTextColor));
        addAndMakeVisible(statusLabel);

        // Set overall editor window size
        setSize(820, 520);

        // Initialize UI display state
        updateDisplay();
    }

    PluginEditor::~PluginEditor() {
        setLookAndFeel(nullptr);
    }

    void PluginEditor::paint(juce::Graphics &g) {
        const auto w = static_cast<float>(getWidth());
        const auto h = static_cast<float>(getHeight());

        // --- Background: dark vertical gradient, simulating carbon‑fiber panel look ---
        juce::ColourGradient bgGrad(
            juce::Colour(0xff2a2a2a),
            w * 0.5f, h * 0.3f,
            juce::Colour(0xff121212),
            w * 0.5f, h,
            false);
        g.setGradientFill(bgGrad);
        g.fillAll();

        // --- Fine diagonal hatch pattern for carbon‑fiber visual effect ---
        g.setColour(juce::Colour(0x80000000));
        for (float i = -h; i < w + h; i += 6) {
            g.drawLine(
                i, 0,
                i + h, h,
                1.0f);
        }

        // --- Top title‑area dividing separator line ---
        g.setColour(juce::Colour(0x33ffffff));
        g.drawHorizontalLine(55, 15, w - 15);

        // --- Bottom control‑panel dividing separator line ---
        g.drawHorizontalLine(static_cast<int>(h * 0.68f), 15, w - 15);

        // --- Corner screw decorative circles at four panel corners ---
        g.setColour(juce::Colour(0xff444444));
        const auto screwR = 5.0f;
        g.fillEllipse(
            10,
            10,
            screwR * 2,
            screwR * 2);
        g.fillEllipse(
            w - 20,
            10,
            screwR * 2,
            screwR * 2);
        g.fillEllipse(
            10,
            h - 20,
            screwR * 2,
            screwR * 2);
        g.fillEllipse(
            w - 20,
            h - 20,
            screwR * 2,
            screwR * 2);

        // 螺丝内部十字
        // Draw cross‑shaped indent inside each screw head
        g.setColour(juce::Colour(0xff222222));
        for (auto& sx : {15.0f, w - 15.0f}) {
            for (auto& sy : {15.0f, h - 15.0f}) {
                g.drawLine(sx - 3, sy, sx + 3, sy, 1.0f);
                g.drawLine(sx, sy - 3, sx, sy + 3, 1.0f);
            }
        }

        // --- Active status LED indicator: red‑yellow‑green dot trio ---
        const auto ledX = w - 130.f;
        const auto ledY = 22.f;
        // Red indicator dot
        g.setColour(juce::Colour(0xffff4444));
        g.fillEllipse(ledX, ledY, 8, 8);
        // Yellow indicator dot
        g.setColour(juce::Colour(0xffffcc00));
        g.fillEllipse(ledX + 12, ledY, 8, 8);
        // Green dot: lit to signal plugin active state
        g.setColour(juce::Colour(0xff44ff44));
        g.fillEllipse(ledX + 24, ledY, 8, 8);
        // 绿色光晕
        // Outer glow for green active LED
        juce::ColourGradient ledGlow(
            juce::Colour(0x6644ff44),
            ledX + 28, ledY + 4,
            juce::Colour(0x0044ff44),
            ledX + 28, ledY + 4,
            true);
        g.setGradientFill(ledGlow);
        g.fillEllipse(ledX + 20, ledY - 4, 16, 16);
    }

    void PluginEditor::resized() {
        const auto w = getWidth();
        const auto h = getHeight();

        // Title label bounds
        titleLabel.setBounds(25, 12, 200, 35);

        // ACTIVE status text, placed to right of LED indicator
        activeLabel.setBounds(w - 95, 14, 80, 24);

        // Middle plotting area for frequency‑response graph
        responseComponent.setBounds(15, 65, w - 30, static_cast<int>(h * 0.58f));

        // Bottom slider control region
        const auto sliderY = static_cast<int>(h * 0.72f);
        constexpr auto sliderH = 40;
        freqLabel.setBounds(25, sliderY - 15, w - 700, 20);
        freqSlider.setBounds(30, sliderY, w - 60, sliderH);
        qLabel.setBounds(25, sliderY + 40, w - 700, 20);
        qSlider.setBounds(30, sliderY + 55, w - 60, sliderH);

        // Bottom status information bar
        statusLabel.setBounds(20, h - 28, w - 40, 20);
    }

    void PluginEditor::sliderValueChanged(juce::Slider * slider) {
        if (slider == &freqSlider) {
            // Sync UI slider value to audio‑processor parameter and DSP backend
            processor.getParameterRefs().freqCenter = static_cast<float>(freqSlider.getValue());
        }
        else if (slider == &qSlider) {
            processor.getParameterRefs().q = static_cast<float>(qSlider.getValue());
        }
        updateDisplay();
    }

    void PluginEditor::updateDisplay() {
        float sampleRate = processor.getBandpassRef().getSampleRate();
        sampleRate = sampleRate > 0 ? sampleRate : Bandpass::defaultSampleRate;
        // Refresh frequency‑response plot component with latest values
        responseComponent.setParameter(
            static_cast<float>(freqSlider.getValue()),
            static_cast<float>(qSlider.getValue()),
            sampleRate);
        // Update bottom status‑bar text
        statusLabel.setText(
            "Sample Rate: " + juce::String(static_cast<int>(sampleRate)) + " Hz",
            juce::NotificationType::dontSendNotification);
    }
}
