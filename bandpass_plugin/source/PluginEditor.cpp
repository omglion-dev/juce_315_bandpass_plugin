// #include "juce_gui_basics/widgets/juce_Slider.h"
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
        // 设置自定义LookAndFeel
        setLookAndFeel(&lookAndFeel);

        // --- 标题 ---
        titleLabel.setFont(BandpassLookAndFeel::getTitleFont());
        titleLabel.setColour(
            juce::Label::textColourId,
            BandpassLookAndFeel::getDefaultColour(BandpassLookAndFeel::Colors::titleLabelColor));
        addAndMakeVisible(titleLabel);

        // --- Active 指示灯 ---
        activeLabel.setFont(BandpassLookAndFeel::getActiveLabelFont());
        activeLabel.setColour(
            juce::Label::textColourId,
            BandpassLookAndFeel::getDefaultColour(BandpassLookAndFeel::Colors::activeLabelColor));
        addAndMakeVisible(activeLabel);

        // --- 频响曲线组件 ---
        addAndMakeVisible(responseComponent);

        freqLabel.setFont(BandpassLookAndFeel::getFreqAndQLabelFont());
        addAndMakeVisible(freqLabel);
        // --- Frequency Center 滑块
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
        // --- Q Factor 滑块 ---
        qSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
        qSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 24);
        qSlider.setRange(Bandpass::minQ, Bandpass::maxQ, 0.1f);
        qSlider.setValue(
            processor.getParameterRefs().q.get(),
            juce::NotificationType::dontSendNotification);
        qSlider.addListener(this);
        addAndMakeVisible(qSlider);

        // --- 底部状态栏 ---
        statusLabel.setFont(BandpassLookAndFeel::getStatusLabelFont());
        statusLabel.setColour(
            juce::Label::textColourId,
            BandpassLookAndFeel::getDefaultColour(BandpassLookAndFeel::Colors::statusLabelTextColor));
        addAndMakeVisible(statusLabel);

        // 设置编辑器大小
        setSize(820, 520);

        // 初始化显示
        updateDisplay();
    }

    PluginEditor::~PluginEditor() {
        setLookAndFeel(nullptr);
    }

    void PluginEditor::paint(juce::Graphics &g) {
        const auto w = static_cast<float>(getWidth());
        const auto h = static_cast<float>(getHeight());

        // --- 背景：深色镜像渐变（模拟碳纤维质感）---
        juce::ColourGradient bgGrad(
            juce::Colour(0xff2a2a2a),
            w * 0.5f, h * 0.3f,
            juce::Colour(0xff121212),
            w * 0.5f, h,
            false);
        g.setGradientFill(bgGrad);
        g.fillAll();

        // --- 细微的斜纹纹理（模拟碳纤维）---
        g.setColour(juce::Colour(0x80000000));
        for (float i = -h; i < w + h; i += 6) {
            g.drawLine(
                i, 0,
                i + h, h,
                1.0f);
        }

        // --- 顶部标题栏分隔线 ---
        g.setColour(juce::Colour(0x33ffffff));
        g.drawHorizontalLine(55, 15, w - 15);

        // --- 底部控制区分隔线 ---
        g.drawHorizontalLine(static_cast<int>(h * 0.68f), 15, w - 15);

        // --- 四个角的螺丝装饰 ---
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
        g.setColour(juce::Colour(0xff222222));
        for (auto& sx : {15.0f, w - 15.0f}) {
            for (auto& sy : {15.0f, h - 15.0f}) {
                g.drawLine(sx - 3, sy, sx + 3, sy, 1.0f);
                g.drawLine(sx, sy - 3, sx, sy + 3, 1.0f);
            }
        }

        // --- Active 指示灯（红绿蓝三色圆点）---
        const auto ledX = w - 130.f;
        const auto ledY = 22.f;
        // 红色
        g.setColour(juce::Colour(0xffff4444));
        g.fillEllipse(ledX, ledY, 8, 8);
        // 黄色
        g.setColour(juce::Colour(0xffffcc00));
        g.fillEllipse(ledX + 12, ledY, 8, 8);
        // 绿色（亮，表示 ACTIVE）
        g.setColour(juce::Colour(0xff44ff44));
        g.fillEllipse(ledX + 24, ledY, 8, 8);
        // 绿色光晕
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

        // 标题
        titleLabel.setBounds(25, 12, 200, 35);

        // ACTIVE 文字（在指示灯右侧）
        activeLabel.setBounds(w - 95, 14, 80, 24);

        // 频响曲线区域（中部）
        responseComponent.setBounds(15, 65, w - 30, static_cast<int>(h * 0.58f));

        // 滑块区域（底部）
        const auto sliderY = static_cast<int>(h * 0.72f);
        constexpr auto sliderH = 40;
        freqLabel.setBounds(25, sliderY - 15, w - 700, 20);
        freqSlider.setBounds(30, sliderY, w - 60, sliderH);
        qLabel.setBounds(25, sliderY + 40, w - 700, 20);
        qSlider.setBounds(30, sliderY + 55, w - 60, sliderH);

        // 底部状态栏
        statusLabel.setBounds(20, h - 28, w - 40, 20);
    }

    void PluginEditor::sliderValueChanged(juce::Slider * slider) {
        if (slider == &freqSlider) {
            // 同步到参数和DSP
            processor.getParameterRefs().freqCenter = static_cast<float>(freqSlider.getValue());
            // processor.getBa
        }
        else if (slider == &qSlider) {
            processor.getParameterRefs().q = static_cast<float>(qSlider.getValue());
        }
        updateDisplay();
    }

    void PluginEditor::updateDisplay() {
        float sampleRate = processor.getBandpassRef().getSampleRate();
        sampleRate = sampleRate > 0 ? sampleRate : Bandpass::defaultSampleRate;
        // 变更频响曲线
        responseComponent.setParameter(
            static_cast<float>(freqSlider.getValue()),
            static_cast<float>(qSlider.getValue()),
            sampleRate);
        // 更新状态栏
        statusLabel.setText(
            "Sample Rate: " + juce::String(static_cast<int>(sampleRate)) + " Hz",
            juce::NotificationType::dontSendNotification);
    }
}
