/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ItutvAudioProcessorEditor::ItutvAudioProcessorEditor (ItutvAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    botonProcesar.reset (new ToggleButton ("botonProcesar"));
    addAndMakeVisible (botonProcesar.get());
    botonProcesar->setButtonText (TRANS("Processar"));
    botonProcesar->addListener (this);

    botonProcesar->setBounds (24, 96, 256, 64);

    setSize (300, 300);
}

ItutvAudioProcessorEditor::~ItutvAudioProcessorEditor()
{
    botonProcesar = nullptr;
}

//==============================================================================
void ItutvAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}


void ItutvAudioProcessorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == botonProcesar.get())
    {
        processor.procesoActivo = botonProcesar->getToggleState();
    }
}


void ItutvAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
