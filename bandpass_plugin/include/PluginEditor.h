#pragma once
#include "BandpassLookAndFeel.h"
#include "FrequencyResponseComponent.h"

namespace bandpass {
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

        // Custom UI styling instance
        BandpassLookAndFeel lookAndFeel;

        // Frequency‑response plot component
        FrequencyResponseComponent responseComponent;

        // UI slider controls
        juce::Slider freqSlider {"Frequency Center"};
        juce::Slider qSlider {"Q Factor"};

        // Text label widgets
        juce::Label titleLabel {"title", "OMG-BP"};
        juce::Label activeLabel {"active", "ACTIVE"};
        juce::Label statusLabel {"status", ""};
        juce::Label freqLabel {"freq", "Frequency Center"};
        juce::Label qLabel {"q", "Q Factor"};

        // Refresh frequency‑response plot and status labels from current slider values
        void updateDisplay();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginEditor);

    };
}
