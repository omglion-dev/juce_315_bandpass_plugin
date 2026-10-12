#include "../include/Bandpass.h"

namespace bandpass {
    namespace {
        auto& addParameterToProcessor(juce::AudioProcessor& processor,  auto&& parameter) {
            auto& parameterRef = *parameter;
            // JUCE 要求由AudioProcessor 统一管理所有注册的参数实例，
            // 即 AudioProcessor 会在插件销毁时自动删除所有注册的参数实例
            // 所以，这里要先调用独占指针的release() 方法，
            // 该方法会返回内部裸指针的解引用（T*），并将自身内部裸指针置空（nullptr）
            processor.addParameter(parameter.release());
            return parameterRef;
        }
    }

    juce::AudioParameterFloat& createFreqCenterParameter(juce::AudioProcessor& processor) {
        constexpr int versionHint = 1;
        auto parameter = std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("frequency.center", versionHint),
            "Frequency center",
            juce::NormalisableRange(
                Bandpass::minFreqCenter,
                Bandpass::maxFreqCenter,
                1.f
                ),
                Bandpass::defaultFreqCenter,
                juce::AudioParameterFloatAttributes{}.withLabel("Hz")
        );
        return addParameterToProcessor(processor, std::move(parameter));
    }

    juce::AudioParameterFloat& createQParameter(juce::AudioProcessor& processor) {
        constexpr int versionHint = 1;
        auto parameter = std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("q", versionHint),
            "Q",
            juce::NormalisableRange(
                Bandpass::minQ,
                Bandpass::maxQ,
                0.1f),
                Bandpass::defaultQ
        );
        return addParameterToProcessor(processor, std::move(parameter));
    }

    Parameters::Parameters(juce::AudioProcessor& processor)
        : freqCenter(createFreqCenterParameter(processor)),
          q(createQParameter(processor))
    {}
}