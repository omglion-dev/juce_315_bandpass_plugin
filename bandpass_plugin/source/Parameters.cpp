#include "../include/Bandpass.h"

namespace bandpass {
    namespace {
        auto& addParameterToProcessor(juce::AudioProcessor& processor,  auto&& parameter) {
            auto& parameterRef = *parameter;
            // JUCE requires AudioProcessor to own all registered parameter instances.
            // AudioProcessor will automatically delete all registered parameter instances
            // on plugin destruction.
            // Therefore we must call release() on the unique‑ptr first.
            // release() returns dereferenced raw pointer (T*),
            // and resets the internal raw pointer to nullptr.
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