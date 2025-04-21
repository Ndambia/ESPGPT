#ifndef OPENAI_CLIENT_H
#define OPENAI_CLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "../lib/config.h"

class OpenAIClient {
private:
  const char* host = "api.openai.com";
  const int port = 443;
  const char* whisperEndpoint = "/v1/audio/transcriptions";
  
  // Simple LRU cache for responses
  struct CacheEntry {
    String prompt;
    String response;
    unsigned long timestamp;
  };
  
  static const int MAX_CACHE_SIZE = 5;
  CacheEntry cache[MAX_CACHE_SIZE];
  int cacheCount = 0;

public:
  OpenAIClient() {
    // Initialize cache
    for (int i = 0; i < MAX_CACHE_SIZE; i++) {
      cache[i] = {"", "", 0};
    }
  }

  // Check if a response is in the cache
  String getCachedResponse(String prompt) {
    for (int i = 0; i < cacheCount; i++) {
      if (cache[i].prompt == prompt) {
        // Update timestamp to mark as recently used
        cache[i].timestamp = millis();
        return cache[i].response;
      }
    }
    return "";
  }

  // Add a response to the cache
  void cacheResponse(String prompt, String response) {
    // If cache is full, find the least recently used entry
    if (cacheCount >= MAX_CACHE_SIZE) {
      int oldestIndex = 0;
      unsigned long oldestTime = cache[0].timestamp;
      
      for (int i = 1; i < MAX_CACHE_SIZE; i++) {
        if (cache[i].timestamp < oldestTime) {
          oldestTime = cache[i].timestamp;
          oldestIndex = i;
        }
      }
      
      // Replace the oldest entry
      cache[oldestIndex].prompt = prompt;
      cache[oldestIndex].response = response;
      cache[oldestIndex].timestamp = millis();
    } else {
      // Add to cache
      cache[cacheCount].prompt = prompt;
      cache[cacheCount].response = response;
      cache[cacheCount].timestamp = millis();
      cacheCount++;
    }
  }

  // Get a response from OpenAI, using cache if available
  String getResponse(String prompt, String systemPrompt = "You are a helpful assistant.") {
    // Check cache first
    String cachedResponse = getCachedResponse(prompt);
    if (cachedResponse.length() > 0) {
      Serial.println("Using cached response");
      return cachedResponse;
    }
    
    // Not in cache, query the API
    String response = queryAPI(prompt, systemPrompt);
    
    // Cache the response if valid
    if (response.length() > 0 && !response.startsWith("Error:")) {
      cacheResponse(prompt, response);
    }
    
    return response;
  }
  
  // Transcribe audio using Whisper API
  String transcribeAudio(uint8_t* audioData, size_t audioSize) {
    WiFiClientSecure client;
    client.setInsecure();  // Note: In production, use proper certificate validation
    
    Serial.println("Connecting to OpenAI Whisper API...");
    
    // Connection with timeout
    unsigned long connectionStart = millis();
    while (!client.connect(host, port)) {
      if (millis() - connectionStart > 10000) {
        return "Error: Connection timeout";
      }
      delay(100);
    }
    
    Serial.println("Connected to Whisper API");
    
    // Generate a boundary for multipart form data
    String boundary = "ESP32AudioBoundary";
    
    // Create multipart form data
    String header = "--" + boundary + "\r\n";
    header += "Content-Disposition: form-data; name=\"file\"; filename=\"audio.webm\"\r\n";
    header += "Content-Type: audio/webm\r\n\r\n";
    
    String footer = "\r\n--" + boundary + "\r\n";
    footer += "Content-Disposition: form-data; name=\"model\"\r\n\r\n";
    footer += "whisper-1\r\n";
    footer += "--" + boundary + "--\r\n";
    
    // Calculate content length
    size_t contentLength = header.length() + audioSize + footer.length();
    
    // Create HTTP request
    String request =
      "POST " + String(whisperEndpoint) + " HTTP/1.1\r\n" +
      "Host: " + String(host) + "\r\n" +
      "Authorization: Bearer " + OPENAI_API_KEY + "\r\n" +
      "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n" +
      "Content-Length: " + String(contentLength) + "\r\n\r\n";
    
    // Send request headers
    client.print(request);
    
    // Send multipart form data header
    client.print(header);
    
    // Send audio data in chunks
    const size_t chunkSize = 1024;
    for (size_t i = 0; i < audioSize; i += chunkSize) {
      size_t bytesToSend = min(chunkSize, audioSize - i);
      client.write(audioData + i, bytesToSend);
      yield(); // Allow ESP32 to handle background tasks
    }
    
    // Send multipart form data footer
    client.print(footer);
    
    // Read response with timeout
    String response = "";
    unsigned long timeout = millis();
    bool headerComplete = false;
    
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        
        // Skip headers until we find an empty line
        if (!headerComplete) {
          if (line == "\r") {
            headerComplete = true;
          }
          continue;
        }
        
        response += line;
        timeout = millis();
      }
      
