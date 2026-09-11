/*
 ==============================================================================
 
 This file contains the basic framework code for a JUCE plugin processor.
 
 ==============================================================================
 */

#pragma once

#include <JuceHeader.h>
#include "Fuzz.h"

//==============================================================================
/**
 */
class OctaFuzzAudioProcessor  : public juce::AudioProcessor
{
  public:
  juce::AudioProcessorValueTreeState apvts; // GUI 연동을 위해 public으로 생성
  
  //==============================================================================
  OctaFuzzAudioProcessor();
  ~OctaFuzzAudioProcessor() override;
  
  //==============================================================================
  void prepareToPlay (double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  
#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
  
  void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  
  //==============================================================================
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;
  
  //==============================================================================
  const juce::String getName() const override;
  
  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;
  
  //==============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram (int index) override;
  const juce::String getProgramName (int index) override;
  void changeProgramName (int index, const juce::String& newName) override;
  
  //==============================================================================
  void getStateInformation (juce::MemoryBlock& destData) override;
  void setStateInformation (const void* data, int sizeInBytes) override;
  
  private:
  
  Fuzz fuzzModule;
  std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
  static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
  
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OctaFuzzAudioProcessor)
};
