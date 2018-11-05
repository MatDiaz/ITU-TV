/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/**
*/
class ItutvAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    ItutvAudioProcessor();
    ~ItutvAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    // Esta será una función para calcular el RMS del buffer circular
    float calculaRMS(float* bufferCircular, int tamBuffer);
    
    bool procesoActivo; // Esta es la variable del boton prendido/apagado

private:
    int contadorBufferCircular;
    // Se crea un contador para saber la posición del buffer
    ScopedPointer<float> bufferCircular;
    // Esta es la variable del buffer circular
    int tiempoDeBufferEnMuestras;
    int tiempoDeBuffer75;
    // Esta variable sera el tiempo de buffer en muestras
    
    // Se crean vectores para almacenar los coeficientes de los filtros
    // Cada filtro tiene coeficientes de numerador(B) y coeficientes de
    // denominador(B)
    float coeffPeak_A[3], coeffPasaAltos_A[3];
    float coeffPeak_B[3], coeffPasaAltos_B[3];
    
    // Se crean vectores para guardar las muestras del filtro
    // x(n), x(n-1), x(n-2)
    // y(n), y(n-1), y(n-2)
    float xn_1[3], yn_1[3];
    float xn_2[3], yn_2[3];
    
    // Valor del RMS segun la norma
    float RMS_Value;
    
    // Constantes de Tiempo para compresion
    float umbral;
    float factorNormalizacion;
    
    float attackTime, tauAttackConstant;
    float releaseTime, tauReleaseConstant;
    float valRef, Yl_Prev;
    //====================================
    
    int contTonoPuro;
    double FrecMuestreo;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItutvAudioProcessor)
};
