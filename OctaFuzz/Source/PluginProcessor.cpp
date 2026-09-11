/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OctaFuzzAudioProcessor::OctaFuzzAudioProcessor()
: AudioProcessor (BusesProperties()
                  .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                  .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

OctaFuzzAudioProcessor::~OctaFuzzAudioProcessor()
{
  // 모노(1채널), 4배 오버샘플링(factor 2 = 2^2), 최고급 필터 사용
  oversampler = std::make_unique<juce::dsp::Oversampling<float>>(1, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR);
}

//==============================================================================
const juce::String OctaFuzzAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OctaFuzzAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OctaFuzzAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OctaFuzzAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OctaFuzzAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OctaFuzzAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OctaFuzzAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OctaFuzzAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OctaFuzzAudioProcessor::getProgramName (int index)
{
    return {};
}

void OctaFuzzAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OctaFuzzAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  const auto numChannels = juce::jmax(1, getTotalNumInputChannels());
  
  oversampler = std::make_unique<juce::dsp::Oversampling<float>>
  (
   numChannels,
   2,
   juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR
  );
  
  oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));
  oversampler->reset();
  
  double oversampledRate = sampleRate * oversampler->getOversamplingFactor();
  
  fuzzModule.prepare(oversampledRate);
}

void OctaFuzzAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OctaFuzzAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OctaFuzzAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  
  juce::ScopedNoDenormals noDenormals;
  
  float currentFuzzAmount = 0.8f;
  fuzzModule.setParameter(currentFuzzAmount);
  
  juce::dsp::AudioBlock<float> audioBlock(buffer);
  
  juce::dsp::AudioBlock<float> upsampledBlock = oversampler->processSamplesUp(audioBlock);
  
  float* channelData = upsampledBlock.getChannelPointer(0);
  int numSamples = static_cast<int>(upsampledBlock.getNumSamples());
  
  for (int i = 0; i < numSamples; ++i) {
    channelData[i] = fuzzModule.processSample(channelData[i]);
  }
  
  oversampler->processSamplesDown(audioBlock);
}

//==============================================================================
bool OctaFuzzAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OctaFuzzAudioProcessor::createEditor()
{
    return new OctaFuzzAudioProcessorEditor (*this);
}

//==============================================================================
void OctaFuzzAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OctaFuzzAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OctaFuzzAudioProcessor();
}

// 파라미터 정의 (ID: "FUZZ_AMOUNT", 기본값: 0.5, 범위: 0.0 ~ 1.0)
juce::AudioProcessorValueTreeState::ParameterLayout OctaFuzzAudioProcessor::createParameterLayout()
{
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  params.push_back (std::make_unique<juce::AudioParameterFloat>
                    (juce::ParameterID { "FUZZ_AMOUNT", 1 },
                     "Fuzz",
                     juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f),
                     0.5f
                     )
                    );
  
  return { params.begin(), params.end() };
}
