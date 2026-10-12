#pragma once

namespace bandpass {
    // ============================================================
    // 频响曲线显示组件：根据当前 fc / Q 实时绘制带通响应
    // ============================================================
    class FrequencyResponseComponent : public juce::Component {
    public:
        FrequencyResponseComponent();

        void paint(juce::Graphics &g) override;

        // 更新参数并触发重绘
        void setParameter(float freqCenter, float q, float sampleRate);

    private:
        float freqCenter = Bandpass::defaultFreqCenter;
        float q = Bandpass::defaultQ;
        float sampleRate = Bandpass::defaultSampleRate;

        // 计算带通滤波器在给定频率处的幅度（归一化：0~1）
        float getMagnitude(const float frequency) const;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyResponseComponent)
    };
}