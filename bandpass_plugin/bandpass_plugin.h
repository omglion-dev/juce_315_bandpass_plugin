/*
==============================================================================

BEGIN_JUCE_MODULE_DECLARATION

   ID:            bandpass_plugin
   vendor:        InnoWisdom
   version:       1.0.0
   name:          OmgBandpass Plugin
   description:   Core of the bandpass plugin
   dependencies:  juce_audio_utils, juce_dsp

   website:
   license:       MIT

END_JUCE_MODULE_DECLARATION

==============================================================================
*/

#pragma once

#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <functional>
#include <ranges>
#include <algorithm>
#include <array>
#include <cmath>
#include <deque>
#include <span>

#include "include/PluginProcessor.h"
#include "include/PluginEditor.h"
#include "include/Bandpass.h"
#include "include/Parameters.h"
#include "include/BandpassLookAndFeel.h"
#include "include/FrequencyResponseComponent.h"