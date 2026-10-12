#pragma once

namespace bandpass {
    class Bandpass {
    public:
        static constexpr float minFreqCenter = 20.f;
        static constexpr float maxFreqCenter = 20000.f;
        static constexpr float minQ = 0.1f;
        static constexpr float maxQ = 10.f;
        static constexpr float defaultFreqCenter = 1000.f;
        static constexpr float defaultQ = 0.7f;
        static constexpr float defaultSampleRate = 44100.f;

        Bandpass() {
            // 构造时初始化平滑器，防止 Multiplicative 初始值为 0（会导致 NaN）
            freqSmoother.setCurrentAndTargetValue(defaultFreqCenter);
            qSmoother.setCurrentAndTargetValue(defaultQ);

            // 设置为带通模式
            bandPassFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        }

        void prepare(const double sampleRate_, const int expectedMaxFramesPerBlock, const int numOutputChannels) {
            // 保存采样率
            sampleRate = static_cast<float>(sampleRate_);

            const juce::dsp::ProcessSpec processSpec{
                .sampleRate = sampleRate_,
                .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
                .numChannels = static_cast<juce::uint32>(numOutputChannels),
            };
            // 准备滤波器，为每个声道创建独立的IIR实例
            bandPassFilter.prepare(processSpec);

            // 设置平滑时间 25ms
            freqSmoother.reset(sampleRate, 0.025);
            qSmoother.reset(sampleRate, 0.025);

            // 初始化滤波器默认参数：中心频率/截止频率、谐振/Q/品质因子
            bandPassFilter.setCutoffFrequency(defaultFreqCenter);
            bandPassFilter.setResonance(defaultQ);
        }

        void process(juce::AudioBuffer<float> &buffer) {
            // ============================================================
            // 块级更新核心逻辑：
            // 1. 取本块起始时刻的平滑值（整块复用这一组系数）
            // 2. 调用 skip() 推进平滑器到本块结束位置（关键！不调则平滑器不动）
            // 3. 若仍在平滑中，用当前值重新生成一次 Biquad 系数
            // 4. 整块音频用同一组系数处理
            // ============================================================

            const int numSamples = buffer.getNumSamples();
            // 步骤1：先记录当前块起始处是否还在平滑中
            const bool needUpdate = freqSmoother.isSmoothing() || qSmoother.isSmoothing();

            // 步骤2：读取本块起始时刻的平滑参数值（只读，不推进状态）
            const float fc = freqSmoother.getCurrentValue();
            const float q = qSmoother.getCurrentValue();

            // 步骤3：推进平滑器内部状态，跨过当前音频块
            // 等价于逐样本调用 numSamples 次 getNextValue()，
            // 但开销极低
            freqSmoother.skip(numSamples);
            qSmoother.skip(numSamples);

            // 步骤4：只要当前块在开始处是需要平滑的，就更新系数
            if (needUpdate) {
                // 更新截止频率/中心频率
                bandPassFilter.setCutoffFrequency(fc);
                // 更新谐振/品质因子
                bandPassFilter.setResonance(q);
            }
            // 步骤5：处理整块音频（替换时处理，直接修改buffer）
            juce::dsp::AudioBlock<float> block(buffer);
            bandPassFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
        }

        void reset() {
            // 重置滤波器内部延迟寄存器（清除历史样本残留，防止爆音）
            bandPassFilter.reset();
            // 平滑器回到默认值（同时设置 current 和 target，无过渡）
            freqSmoother.setCurrentAndTargetValue(defaultFreqCenter);
            qSmoother.setCurrentAndTargetValue(defaultQ);
            bandPassFilter.setCutoffFrequency(defaultFreqCenter);
            bandPassFilter.setResonance(defaultQ);
        }

        void setFreqCenter(const float freqCenter) noexcept {
            // jlimit 做边界保护，然后直接交给平滑器作为目标值
            // setTargetValue 内部会判断：新目标 == 旧目标则直接返回，不重置斜坡
            freqSmoother.setTargetValue(
                juce::jlimit(
                minFreqCenter,
                maxFreqCenter,
                freqCenter));
        }

        void setQ(const float q) noexcept {
            qSmoother.setTargetValue(juce::jlimit(minQ, maxQ, q));
        }

        float getSampleRate() const noexcept {
            return sampleRate;
        }

    private:
        float sampleRate = defaultSampleRate;

        // StateVariableTPTFilter：
        // **State Variable**：状态可变滤波器（State-Variable Filter, SVF），一种经典的模拟滤波器拓扑
        // **TPT**：**Topology-Preserving Transform**（拓扑保持变换）
        juce::dsp::StateVariableTPTFilter<float> bandPassFilter;

        // 频率：乘法平滑（人耳对数感知，倍频程过渡时间一致）
        // Q：线性平滑（人耳对 Q 的变化是线性感知）
        // 平滑器内部已经保存了 target / current / 剩余步数，
        // 不需要外部再写成员变量保存目标值或判断是否需要平滑。
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> freqSmoother;
        juce::SmoothedValue<float> qSmoother;
    };
}
