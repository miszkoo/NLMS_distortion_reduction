/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NLMS_filterAudioProcessorEditor::NLMS_filterAudioProcessorEditor (NLMS_filterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    processorPointer = &p;
    button.onClick = [&]() {processorPointer->resetFilter(); };

    addAndMakeVisible(button);
    button.setButtonText("Filter reset");
    
    setSize (400, 300);
}

NLMS_filterAudioProcessorEditor::~NLMS_filterAudioProcessorEditor()
{
}

//==============================================================================
void NLMS_filterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

   // g.setColour (juce::Colours::white);
   // g.setFont (15.0f);
   // g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void NLMS_filterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    button.setBounds(getWidth() / 2 - 50, getHeight() / 2 - 25, 100, 50);
}
