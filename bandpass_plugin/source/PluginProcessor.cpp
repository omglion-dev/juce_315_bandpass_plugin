namespace bandpass {
    PluginProcessor::PluginProcessor()
        : AudioProcessor(
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    }

    const juce::String PluginProcessor::getName() const {
        // JUCE_315_PLUGIN_NAME is defined in CMakeLists.txt
        return JUCE_315_PLUGIN_NAME;
    }

    bool PluginProcessor::acceptsMidi() const {
        return false;
    }

    bool PluginProcessor::producesMidi() const {
        return false;
    }

    bool PluginProcessor::isMidiEffect() const {
        return false;
    }

    double PluginProcessor::getTailLengthSeconds() const {
        return 0.0;
    }

    int PluginProcessor::getNumPrograms() {
        // Some hosts don't cope very well if you tell them there are 0 programs, so
        // this should be at least 1, even if you're not really implementing programs.
        return 1;
    }

    int PluginProcessor::getCurrentProgram() {
        return 0;
    }

    void PluginProcessor::setCurrentProgram(int index) {
        juce::ignoreUnused(index);
    }

    const juce::String PluginProcessor::getProgramName(int index) {
        juce::ignoreUnused(index);
        return "None";
    }

    void PluginProcessor::changeProgramName(int index,
                                            const juce::String &newName) {
        juce::ignoreUnused(index, newName);
    }

    // Host calls prepareToPlay() to inform plugin of sample‑rate;
    // also passes expected maximum block‑size.
    // Host guarantees audio‑buffer frame count will not exceed this value.
    // Plugin shall pre‑allocate all required audio‑processing memory based on these two values.
    void PluginProcessor::prepareToPlay(double sampleRate,
                                        int expectedMaxFramesPerBlock) {
        // Read total input‑channel count instead of output‑channel count,
        // to avoid issues when output‑channel count exceeds input‑channel count
        bandpass.prepare(
            sampleRate,
            expectedMaxFramesPerBlock,
            getTotalNumInputChannels());
    }

    void PluginProcessor::releaseResources() {
        // When playback stops, you can use this as an opportunity to free up any
        // spare memory, etc.
        bandpass.reset();
    }

    bool PluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
        // This is the place where you check if the layout is supported.
        // In this template code we only support mono or stereo.
        // Some plugin hosts, such as certain GarageBand versions, will only
        // load plugins that support stereo bus layouts.
        if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
            layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
            return false;
        }

        // This checks if the input layout matches the output layout
        if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
            return false;
        }

        return true;
    }

    void PluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                       juce::MidiBuffer &midiMessages) {
        // Core audio‑sample processing happens in processBlock().
        // Real‑time audio processing imposes strict execution‑time constraints:
        // processBlock() must finish execution faster than
        // the time‑duration represented by the audio buffer.
        // Exceeding time budget will cause audio drop‑outs.

        juce::ignoreUnused(midiMessages);

        juce::ScopedNoDenormals noDenormals;
        const auto totalNumInputChannels = getTotalNumInputChannels();
        const auto totalNumOutputChannels = getTotalNumOutputChannels();

        // In case we have more outputs than inputs, this code clears any output
        // channels that didn't contain input data, (because these aren't
        // guaranteed to be empty - they may contain garbage).
        // This is here to avoid people getting screaming feedback
        // when they first compile a plugin, but obviously you don't need to keep
        // this code if your algorithm always overwrites all the output channels.
        for (const auto channelToClear:
             std::views::iota(totalNumInputChannels, totalNumOutputChannels)) {
            buffer.clear(channelToClear, 0, buffer.getNumSamples());
        }

        // update parameters
        bandpass.setFreqCenter(parameters.freqCenter.get());
        bandpass.setQ(parameters.q.get());

        // apply bandpss
        bandpass.process(buffer);
    }

    bool PluginProcessor::hasEditor() const {
        return true;
    }

    // This function will be called to create an instance of the editor
    juce::AudioProcessorEditor *PluginProcessor::createEditor() {
        return new PluginEditor(*this);
    }

    /**
     * @brief Automatically invoked when DAW saves project;
     * serializes all plugin parameter states
     * @param destData Expandable memory‑block provided by host;
     * serialized JSON data will be written here
     * @details 1. Create memory output stream bound to destData, true enables append‑mode
     *          2. Serialize plugin parameters to JSON text and write into memory‑block
     * @note Host owns destData memory; binary content is persisted
     *       inside project file after return.
     *       All persistable settings must be written to destData,
     *       otherwise values are lost after reload.
     */
    void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
        // You should use this method to store your parameters in the memory block.
        // You could do that either as raw data, or use the XML or ValueTree classes
        // as intermediaries to make it easy to save and load complex data.
        juce::ignoreUnused(destData);
    }

    /**
     * @brief Automatically invoked when DAW loads project / preset; restore plugin parameters from serialized binary data
     * @param data Read‑only raw binary pointer supplied by host, holds previously serialized JSON bytes
     * @param sizeInBytes Total byte‑length of memory‑block, used for read‑boundary checking
     * @details 1. Construct input‑stream over read‑only memory; false avoids copying memory for performance
     *          2. Execute JSON deserialization and write restored values back to plugin parameter container
     *          3. Print debug error log on parse failure (only active in Debug build; stripped for Release)
     *          4. Force‑sync smoothers and disable ramping transition to avoid audio clicks on project‑load
     * @warning 1. data pointer is const read‑only; do not modify, expand or free this memory region
     *          2. After loading state, all parameter smoothers should be forced‑refreshed
     *          3. DBG macro is for debugging only; never place business logic inside DBG blocks
     */
    void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
        // You should use this method to restore your parameters from this memory
        // block, whose contents will have been created by the getStateInformation()
        // call.
        juce::ignoreUnused(data, sizeInBytes);
    }

    Parameters& PluginProcessor::getParameterRefs() noexcept {
        return parameters;
    }

    Bandpass& PluginProcessor::getBandpassRef() noexcept {
        return bandpass;
    }

}

// This creates new instances of the plugin.
// This function definition must be in the global namespace.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new bandpass::PluginProcessor();
}
