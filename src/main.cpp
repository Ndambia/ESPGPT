#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "../lib/config.h"
#include "../include/knowledge_base.h"
#include "../include/openai_client.h"
#include "../include/web_server.h"

// Simple audio processing definitions
#define MIC_PIN 34
#define AUDIO_BUFFER_SIZE 512
#define SAMPLE_RATE 16000
#define WAKE_WORD_THRESHOLD 2000

// Audio processing variables
bool wakeWordDetected = false;
TaskHandle_t audioTaskHandle = NULL;

// Create instances of our classes
KnowledgeBase knowledgeBase;
OpenAIClient openAI;
AIWebServer webServer(WEB_SERVER_PORT, knowledgeBase, openAI);

// Flag to indicate if wake word was detected
bool wakeWordActivated = false;

// Audio processing task
void audioProcessingTask(void* parameter) {
  int16_t audioBuffer[AUDIO_BUFFER_SIZE];
  
  while (true) {
    // Read audio samples from ADC
    float energy = 0;
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
      // Read from ADC
      int adcValue = analogRead(MIC_PIN);
      
      // Convert to signed 16-bit
      audioBuffer[i] = (adcValue - 2048) * 16;
      
      // Accumulate energy
      energy += abs(audioBuffer[i]);
      
      // Delay for sample rate
      delayMicroseconds(1000000 / SAMPLE_RATE);
    }
    
    // Calculate average energy
    energy /= AUDIO_BUFFER_SIZE;
    
    // Check for wake word (simple energy threshold)
    if (energy > WAKE_WORD_THRESHOLD) {
      Serial.print("Audio energy detected: ");
      Serial.println(energy);
      wakeWordActivated = true;
    }
    
    // Small delay
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setupWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  // Wait for connection with timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    // Timeout after 30 seconds
    if (millis() - startTime > 30000) {
      Serial.println("\nWiFi connection timeout!");
      ESP.restart();
    }
  }
  
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}


void setupOTA() {
  // Set hostname based on MAC address for easy identification
  String hostname = "ESP32-AI-";
  hostname += String((uint32_t)ESP.getEfuseMac(), HEX);
  
  ArduinoOTA.setHostname(hostname.c_str());
  
  // IMPORTANT: Change this password in production environments!
  ArduinoOTA.setPassword("admin");
  
  // Optional: Change port (default is 3232)
  // ArduinoOTA.setPort(8266);
  
  // Event handlers for OTA process
  ArduinoOTA.onStart([]() {
    Serial.println("Starting OTA update...");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA update complete!");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed - Check password");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed - Insufficient space?");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed - Check network");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed - Check connection stability");
    else if (error == OTA_END_ERROR) Serial.println("End Failed - Verify integrity");
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA ready - Use 'pio run -t upload --upload-port " + WiFi.localIP().toString() + "' for updates");
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n\n--- ESP32 AI Assistant ---");
  
  // Connect to WiFi
  setupWiFi();
  
  // Setup OTA updates
  setupOTA();
  
  // Add some additional knowledge entries
  knowledgeBase.addEntry("ESP32 features capabilities specs",
    "The ESP32 is a powerful microcontroller with dual-core processor, WiFi, Bluetooth, and extensive GPIO capabilities.", 1.2);
  
  knowledgeBase.addEntry("PlatformIO IDE development environment",
    "PlatformIO is a cross-platform IDE and unified debugger that supports many development boards including ESP32.", 1.0);
  
  knowledgeBase.addEntry("Arduino framework programming",
    "The Arduino framework provides a simple and accessible way to program microcontrollers with C/C++.", 1.0);
  
  // Add wake word related knowledge
  knowledgeBase.addEntry("wake word voice activation",
    "The system supports wake word detection to activate voice commands without pressing buttons.", 1.2);
  
  // Initialize ADC for microphone
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  
  // Create audio processing task
  xTaskCreatePinnedToCore(
    audioProcessingTask,
    "AudioProcessing",
    4096,
    NULL,
    1,
    &audioTaskHandle,
    0
  );
  
  Serial.println("Audio processing initialized");
  Serial.println("Microphone connected to pin: " + String(MIC_PIN));
  Serial.println("Wake word detection enabled with threshold: " + String(WAKE_WORD_THRESHOLD));
  
  // Start the web server
  webServer.begin();
  
  Serial.println("System ready!");
  Serial.println("Access the AI assistant at http://" + WiFi.localIP().toString() + ":" + String(WEB_SERVER_PORT));
  Serial.println("Or say 'Hey ESP' to activate voice commands");
  
  // Print OTA update instructions
  Serial.println("\n--- OTA Update Instructions ---");
  Serial.println("For wireless updates, use:");
  Serial.println("pio run -t upload --upload-port " + WiFi.localIP().toString());
  Serial.println("Password: admin");
  Serial.println("-------------------------------");
}

void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();
  
  // Handle web server clients
  webServer.handleClient();
  
  // Handle wake word activation
  if (wakeWordActivated) {
    // Play a sound or flash an LED to indicate activation
    Serial.println("Processing voice command...");
    
    // In a real implementation, you would:
    // 1. Record audio for a few seconds
    // 2. Send it to the OpenAI API for transcription
    // 3. Process the transcribed command
    // 4. Generate and speak a response
    
    // For now, we'll just simulate this with a delay
    delay(3000);
    
    // Reset the wake word flag
    wakeWordActivated = false;
    
    Serial.println("Voice command processing complete");
  }
  
  // Monitor WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    WiFi.reconnect();
    
    // Wait for reconnection with timeout
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      
      // Timeout after 30 seconds
      if (millis() - startTime > 30000) {
        Serial.println("\nWiFi reconnection timeout! Restarting...");
        ESP.restart();
      }
    }
    
    Serial.println("\nWiFi reconnected!");
  }
  
  // Small delay to prevent watchdog timer issues
  delay(10);
}


/**
 * Setup Over-The-Air (OTA) updates
 *
 * How to use OTA:
 * 1. First upload via USB using: pio run -t upload
 * 2. Find ESP32's IP address in serial monitor
 * 3. For subsequent updates, use: pio run -t upload --upload-port [ESP-IP-ADDRESS]
 *
 * Alternative method (using platformio.ini):
 * Add these lines to platformio.ini:
 *   upload_protocol = espota
 *   upload_port = [ESP-IP-ADDRESS]
 *   upload_flags = --auth=admin
 *
 * Security considerations:
 * - Change the default password in production
 * - Only perform OTA updates on secure networks
 * - Keep a backup of working firmware
 */