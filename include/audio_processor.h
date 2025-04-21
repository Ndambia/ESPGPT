#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <Arduino.h>
#include "../lib/config.h"

class AudioProcessor {
private:
    // Audio buffer
    int16_t audioBuffer[AUDIO_BUFFER_SIZE];
    
    // Wake word detection variables
    bool wakeWordDetected;
    char wakeWord[32];
    
    // Audio processing task handle
    TaskHandle_t audioTaskHandle;
    
    // Audio recording state
    bool isRecording;
    bool isProcessing;
    
    // Callback function type
    typedef void (*WakeWordCallback)(void);
    WakeWordCallback wakeWordCallback;
    
    // Audio processing task
    static void audioProcessingTask(void* parameter) {
        AudioProcessor* processor = (AudioProcessor*)parameter;
        
        while (true) {
            if (processor->isProcessing) {
                // Read audio samples from ADC
                processor->readAudioSamples();
                
                // Check for wake word
                if (processor->detectWakeWord()) {
                    processor->wakeWordDetected = true;
                    
                    // Call the callback if registered
                    if (processor->wakeWordCallback != NULL) {
                        processor->wakeWordCallback();
                    }
                    
                    // Pause processing for a moment after detection
                    vTaskDelay(pdMS_TO_TICKS(1000));
                    processor->wakeWordDetected = false;
                }
            }
            
            // Small delay to prevent task starvation
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    
    // Read audio samples from ADC
    void readAudioSamples() {
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            // Read from ADC and convert to signed 16-bit
            int adcValue = analogRead(MIC_PIN);
            
            // Convert ADC value (0-4095) to signed 16-bit (-32768 to 32767)
            // Assuming the signal is centered around 2048
            audioBuffer[i] = (adcValue - 2048) * 16;
            
            // Small delay between samples to achieve desired sample rate
            delayMicroseconds(1000000 / SAMPLE_RATE);
        }
    }
    
    // Simple wake word detection algorithm
    bool detectWakeWord() {
        // Calculate audio energy
        float energy = 0;
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
            energy += abs(audioBuffer[i]);
        }
        energy /= AUDIO_BUFFER_SIZE;
        
        // Simple threshold-based detection
        if (energy > WAKE_WORD_THRESHOLD) {
            Serial.print("Audio energy detected: ");
            Serial.println(energy);
            return true;
        }
        
        return false;
    }

public:
    AudioProcessor() : 
        wakeWordDetected(false),
        audioTaskHandle(NULL),
        isRecording(false),
        isProcessing(false),
        wakeWordCallback(NULL)
    {
        // Initialize wake word
        strncpy(wakeWord, DEFAULT_WAKE_WORD, sizeof(wakeWord) - 1);
        wakeWord[sizeof(wakeWord) - 1] = '\0';
        
        // Configure ADC
        analogReadResolution(12);
        analogSetAttenuation(ADC_11db);
    }
    
    // Initialize the audio processor
    void begin() {
        if (AUDIO_ENABLED) {
            // Create audio processing task
            xTaskCreatePinnedToCore(
                audioProcessingTask,
                "AudioProcessing",
                4096,
                this,
                1,
                &audioTaskHandle,
                0
            );
            
            Serial.println("Audio processor initialized");
            Serial.print("Microphone connected to pin: ");
            Serial.println(MIC_PIN);
            Serial.print("Wake word: '");
            Serial.print(wakeWord);
            Serial.println("'");
            Serial.print("Wake word threshold: ");
            Serial.println(WAKE_WORD_THRESHOLD);
        } else {
            Serial.println("Audio processing disabled in config");
        }
    }
    
    // Start audio processing
    void startProcessing() {
        if (AUDIO_ENABLED && WAKE_WORD_ENABLED) {
            isProcessing = true;
            Serial.println("Audio processing started - listening for wake word");
        } else {
            Serial.println("Audio processing or wake word detection is disabled in config");
        }
    }
    
    // Stop audio processing
    void stopProcessing() {
        isProcessing = false;
        Serial.println("Audio processing stopped");
    }
    
    // Set wake word
    void setWakeWord(const char* word) {
        strncpy(wakeWord, word, sizeof(wakeWord) - 1);
        wakeWord[sizeof(wakeWord) - 1] = '\0';
        Serial.print("Wake word set to: ");
        Serial.println(wakeWord);
    }
    
    // Register wake word detection callback
    void onWakeWordDetected(WakeWordCallback callback) {
        wakeWordCallback = callback;
    }
    
    // Check if wake word was detected
    bool isWakeWordDetected() {
        return wakeWordDetected;
    }
    
    // Reset wake word detection state
    void resetWakeWordDetection() {
        wakeWordDetected = false;
    }
    
    // Start recording audio
    void startRecording() {
        isRecording = true;
        Serial.println("Audio recording started");
    }
    
    // Stop recording audio
    void stopRecording() {
        isRecording = false;
        Serial.println("Audio recording stopped");
    }
    
    // Get audio buffer
    int16_t* getAudioBuffer() {
        return audioBuffer;
    }
    
    // Get audio buffer size
    size_t getAudioBufferSize() {
        return AUDIO_BUFFER_SIZE;
    }
};

#endif