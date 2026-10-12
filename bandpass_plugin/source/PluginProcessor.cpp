namespace bandpass {
    PluginProcessor::PluginProcessor()
        : AudioProcessor(
            BusesProperties()
            .withInput("Input", juce::AudioChannelSet::stereo(), true)
            .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    }

    const juce::String PluginProcessor::getName() const {
        // JUCE_315_PLUGIN_NAME 在 CMakeLists.txt 中定义
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

    // 宿主通过调用 prepareToPlay() 方法，将采样率告知插件；
    // 同时还会传入预期的最大块大小，相当于宿主告知插件：「我传入的音频缓冲区，帧数不会超过这个数值」。
    // 插件应当基于这两个信息，预先分配好音频处理所需的全部缓冲区内存。
    void PluginProcessor::prepareToPlay(double sampleRate,
                                        int expectedMaxFramesPerBlock) {
        // 之所以获取总输入通道的数量，而不是总输出通道的数量，
        // 是为了避免出现输出通道数量超过输入通道数量的情况
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
        // 处理器在 processBlock()
        // 中的核心任务就是处理输入采样。但实时音频信号处理的特性，
        // 对该方法的执行时长有严格约束：简单来说，
        // processBlock() 的执行耗时必须短于它所处理的音频缓冲区对应的时长，
        // 无法按时完成会导致严重的后果。

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
        // 如果输出通道数量多于输入通道，这段代码会清空所有未承载输入数据的输出
        // 通道（因为这些通道不一定是空的，可能存有无效垃圾数据）。
        // 添加这段代码是为了避免用户在首次编译插件时听到刺耳的反馈噪音。
        // 但如果你的算法会覆盖全部输出通道，那么显然就不需要保留这段代码。
        for (const auto channelToClear:
             std::views::iota(totalNumInputChannels, totalNumOutputChannels)) {
            buffer.clear(channelToClear, 0, buffer.getNumSamples());
        }

        // update parameters
        bandpass.setFreqCenter(parameters.freqCenter.get());
        bandpass.setQ(parameters.q.get());

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
     * @brief DAW宿主保存【工程】时自动回调，序列化输出插件全部参数状态
     * @param destData 宿主提供可扩容内存块，所有序列化JSON数据将写入此容器
     * @details 1. 创建绑定destData的内存输出流，true代表追加写入模式
     *          2. 调用JsonSerializer将当前插件参数序列化为JSON文本写入内存块
     * @note 宿主持有destData内存，函数执行完毕后会将二进制存入工程文件持久化；
     *       所有需要持久化的配置必须在此写入destData，否则重启丢失
     */
    void PluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
        // juce::MemoryOutputStream memoryOutputStream{destData, true};
        // JsonSerializer::serialize(parameters, memoryOutputStream);
        juce::ignoreUnused(destData);
    }

    /**
     * @brief DAW加载【工程】/载入预设时自动回调，从宿主内存反序列化恢复插件参数
     * @param data 宿主传入只读原始二进制内存指针，存储之前序列化的JSON字节
     * @param sizeInBytes 内存块总字节长度，用于限定读取边界
     * @details 1. 基于只读内存创建输入流，false表示不拷贝内存节省开销
     *          2. 执行JSON反序列化，将存档数值回填插件Parameters参数容器
     *          3. 解析失败时通过DBG打印错误日志（仅Debug构建生效，Release自动移除）
     *          4. 强制同步旁路平滑器，禁用渐变过渡，避免加载时音频爆音
     * @warning 1. data为const只读，不可修改、扩容、释放该内存
     *          2. 所有带参数平滑器加载状态后都应调用setBypassForced强制刷新
     *          3. DBG仅用于开发调试，切勿写入业务逻辑到宏内部
     */
    void PluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
        // juce::MemoryInputStream memoryInputStream{data, static_cast<size_t>(sizeInBytes), false};
        //
        // if (const auto result = JsonSerializer::deserialize(memoryInputStream, parameters); result.failed()) {
        //     DBG(result.getErrorMessage());
        // }
        // bypassTransitionSmoother.setBypassForced(parameters.bypassed.get());
        juce::ignoreUnused(data, sizeInBytes);
    }

    Parameters& PluginProcessor::getParameterRefs() noexcept {
        return parameters;
    }

    Bandpass& PluginProcessor::getBandpassRef() noexcept {
        return bandpass;
    }

    // juce::AudioProcessorParameter *PluginProcessor::getBypassParameter() const {
    //     return &parameters.bypassed;
    // }

    // double PluginProcessor::getSampleRateThreadSafe() const noexcept {
    //     return currentSampleRate;
    // }

    // void PluginProcessor::readAllLfoSamples(juce::AudioBuffer<float> &bufferToFill) {
    //     tremolo.readAllLfoSamples(bufferToFill);
    // }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new bandpass::PluginProcessor();
}
