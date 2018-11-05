/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ItutvAudioProcessor::ItutvAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Inicializacion de las variables del código
    // Todas las variables necesarias para el filtro se ponen en 0
    xn_1[0] = 0; xn_1[1] = 0; xn_1[2] = 0;
    yn_1[0] = 0; yn_1[1] = 0; yn_1[2] = 0;
    // ==========================================================
    xn_2[0] = 0; xn_2[1] = 0; xn_2[2] = 0;
    yn_2[0] = 0; yn_2[1] = 0; yn_2[2] = 0;
    // Se inicializan los coeficientes de los filtros
    // Coeficientes del Peaking
    coeffPeak_A[0] = 1;
    coeffPeak_A[1] = -1.69065929318241;
    coeffPeak_A[2] = 0.73248077421585;
    
    coeffPeak_B[0] = 1.53512485958697;
    coeffPeak_B[1] = -2.69169618940638;
    coeffPeak_B[2] = 1.19839281085285;
    
    // Coeficientes del pasa altos
    coeffPasaAltos_A[0] = 1;
    coeffPasaAltos_A[1] = -1.99004745483398;
    coeffPasaAltos_A[2] = 0.99007225036621;

    coeffPasaAltos_B[0] = 1;
    coeffPasaAltos_B[1] = -2;
    coeffPasaAltos_B[2] = 1;
    
    //===========================================================
    contTonoPuro = 0;
    //===========================================================
    
    umbral = -24; // Umbral estipulado por la norma
    
    attackTime = 10; // Tiempos de ataque y Release en milisegundos
    releaseTime = 500;
    
    valRef = 0;
    Yl_Prev = 0;
    
    // ==========================================================
    // Otras variables
    procesoActivo = false;
    contadorBufferCircular = 0;
    RMS_Value = -70;
}

ItutvAudioProcessor::~ItutvAudioProcessor()
{
}

//==============================================================================
const String ItutvAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ItutvAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ItutvAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ItutvAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ItutvAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ItutvAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ItutvAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ItutvAudioProcessor::setCurrentProgram (int index)
{
}

const String ItutvAudioProcessor::getProgramName (int index)
{
    return {};
}

void ItutvAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ItutvAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // 400 ms, pero en muestras
    tiempoDeBufferEnMuestras = sampleRate * 0.4;
    
    // 75% del Buffer, en Muestras
    tiempoDeBuffer75 = sampleRate * 0.3;
    
    // Si el buffer circular está vacío
    if (bufferCircular == nullptr)
    {
        // Se le da un tamaño de 400ms (En muestras)
        bufferCircular = new float[tiempoDeBufferEnMuestras];
        // Se llena el buffer circular de ceros
        memset(bufferCircular, 0, sizeof(float) * tiempoDeBufferEnMuestras);
    }
    // Se devuelve el contador a la primera posicion
    contadorBufferCircular = 0;
    
    // Se convierten las constantes de tiempo del compresor
    // de segundos, a muestras
    tauAttackConstant = exp( -1 / (attackTime * sampleRate * 0.001));
    tauReleaseConstant = exp( -1 / (releaseTime * sampleRate * 0.001));
    
    //=======================================================
    FrecMuestreo = sampleRate;
}

void ItutvAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ItutvAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void ItutvAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    // auto totalNumOutputChannels = getTotalNumOutputChannels();

    if (procesoActivo)
    {
        if (totalNumInputChannels == 1) // Si es mono
        {
            // Se toma el vector mono
            float* channelMono = buffer.getWritePointer(0);
            // Un ciclo for para recorrer el vector
            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                /*
                 float Seno = sin(2*3.14159*(contTonoPuro/FrecMuestreo) * 1000);
                 if(++contTonoPuro > FrecMuestreo) { contTonoPuro = 0; }
                 channelL[i] = Seno;
                 channelR[i] = Seno;
                 */
                
                // Se saca el mono de los dos canales
                float sumaMono = channelMono[i];
                // Filtro de Campana
                xn_1[0] = sumaMono;
                
                yn_1[0] = (coeffPeak_B[0]*xn_1[0]) + (coeffPeak_B[1]*xn_1[1]) + (coeffPeak_B[2]*xn_1[2]) - (coeffPeak_A[1]*yn_1[1])-(coeffPeak_A[2]*yn_1[2]);
                
                xn_1[2] = xn_1[1];
                xn_1[1] = xn_1[0];
                
                yn_1[2] = yn_1[1];
                yn_1[1] = yn_1[0];
                
                // Filtro Pasa Altas
                xn_2[0] = yn_1[0];
                
                yn_2[0] = (coeffPasaAltos_B[0]*xn_2[0]) + (coeffPasaAltos_B[1]*xn_2[1]) + (coeffPasaAltos_B[2]*xn_2[2]) - (coeffPasaAltos_A[1]*yn_2[1])-(coeffPasaAltos_A[2]*yn_2[2]);
                
                xn_2[2] = xn_2[1];
                xn_2[1] = xn_2[0];
                
                yn_2[2] = yn_2[1];
                yn_2[1] = yn_2[0];
                
                // channelL[i] = yn_2[0];
                // channelR[i] = yn_2[0];
                
                // =================================================================
                
                bufferCircular[contadorBufferCircular] = yn_2[0];
                
                contadorBufferCircular = contadorBufferCircular + 1;
                
                // Si el contador sobrepasa el tamaño del buffer
                // Se reinicia
                if (contadorBufferCircular > tiempoDeBufferEnMuestras)
                {
                    contadorBufferCircular = 0;
                }
                // Durante cada buffer se ingresa el 25% de muestras nuevas
                // Luego se calcula el RMS segun la norma con el 75% de muestras
                // Viejas
                if (contadorBufferCircular == (tiempoDeBuffer75 - 1))
                {
                    RMS_Value = calculaRMS(bufferCircular, tiempoDeBufferEnMuestras);
                    RMS_Value = -0.691 + 10*log10(RMS_Value);
                }
                
                // =================================================================
                // Segmento de Compresion
                
                float valorObjetivo = 0;
                // Si la sonoridad esta por encima del umbral
                // El valor objetivo de la senal es el umbral
                if (RMS_Value >= umbral)
                    valorObjetivo = umbral;
                else
                    valorObjetivo = RMS_Value;
                // Si no, el valor objetivo es el mismo de sonoridad
                
                // El delta de compresion es que tanto hay que multiplicar
                // la senal para que alcance el nivel deseado
                float deltaCompresion = RMS_Value - valorObjetivo;
                
                if (i == 0)
                    valRef = deltaCompresion;
                
                float Yl_Actual;
                
                // Un compresor es un circuito RC, por lo que las constantes de tiempo
                // se pueden expresar como un filtro, de primer orden
                // Este es el filtrado
                if (valRef > Yl_Prev)
                {
                    Yl_Actual = tauAttackConstant * Yl_Prev + (1 - tauAttackConstant) * deltaCompresion;
                }
                else
                {
                    Yl_Actual = tauReleaseConstant * Yl_Prev + (1 - tauReleaseConstant) * deltaCompresion;
                }
                // Todo se esta calculando en dB, por lo que se debe pasar a decimal
                // nuevamente
                float gainControl = pow(10, (-Yl_Actual/20));
                Yl_Prev = Yl_Actual;
                
                // Se aplica el factor de ganancia a la senal
                
                channelMono[i] = channelMono[i] * gainControl;
            }
        }
        else if (totalNumInputChannels == 2) // Si es estereo
        {
            // Se toma el vector L
            float* channelL = buffer.getWritePointer(0);
            // Se toma el vector R
            float* channelR = buffer.getWritePointer(1);
            // Un ciclo for para recorrer el vector
            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                /*
                float Seno = sin(2*3.14159*(contTonoPuro/FrecMuestreo) * 1000);
                if(++contTonoPuro > FrecMuestreo) { contTonoPuro = 0; }
                channelL[i] = Seno;
                channelR[i] = Seno;
                */
                
                // Se saca el mono de los dos canales
                float sumaMono = (channelL[i] + channelR[i]) / 2;
                // Filtro de Campana
                xn_1[0] = sumaMono;
                
                yn_1[0] = (coeffPeak_B[0]*xn_1[0]) + (coeffPeak_B[1]*xn_1[1]) + (coeffPeak_B[2]*xn_1[2]) - (coeffPeak_A[1]*yn_1[1])-(coeffPeak_A[2]*yn_1[2]);
                
                xn_1[2] = xn_1[1];
                xn_1[1] = xn_1[0];
                
                yn_1[2] = yn_1[1];
                yn_1[1] = yn_1[0];
                
                // Filtro Pasa Altas
                xn_2[0] = yn_1[0];
                
                yn_2[0] = (coeffPasaAltos_B[0]*xn_2[0]) + (coeffPasaAltos_B[1]*xn_2[1]) + (coeffPasaAltos_B[2]*xn_2[2]) - (coeffPasaAltos_A[1]*yn_2[1])-(coeffPasaAltos_A[2]*yn_2[2]);
                
                xn_2[2] = xn_2[1];
                xn_2[1] = xn_2[0];
                
                yn_2[2] = yn_2[1];
                yn_2[1] = yn_2[0];
                
                // channelL[i] = yn_2[0];
                // channelR[i] = yn_2[0];
                
                // =================================================================
            
                bufferCircular[contadorBufferCircular] = yn_2[0];
                
                contadorBufferCircular = contadorBufferCircular + 1;
                
                // Si el contador sobrepasa el tamaño del buffer
                // Se reinicia  
                if (contadorBufferCircular > tiempoDeBufferEnMuestras)
                {
                    contadorBufferCircular = 0;
                }
                // Durante cada buffer se ingresa el 25% de muestras nuevas
                // Luego se calcula el RMS segun la norma con el 75% de muestras
                // Viejas
                if (contadorBufferCircular == (tiempoDeBuffer75 - 1))
                {
                    RMS_Value = calculaRMS(bufferCircular, tiempoDeBufferEnMuestras);
                    RMS_Value = -0.691 + 10*log10(RMS_Value);
                }
                
                // =================================================================
                // Segmento de Compresion
                
                float valorObjetivo = 0;
                // Si la sonoridad esta por encima del umbral
                // El valor objetivo de la senal es el umbral
                if (RMS_Value >= umbral)
                    valorObjetivo = umbral;
                else
                    valorObjetivo = RMS_Value;
                // Si no, el valor objetivo es el mismo de sonoridad
                
                // El delta de compresion es que tanto hay que multiplicar
                // la senal para que alcance el nivel deseado
                float deltaCompresion = RMS_Value - valorObjetivo;
                
                if (i == 0)
                    valRef = deltaCompresion;
                
                float Yl_Actual;
                
                // Un compresor es un circuito RC, por lo que las constantes de tiempo
                // se pueden expresar como un filtro, de primer orden
                // Este es el filtrado
                if (valRef > Yl_Prev)
                {
                    Yl_Actual = tauAttackConstant * Yl_Prev + (1 - tauAttackConstant) * deltaCompresion;
                }
                else
                {
                    Yl_Actual = tauReleaseConstant * Yl_Prev + (1 - tauReleaseConstant) * deltaCompresion;
                }
                // Todo se esta calculando en dB, por lo que se debe pasar a decimal
                // nuevamente
                float gainControl = pow(10, (-Yl_Actual/20));
                Yl_Prev = Yl_Actual;
                
                // Se aplica el factor de ganancia a la senal
                
                channelL[i] = channelL[i] * gainControl;
                channelR[i] = channelR[i] * gainControl;
            }
        }
    }
}

float ItutvAudioProcessor::calculaRMS(float *bufferCircular, int tamBuffer)
{
    float RMS_Sum = 0;
    
    for (int i = 0; i < tamBuffer; i++)
    {
        RMS_Sum = RMS_Sum + pow(bufferCircular[i], 2);
    }
    
    return RMS_Sum = RMS_Sum/(float) tamBuffer;
}

//==============================================================================
bool ItutvAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ItutvAudioProcessor::createEditor()
{
    return new ItutvAudioProcessorEditor (*this);
}

//==============================================================================
void ItutvAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ItutvAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ItutvAudioProcessor();
}
