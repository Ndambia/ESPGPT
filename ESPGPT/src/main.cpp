#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

const char* ssid = "Trojan";
const char* password = "Brian254";

#define OPENAI_API_KEY "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"
const char* openai_host = "api.openai.com";
const int httpsPort = 443;

const String knowledgeBase[] = {
  "AI stands for Artificial Intelligence.",
  "ESP32 is a microcontroller with WiFi and Bluetooth.",
  "Arduino was created by developers in Italy.",
};
const int kbSize = sizeof(knowledgeBase) / sizeof(knowledgeBase[0]);

// --- Match functions ---
int keywordMatch(String query, String text) {
  int score = 0;
  query.toLowerCase(); text.toLowerCase();

  int start = 0;
  while (start < query.length()) {
    int end = query.indexOf(' ', start);
    if (end == -1) end = query.length();
    String word = query.substring(start, end);
    if (text.indexOf(word) >= 0) score++;
    start = end + 1;
  }
  return score;
}

String getBestMatch(String question) {
  int bestScore = -1, bestIndex = 0;
  for (int i = 0; i < kbSize; i++) {
    int score = keywordMatch(question, knowledgeBase[i]);
    if (score > bestScore) {
      bestScore = score;
      bestIndex = i;
    }
  }
  return knowledgeBase[bestIndex];
}

// --- OpenAI API call ---
String askOpenAI(String prompt) {
  WiFiClientSecure client;
  client.setInsecure();  // Skip certificate verification

  if (!client.connect("api.openai.com", 443)) {
    Serial.println("❌ Connection to OpenAI failed!");
    return "Connection failed";
  }

  JsonDocument doc;
  doc["model"] = "gpt-3.5-turbo";

  // Proper way to add messages in ArduinoJson v7+
  JsonArray messages = doc["messages"].to<JsonArray>();
  JsonObject systemMsg = messages.add<JsonObject>();
  systemMsg["role"] = "system";
  systemMsg["content"] = "You are a helpful assistant.";

  JsonObject userMsg = messages.add<JsonObject>();
  userMsg["role"] = "user";
  userMsg["content"] = prompt;

  String body;
  serializeJson(doc, body);

  String request =
    "POST /v1/chat/completions HTTP/1.1\r\n" +
    String("Host: api.openai.com\r\n") +
    "Authorization: Bearer " + OPENAI_API_KEY + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Content-Length: " + body.length() + "\r\n\r\n" +
    body;

  client.print(request);

  // Read full response
  String response = "";
  unsigned long timeout = millis();
  while (client.connected() || client.available()) {
    if (client.available()) {
      response += client.readStringUntil('\n');
      timeout = millis();  // reset timeout on activity
    }
    if (millis() - timeout > 10000) { // 10 seconds timeout
      Serial.println("❌ Timeout while waiting for response.");
      break;
    }
  }
  

  Serial.println("\n[RAW RESPONSE]");
  Serial.println(response);

  // Extract JSON body (skip headers)
 
  int jsonStart = response.indexOf('{');
  if (jsonStart == -1) {
    Serial.println("❌ No JSON object found in response");
    return "Error";
  }
  String jsonBody = response.substring(jsonStart);
  
  // Debug print
  Serial.println("✅ JSON Body:");
  Serial.println(jsonBody);
  
  // Parse JSON
  JsonDocument responseDoc;
  DeserializationError error = deserializeJson(responseDoc, jsonBody);
  if (error) {
    Serial.print("❌ JSON parse failed: ");
    Serial.println(error.c_str());
    return "Error";
  }
  
  String result = responseDoc["choices"][0]["message"]["content"] | "No content";
  result.trim();  // Clean any leading/trailing whitespace
  return result;
}

// --- Main logic ---
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected!");


  WiFiClientSecure testClient;
testClient.setInsecure();

if (testClient.connect("api.openai.com", 443)) {
  Serial.println("✅ HTTPS Test Connection OK!");
} else {
  Serial.println("❌ HTTPS Test Connection Failed!");
}

}

void loop() {
  String question = "What is ESP32?";
  String context = getBestMatch(question);
  String prompt = context + "\n\nQ: " + question + "\nA:";

  Serial.println("Asking OpenAI:");
  Serial.println(prompt);

  String response = askOpenAI(prompt);

  Serial.println("\nOpenAI Response:");
  Serial.println(response);

  delay(30000);  // avoid spamming
}