      if (millis() - timeout > 15000) {
        Serial.println("Response timeout");
        break;
      }
      
      // Small delay to allow data to arrive
      delay(10);
    }
    
    // Parse JSON response
    int jsonStart = response.indexOf('{');
    if (jsonStart == -1) {
      return "Error: Invalid response format";
    }
    
    String jsonBody = response.substring(jsonStart);
    
    JsonDocument resDoc;
    DeserializationError error = deserializeJson(resDoc, jsonBody);
    
    if (error) {
      return "Error: JSON parsing failed - " + String(error.c_str());
    }
    
    // Extract the transcription text
    if (resDoc["error"].is<JsonObject>()) {
      return "Error: " + String(resDoc["error"]["message"].as<const char*>());
    }
    
    String result = resDoc["text"] | "No transcription";
    result.trim();
    
    return result;
  }

private:
  // Make the actual API call
  String queryAPI(String prompt, String systemPrompt) {
    WiFiClientSecure client;
    client.setInsecure();  // Note: In production, use proper certificate validation
    
    Serial.println("Connecting to OpenAI API...");
    
    // Connection with timeout
    unsigned long connectionStart = millis();
    while (!client.connect(host, port)) {
      if (millis() - connectionStart > 10000) {
        return "Error: Connection timeout";
      }
      delay(100);
    }
    
    Serial.println("Connected to API server");
    
    // Create JSON request
    JsonDocument doc;
    doc["model"] = "gpt-3.5-turbo";
    JsonArray messages = doc["messages"].to<JsonArray>();
    
    JsonObject sys = messages.add<JsonObject>();
    sys["role"] = "system";
    sys["content"] = systemPrompt;
    
    JsonObject user = messages.add<JsonObject>();
    user["role"] = "user";
    user["content"] = prompt;
    
    String body;
    serializeJson(doc, body);
    
    // Create HTTP request
    String request =
      "POST /v1/chat/completions HTTP/1.1\r\n" +
      String("Host: ") + host + "\r\n" +
      "Authorization: Bearer " + OPENAI_API_KEY + "\r\n" +
      "Content-Type: application/json\r\n" +
      "Content-Length: " + body.length() + "\r\n\r\n" +
      body;
    
    // Send request
    client.print(request);
    
    // Read response with timeout
    String response = "";
    unsigned long timeout = millis();
    bool headerComplete = false;
    
    while (client.connected() || client.available()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        
        // Skip headers until we find an empty line
        if (!headerComplete) {
          if (line == "\r") {
            headerComplete = true;
          }
          continue;
        }
        
        response += line;
        timeout = millis();
      }
      
      if (millis() - timeout > 15000) {
        Serial.println("Response timeout");
        break;
      }
      
      // Small delay to allow data to arrive
      delay(10);
    }
    
    // Parse JSON response
    int jsonStart = response.indexOf('{');
    if (jsonStart == -1) {
      return "Error: Invalid response format";
    }
    
    String jsonBody = response.substring(jsonStart);
    
    JsonDocument resDoc;
    DeserializationError error = deserializeJson(resDoc, jsonBody);
    
    if (error) {
      return "Error: JSON parsing failed - " + String(error.c_str());
    }
    
    // Extract the response text
    if (resDoc["error"].is<JsonObject>()) {
      return "Error: " + String(resDoc["error"]["message"].as<const char*>());
    }
    
    String result = resDoc["choices"][0]["message"]["content"] | "No response";
    result.trim();
    
    return result;
  }
};

#endif