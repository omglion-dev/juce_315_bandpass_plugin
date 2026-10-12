#pragma once

namespace bandpass {
    struct Parameters {
        explicit Parameters(juce::AudioProcessor &);

        juce::AudioParameterFloat& freqCenter;
        juce::AudioParameterFloat& q;

        JUCE_DECLARE_NON_COPYABLE(Parameters);
        JUCE_DECLARE_NON_MOVEABLE(Parameters);
    };
}