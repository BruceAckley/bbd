/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BbdAudioProcessor::BbdAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("mix", this);
    treeState.addParameterListener("regen", this);
    treeState.addParameterListener("delay", this);
    treeState.addParameterListener("modulate", this);
}

BbdAudioProcessor::~BbdAudioProcessor()
{
    treeState.removeParameterListener("mix", this);
    treeState.removeParameterListener("regen", this);
    treeState.removeParameterListener("delay", this);
    treeState.removeParameterListener("modulate", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout BbdAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;

    // TODO: Set sensible defaults
    params.push_back(std::move(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", -24.0, 24.0, 0.0)));
    params.push_back(std::move(std::make_unique<juce::AudioParameterFloat>("regen", "Regen", -24.0, 24.0, 0.0)));
    params.push_back(std::move(std::make_unique<juce::AudioParameterFloat>("delay", "Delay", -24.0, 24.0, 0.0)));
    params.push_back(std::move(std::make_unique<juce::AudioParameterBool>("modulate", "Modulate", false)));

    return { params.begin(), params.end() };
}

void BbdAudioProcessor::parameterChanged(const juce::String& parameterId, float newValue)
{
    if (parameterId == "mix")
    {
        mix = newValue;
    }

    if (parameterId == "regen") {
        regen = newValue;
    }

    if (parameterId == "delay") {
        delay = newValue;
    }

    if (parameterId == "modulate") {
        modulate = newValue;  // float 1 or 0 works for boolean
    }
}

//==============================================================================
const juce::String BbdAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BbdAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BbdAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BbdAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BbdAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BbdAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BbdAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BbdAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BbdAudioProcessor::getProgramName (int index)
{
    return {};
}

void BbdAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BbdAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Reset parameters to their values in the tree state
    mix = *treeState.getRawParameterValue("mix");
    regen = *treeState.getRawParameterValue("regen");
    delay = *treeState.getRawParameterValue("delay");
    modulate = *treeState.getRawParameterValue("modulate");
}

void BbdAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BbdAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void BbdAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // House keeping
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Guts
    juce::dsp::AudioBlock<float> block(buffer);

    for (int channel = 0; channel < block.getNumChannels(); ++channel) {
        auto* channelData = block.getChannelPointer(channel);

        for (int sample = 0; sample < block.getNumSamples(); ++sample) {
            // Do something interesting here
        }
    }
}

//==============================================================================
bool BbdAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BbdAudioProcessor::createEditor()
{
    //return new BbdAudioProcessorEditor (*this);

    // UI prototyping
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void BbdAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void BbdAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));

    if (tree.isValid()) {
        // Could get invalid if you have two versions of this plugin and you're making changes
        // to one. It can do weird stuff.

        treeState.state = tree;

        mix = *treeState.getRawParameterValue("mix");
        regen = *treeState.getRawParameterValue("regen");
        delay = *treeState.getRawParameterValue("delay");
        modulate = *treeState.getRawParameterValue("modulate");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BbdAudioProcessor();
}
