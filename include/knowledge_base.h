#ifndef KNOWLEDGE_BASE_H
#define KNOWLEDGE_BASE_H

#include <Arduino.h>
#include <vector>

// Structure for knowledge entries
struct KnowledgeEntry {
  String keywords;
  String content;
  float importance;
};

class KnowledgeBase {
private:
  std::vector<KnowledgeEntry> entries;

public:
  KnowledgeBase() {
    // Initialize with some default entries
    addEntry("AI artificial intelligence", "AI stands for Artificial Intelligence.", 1.0);
    addEntry("ESP32 microcontroller wifi bluetooth", "ESP32 is a microcontroller with WiFi and Bluetooth capabilities.", 1.0);
    addEntry("Arduino Italy developers", "Arduino was created by developers in Italy.", 1.0);
  }

  void addEntry(String keywords, String content, float importance = 1.0) {
    KnowledgeEntry entry = {keywords, content, importance};
    entries.push_back(entry);
  }

  int getSize() {
    return entries.size();
  }

  String getContent(int index) {
    if (index >= 0 && index < entries.size()) {
      return entries[index].content;
    }
    return "";
  }

  // Find the best matching entry for a query
  String getBestMatch(String query) {
    int bestScore = -1;
    int bestIndex = 0;
    
    query.toLowerCase();
    
    for (int i = 0; i < entries.size(); i++) {
      int score = keywordMatch(query, entries[i].keywords) * entries[i].importance;
      if (score > bestScore) {
        bestScore = score;
        bestIndex = i;
      }
    }
    
    return entries[bestIndex].content;
  }

private:
  // Score a query against keywords
  int keywordMatch(String query, String keywords) {
    int score = 0;
    keywords.toLowerCase();
    
    int start = 0;
    while (start < query.length()) {
      int end = query.indexOf(' ', start);
      if (end == -1) end = query.length();
      
      String word = query.substring(start, end);
      if (keywords.indexOf(word) >= 0) score++;
      
      start = end + 1;
    }
    
    return score;
  }
};

#endif