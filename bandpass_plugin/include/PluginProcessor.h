#pragma once
#include "Bandpass.h"
#include "Parameters.h"

namespace bandpass {
    class PluginProcessor : public juce::AudioProcessor {
    public:
        PluginProcessor();

        void prepareToPlay(double sampleRate, int expectedMaxFramesPerBlock) override;

        void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

        using AudioProcessor::processBlock;

        void releaseResources() override;

        juce::AudioProcessorEditor *createEditor() override;

        bool hasEditor() const override;

        const juce::String getName() const override;

        bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

        bool acceptsMidi() const override;

        bool producesMidi() const override;

        bool isMidiEffect() const override;

        double getTailLengthSeconds() const override;

        int getNumPrograms() override;

        int getCurrentProgram() override;

        void setCurrentProgram(int index) override;

        const juce::String getProgramName(int index) override;

        void changeProgramName(int index, const juce::String &newName) override;

        void getStateInformation(juce::MemoryBlock &destData) override;

        void setStateInformation(const void *data, int sizeInBytes) override;

        // juce::AudioProcessorParameter* getBypassParameter() const override;

        Parameters& getParameterRefs() noexcept;
        Bandpass& getBandpassRef() noexcept;

        // void readAllLfoSamples(juce::AudioBuffer<float>& bufferToFill);

        /** @brief 以线程安全的方式获取处理器接收到的最新采样率 */
        // double getSampleRateThreadSafe() const noexcept;

    private:
        Bandpass bandpass;
        Parameters parameters {*this}; // 对 this指针解引用

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginProcessor)
    };
}
