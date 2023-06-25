/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NLMS_filterAudioProcessor::NLMS_filterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       //.withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       //.withInput  ("Input",  juce::AudioChannelSet::discreteChannels(4), true)
                       .withInput  ("Input",  juce::AudioChannelSet::quadraphonic(), true)
                      #endif
                       //.withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       //.withOutput ("Output", juce::AudioChannelSet::discreteChannels(4), true)
                       .withOutput ("Output", juce::AudioChannelSet::quadraphonic(), true)
                     #endif
                       )
#endif
{
}

NLMS_filterAudioProcessor::~NLMS_filterAudioProcessor()
{
}

//==============================================================================
const juce::String NLMS_filterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NLMS_filterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NLMS_filterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NLMS_filterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NLMS_filterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NLMS_filterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NLMS_filterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NLMS_filterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NLMS_filterAudioProcessor::getProgramName (int index)
{
    return {};
}

void NLMS_filterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NLMS_filterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    h = new double[H];
    bufStaryX = new double[samplesPerBlock];
    bufStaryD = new double[samplesPerBlock];
    bufStaryQ = new double[samplesPerBlock];

    for (int i = 0; i < H; i++) h[i] = 0.0;
    for (int i = 0; i < samplesPerBlock; i++) bufStaryX[i] = 0.0;
    for (int i = 0; i < samplesPerBlock; i++) bufStaryD[i] = 0.0;
    for (int i = 0; i < samplesPerBlock; i++) bufStaryQ[i] = 0.0;

    bufX = new double[2 * samplesPerBlock];
    bufD = new double[2 * samplesPerBlock];
    bufQ = new double[2 * samplesPerBlock];


    e = new double[2 * samplesPerBlock];
    y = new double[2 * samplesPerBlock];
    
    s = new double[H];
    sQ = new double[H]; 

}

void NLMS_filterAudioProcessor::resetFilter()
{
    for (int i = 0; i < H; i++) h[i] = 0.0;
}

void NLMS_filterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NLMS_filterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo() 
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::quadraphonic())
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

void NLMS_filterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    //for (int channel = 0; channel < totalNumInputChannels; ++channel)
    //{
        //auto* channelData = buffer.getWritePointer (channel);
    auto* x = buffer.getWritePointer(0); // szum
    auto* d = buffer.getWritePointer(1); // szum + sygnal
    auto* q = buffer.getWritePointer(2); // kanał do filtracji (szum + sygnal)
    auto* w = buffer.getWritePointer(3); 
    

    N = buffer.getNumSamples();
    N2 = 2 * N;

    // ..do something to the data...

//wyznaczenie sygnału szumu
    for (int i = 0; i < N; i++) {
        //w[i] = 0;
        w[i] = d[i] - x[i];
        x[i] = w[i];
    }

//wyzerowanie wektorów e i y
    for (int i = 0; i < N2; i++)
    {
        e[i] = 0.0;
        y[i] = 0.0;
    }

    //złożenie wektorów
    for (int i = 0; i < N; i++) {
        if (i < N2) {
            bufX[i] = bufStaryX[i];
            bufD[i] = bufStaryD[i];
            bufQ[i] = bufStaryQ[i];
        }
    }
    for (int i = N; i < N2; i++) {
        bufX[i] = x[i - N];
        bufD[i] = d[i - N];
        bufQ[i] = q[i - N];  // to jak jest odkomentowane to się wywala, jeżeli ustawimy c[i -N] to wtedy się wywala 
    }

    //przepisanie aktualnego buforu do buforu historycznego
    for (int i = 0; i < N; i++) {
        bufStaryX[i] = x[i];
        bufStaryD[i] = d[i];
        bufStaryQ[i] = q[i];
    }
    //---------------------------------------------------------------------------
    
    // SPRÓBOWAĆ ZAMIENIĆ SYGNAŁY x i d !!!!!!!!!!!!!!!!!!!!
    // po tym sprawdzić co stanie się z sygnałami e, y oraz q 
    // 
    //---------------------------------------------------------------------------


    // for n=H:N 
    for (int n = N; n < N2; n++) { //for (int n = H; n < N; ++n) // for (int n = N + H; n<N2; ++n)
        // s = x[n:-1:n-H+1]
        for (int i = 0; i < H; i++) s[i] = bufX[n - i];
        //s do filtracji kanału q (dla wzmacniacza)
        for (int i = 0; i < H; i++) sQ[i] = bufX[n - i]; // chyba można by to przerobić i używać cały czas s
                                                         // to się wzbudza i robi echo bo na wyjście q (3) podajemy 
                                                         // ten sam sygnał co wcześniej czyli sygnał x

        // e[n] = d[n] - s'*h
        e[n] = bufD[n];
        for (int i = 0; i < H; i++) e[n] -= s[i] * h[i];

        // (1) filtracja kanału q (dla wzmacniacza)
        //for (int i = 0; i < H; i++) bufQ[n] -= sQ[i] * h[i]; // jeżeli nie będzie działać  
        //to pewnie musi być wcześniej bo tu działa już na nowych współczynniakch
        // (1)

        // NLMS: h = h + ((2*μ)/(γ+s'*s))*e[n]*s; //LMS: h = h + μ*e[n]*s
        ss = 0.0;
        for (int i = 0; i < H; i++) ss += s[i] * s[i];
        mianow = 1 / (gamma + ss);
        for (int i = 0; i < H; i++) h[i] += (2 * mu) * mianow * e[n] * s[i];

        
        // y[n] = s'*h                                         
        for (int i = 0; i < H; i++) y[n] += s[i] * h[i];

        // BY POPRAWIĆ DZIAŁANIE STANDARDOWEGO FILTRU NLMS WARTO STWORZYĆ BUFOR DLA SYGANŁU D
        // I PRZEFITLROWAĆ SYGAŁ BY UZYSKAĆ SYGNAŁ BŁĘDU DOPIERO TERAZ BO W TYM MIEJSCU 
        // MAMY WSPÓŁCZYNNIKI PO AKTUALIZACJI 
        

        // (2) filtracja kanału q (dla wzmacniacza)
        for (int i = 0; i < H; i++) bufQ[n] -= sQ[i] * h[i]; // jeżeli nie będzie działać  
        //for (int i = 0; i < H; i++) bufQ[n] += sQ[i] * h[i]; // wersja v2 do sprawdzenia  
        //to pewnie musi być wcześniej bo tu działa już na nowych współczynniakch
        // (2)
        //Sygnał cichy ale praktycznie idealnie odszumiony uzyskuje się jeżeli odejmie się od 
        //sygnału błędu wyliczonego wcześniej, sygnał błędu wyliczony po aktualizacji współczynników 


    }

    //wyjscie
      //return e,y
    for (int n = N; n < N2; n++) {
        x[n - N] = e[n];
        d[n - N] = y[n];
        q[n - N] = bufQ[n];
        //q[n - N] = e[n];
    }
    // delete[] e;
    // delete[] y;
    // delete[] s;
    //}
}

//==============================================================================
bool NLMS_filterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NLMS_filterAudioProcessor::createEditor()
{
    return new NLMS_filterAudioProcessorEditor (*this);
}

//==============================================================================
void NLMS_filterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NLMS_filterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NLMS_filterAudioProcessor();
}
