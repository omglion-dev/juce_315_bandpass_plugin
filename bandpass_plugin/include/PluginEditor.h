#pragma once
#include "BandpassLookAndFeel.h"
#include "FrequencyResponseComponent.h"

namespace bandpass {
    // ============================================================
    // 自定义 LookAndFeel：实现设计图中的蓝色发光水平滑块
    // ============================================================
    class PluginEditor
        : public juce::AudioProcessorEditor,
          public juce::Slider::Listener {
    public:
        explicit PluginEditor(PluginProcessor& processorRef);
        ~PluginEditor() override;

        void paint(juce::Graphics& g) override;
        void resized() override;
        void sliderValueChanged(juce::Slider *) override;
    private:
        PluginProcessor& processor;

        // 自定义外观
        BandpassLookAndFeel lookAndFeel;

        // 频响曲线
        FrequencyResponseComponent responseComponent;

        // slider 滑块
        juce::Slider freqSlider {"Frequency Center"};
        juce::Slider qSlider {"Q Factor"};

        // 标签
        juce::Label titleLabel {"title", "OMG-BP"};
        juce::Label activeLabel {"active", "ACTIVE"};
        juce::Label statusLabel {"status", ""};
        juce::Label freqLabel {"freq", "Frequency Center"};
        juce::Label qLabel {"q", "Q Factor"};

        // 根据滑块当前值更新频响曲线和数值标签
        void updateDisplay();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor);

    };
}
