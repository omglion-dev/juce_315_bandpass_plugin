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
            // Initialize smoothers in constructor,
            // avoid NaN caused by initial value 0 for Multiplicative smoother
            freqSmoother.setCurrentAndTargetValue(defaultFreqCenter);
            qSmoother.setCurrentAndTargetValue(defaultQ);
            // Set filter mode to band‑pass
            bandPassFilter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
        }

        void prepare(const double sampleRate_, const int expectedMaxFramesPerBlock, const int numOutputChannels) {
            // Store sample‑rate
            sampleRate = static_cast<float>(sampleRate_);
            const juce::dsp::ProcessSpec processSpec{
                .sampleRate = sampleRate_,
                .maximumBlockSize = static_cast<juce::uint32>(expectedMaxFramesPerBlock),
                .numChannels = static_cast<juce::uint32>(numOutputChannels),
            };
            // Prepare filter, allocate independent IIR state for each audio channel
            bandPassFilter.prepare(processSpec);
            // Set smoothing time to 25ms
            freqSmoother.reset(sampleRate, 0.025);
            qSmoother.reset(sampleRate, 0.025);
            // Initialize filter with default center‑frequency and Q‑factor
            bandPassFilter.setCutoffFrequency(defaultFreqCenter);
            bandPassFilter.setResonance(defaultQ);
        }

        void process(juce::AudioBuffer<float> &buffer) {
            // =====================================================================
            // Block‑wise parameter update logic
            // 1. Read smoothed parameter values at the start of current audio block
            // 2. Call skip() to advance smoother state to end of block (critical)
            // 3. Re‑compute filter coefficients if smoothing is still in‑progress
            // 4. Process entire audio block using one set of coefficients
            // =====================================================================
            const int numSamples = buffer.getNumSamples();
            // Step 1: Check whether any parameter is still smoothing at block start
            const bool needUpdate = freqSmoother.isSmoothing() || qSmoother.isSmoothing();
            // Step 2: Read values at block start (read‑only, does NOT advance state)
            const float fc = freqSmoother.getCurrentValue();
            const float q = qSmoother.getCurrentValue();
            // Step 3: Advance smoother state across this whole block
            // Equivalent to calling getNextValue() numSamples times, low‑overhead
            freqSmoother.skip(numSamples);
            qSmoother.skip(numSamples);
            // Step 4: Update filter coefficients only if smoothing is active
            if (needUpdate) {
                bandPassFilter.setCutoffFrequency(fc);
                bandPassFilter.setResonance(q);
            }
            // Step5: Process audio block in‑place, modify buffer contents
            juce::dsp::AudioBlock<float> block(buffer);
            bandPassFilter.process(juce::dsp::ProcessContextReplacing<float>(block));
        }

        void reset() {
            // Clear filter delay states to remove leftover sample history, prevent audio clicks
            bandPassFilter.reset();
            // Reset smoothers to default values; set both current and target for instant jump without ramping
            freqSmoother.setCurrentAndTargetValue(defaultFreqCenter);
            qSmoother.setCurrentAndTargetValue(defaultQ);
            bandPassFilter.setCutoffFrequency(defaultFreqCenter);
            bandPassFilter.setResonance(defaultQ);
        }

        void setFreqCenter(const float freqCenter) noexcept {
            // Clamp value to valid range, assign as smoother target value
            // setTargetValue skips work internally if new target equals existing target
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
        // StateVariableTPTFilter
        // State‑Variable Filter(SVF): classic analog filter topology
        // TPT: Topology‑Preserving Transform, discrete‑time transformation method
        juce::dsp::StateVariableTPTFilter<float> bandPassFilter;

        // Frequency uses multiplicative smoothing:
        // Consistent octave‑wise transition speed for logarithmic human pitch perception
        // Q‑factor uses linear smoothing: human perception of Q‑change is linear
        // Smoother internally stores target, current value and remaining interpolation steps.
        // No extra member variables required to hold target or judge smoothing status.
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> freqSmoother;
        juce::SmoothedValue<float> qSmoother;
    };
}
