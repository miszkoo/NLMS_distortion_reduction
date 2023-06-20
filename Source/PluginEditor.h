/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class NLMS_filterAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NLMS_filterAudioProcessorEditor (NLMS_filterAudioProcessor&);
    ~NLMS_filterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    juce::TextButton button;
    NLMS_filterAudioProcessor& audioProcessor;
    NLMS_filterAudioProcessor* processorPointer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NLMS_filterAudioProcessorEditor)
};
