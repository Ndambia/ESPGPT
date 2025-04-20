#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include "knowledge_base.h"
#include "openai_client.h"

class AIWebServer {
private:
  WebServer server;
  KnowledgeBase& kb;
  OpenAIClient& ai;
  
  // HTML templates
  const char* mainPageTemplate = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 AI Assistant</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <!-- Prism CSS for code highlighting -->
  <style>
    /* Modern Color Scheme */
    :root {
      --primary-color: #6200ee;
      --primary-dark: #3700b3;
      --primary-light: #bb86fc;
      --secondary-color: #03dac6;
      --secondary-dark: #018786;
      --background: #121212;
      --surface: #1e1e1e;
      --error: #cf6679;
      --on-primary: #ffffff;
      --on-secondary: #000000;
      --on-background: #ffffff;
      --on-surface: #ffffff;
      --on-error: #000000;
      --code-background: #2d2d2d;
      --code-foreground: #f8f8f2;
      --code-comment: #6272a4;
      --code-keyword: #ff79c6;
      --code-function: #50fa7b;
      --code-string: #f1fa8c;
      --code-number: #bd93f9;
      --code-operator: #ff79c6;
      --code-class: #8be9fd;
      --code-variable: #f8f8f2;
    }
    
    /* Base Styles */
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: var(--background);
      margin: 0;
      padding: 20px;
      color: var(--on-background);
      line-height: 1.6;
    }
    
    .container {
      max-width: 900px;
      margin: 0 auto;
      background: var(--surface);
      border-radius: 12px;
      box-shadow: 0 4px 20px rgba(0,0,0,0.3);
      padding: 24px;
      overflow: hidden;
    }
    
    h1 {
      color: var(--primary-light);
      text-align: center;
      margin-bottom: 24px;
      font-weight: 600;
      letter-spacing: 0.5px;
    }
    
    /* Chat Container */
    .chat-container {
      border: 1px solid rgba(255,255,255,0.1);
      border-radius: 12px;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      height: 70vh;
      background-color: rgba(30,30,30,0.7);
    }
    
    #chat-history {
      flex-grow: 1;
      overflow-y: auto;
      padding: 20px;
      background-color: var(--surface);
      scrollbar-width: thin;
      scrollbar-color: var(--primary-color) var(--surface);
    }
    
    #chat-history::-webkit-scrollbar {
      width: 8px;
    }
    
    #chat-history::-webkit-scrollbar-track {
      background: var(--surface);
    }
    
    #chat-history::-webkit-scrollbar-thumb {
      background-color: var(--primary-color);
      border-radius: 4px;
    }
    
    /* Input Area */
    .input-area {
      display: flex;
      padding: 16px;
      background-color: rgba(40,40,40,0.9);
      border-top: 1px solid rgba(255,255,255,0.1);
    }
    
    #question {
      flex-grow: 1;
      padding: 12px 16px;
      border: 1px solid rgba(255,255,255,0.2);
      border-radius: 8px;
      font-size: 16px;
      background-color: rgba(30,30,30,0.8);
      color: var(--on-background);
      transition: all 0.3s ease;
    }
    
    #question:focus {
      outline: none;
      border-color: var(--primary-light);
      box-shadow: 0 0 0 2px rgba(187,134,252,0.3);
    }
    
    #send-btn {
      background-color: var(--primary-color);
      color: var(--on-primary);
      border: none;
      border-radius: 8px;
      padding: 12px 24px;
      margin-left: 12px;
      cursor: pointer;
      font-size: 16px;
      font-weight: 500;
      transition: all 0.2s ease;
    }
    
    #send-btn:hover {
      background-color: var(--primary-dark);
      transform: translateY(-2px);
      box-shadow: 0 4px 8px rgba(0,0,0,0.2);
    }
    
    #send-btn:disabled {
      background-color: rgba(98, 0, 238, 0.3);
      cursor: not-allowed;
      transform: none;
      box-shadow: none;
    }
    
    /* Chat Messages */
    .chat-entry {
      margin-bottom: 20px;
      display: flex;
      animation: fadeIn 0.3s ease;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(10px); }
      to { opacity: 1; transform: translateY(0); }
    }
    
    .chat-entry.user {
      justify-content: flex-end;
    }
    
    .chat-entry.assistant {
      justify-content: flex-start;
    }
    
    .chat-entry.system {
      justify-content: center;
    }
    
    .chat-bubble {
      max-width: 85%;
      padding: 12px 18px;
      border-radius: 18px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.15);
      word-wrap: break-word;
      line-height: 1.5;
    }
    
    .user .chat-bubble {
      background-color: var(--primary-color);
      color: var(--on-primary);
      border-bottom-right-radius: 4px;
    }
    
    .assistant .chat-bubble {
      background-color: rgba(60,60,60,0.9);
      color: var(--on-surface);
      border-bottom-left-radius: 4px;
    }
    
    .system .chat-bubble {
      background-color: var(--error);
      color: var(--on-error);
      font-size: 14px;
      padding: 8px 12px;
      border-radius: 8px;
    }
    
    /* Code Highlighting */
    pre {
      background-color: var(--code-background);
      border-radius: 8px;
      padding: 12px;
      overflow-x: auto;
      margin: 10px 0;
    }
    
    code {
      font-family: 'Fira Code', Consolas, Monaco, 'Andale Mono', monospace;
      color: var(--code-foreground);
      font-size: 14px;
      line-height: 1.5;
      tab-size: 2;
    }
    
    .code-header {
      background-color: rgba(0,0,0,0.3);
      padding: 6px 12px;
      border-top-left-radius: 8px;
      border-top-right-radius: 8px;
      font-size: 12px;
      color: #ccc;
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-top: 10px;
      margin-bottom: -10px;
    }
    
    .language-javascript { color: var(--code-foreground); }
    .token.comment { color: var(--code-comment); }
    .token.keyword { color: var(--code-keyword); }
    .token.function { color: var(--code-function); }
    .token.string { color: var(--code-string); }
    .token.number { color: var(--code-number); }
    .token.operator { color: var(--code-operator); }
    .token.class-name { color: var(--code-class); }
    .token.variable { color: var(--code-variable); }
    
    /* Loading Indicator */
    #loading {
      text-align: center;
      padding: 16px;
      color: var(--primary-light);
      font-weight: 500;
    }
    
    .loading-dots:after {
      content: '.';
      animation: dots 1.5s steps(5, end) infinite;
    }
    
    @keyframes dots {
      0%, 20% { content: '.'; }
      40% { content: '..'; }
      60% { content: '...'; }
      80%, 100% { content: ''; }
    }
    
    .hidden {
      display: none;
    }
    
    /* Footer */
    .footer {
      text-align: center;
      margin-top: 24px;
      font-size: 14px;
      color: rgba(255,255,255,0.5);
    }
    
    .footer a {
      color: var(--primary-light);
      text-decoration: none;
    }
    
    .footer a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 AI Assistant</h1>
    <div class="chat-container">
      <div id="chat-history"></div>
      <div id="loading" class="hidden">
        Thinking<span class="loading-dots"></span>
      </div>
      <div class="input-area">
        <input id="question" type="text" placeholder="Ask me anything..." />
        <button id="send-btn" onclick="ask()">Send</button>
      </div>
    </div>
    <div class="footer">
      Powered by <a href="#" onclick="showInfo(); return false;">ESP32 & OpenAI</a>
    </div>
  </div>

  <script>
    // Keep chat history
    let chatHistory = [];
    
    // Language detection patterns
    const codePatterns = {
      javascript: /```(javascript|js)\n([\s\S]*?)```/g,
      python: /```(python|py)\n([\s\S]*?)```/g,
      cpp: /```(cpp|c\+\+|c)\n([\s\S]*?)```/g,
      html: /```(html|xml)\n([\s\S]*?)```/g,
      css: /```(css)\n([\s\S]*?)```/g,
      json: /```(json)\n([\s\S]*?)```/g,
      bash: /```(bash|sh|shell)\n([\s\S]*?)```/g,
      plaintext: /```(plaintext|text)?\n([\s\S]*?)```/g
    };
    
    // Add system message on load
    window.onload = function() {
      addToHistory('system', 'Welcome! Ask me anything about programming, ESP32, or AI. I can show code examples with syntax highlighting.');
    };
    
    async function ask() {
      const questionInput = document.getElementById('question');
      const question = questionInput.value.trim();
      if (!question) return;
      
      // Show loading indicator
      document.getElementById('loading').classList.remove('hidden');
      document.getElementById('send-btn').disabled = true;
      
      // Add question to history
      addToHistory('user', question);
      
      try {
        const res = await fetch("/ask?q=" + encodeURIComponent(question));
        if (!res.ok) {
          throw new Error(`HTTP error! status: ${res.status}`);
        }
        const text = await res.text();
        addToHistory('assistant', text);
      } catch (err) {
        addToHistory('system', "Error: " + err.message);
      } finally {
        document.getElementById('loading').classList.add('hidden');
        document.getElementById('send-btn').disabled = false;
        questionInput.value = '';
        questionInput.focus();
      }
    }
    
    function formatCodeBlocks(text) {
      // Process each language pattern
      let formattedText = text;
      
      // First replace all code blocks with placeholders
      let codeBlocks = [];
      let codeBlockIndex = 0;
      
      // Process each language
      for (const [language, pattern] of Object.entries(codePatterns)) {
        formattedText = formattedText.replace(pattern, (match, lang, code) => {
          const placeholder = `__CODE_BLOCK_${codeBlockIndex}__`;
          codeBlocks.push({
            placeholder,
            language: lang.trim(),
            code: code.trim()
          });
          codeBlockIndex++;
          return placeholder;
        });
      }
      
      // Replace placeholders with formatted code
      for (const block of codeBlocks) {
        const languageDisplay = block.language === 'plaintext' ? '' : block.language;
        const formattedCode = `
          <div class="code-block">
            <div class="code-header">
              <span>${languageDisplay}</span>
            </div>
            <pre><code class="language-${block.language}">${escapeHtml(block.code)}</code></pre>
          </div>
        `;
        formattedText = formattedText.replace(block.placeholder, formattedCode);
      }
      
      // Apply syntax highlighting to inline code
      formattedText = formattedText.replace(/`([^`]+)`/g, '<code class="inline-code">$1</code>');
      
      return formattedText;
    }
    
    function escapeHtml(text) {
      return text
        .replace(/&/g, "&amp;")
        .replace(/</g, "&lt;")
        .replace(/>/g, "&gt;")
        .replace(/"/g, "&quot;")
        .replace(/'/g, "&#039;");
    }
    
    function highlightSyntax(element) {
      // Apply syntax highlighting to code elements
      const codeElements = element.querySelectorAll('code');
      
      codeElements.forEach(codeEl => {
        const language = codeEl.className.replace('language-', '');
        const code = codeEl.textContent;
        
        // Simple syntax highlighting for common elements
        let highlighted = code;
        
        // Keywords
        highlighted = highlighted.replace(
          /\b(function|return|if|else|for|while|var|let|const|class|import|export|from|async|await|try|catch|throw|new|this)\b/g,
          '<span class="token keyword">$1</span>'
        );
        
        // Strings
        highlighted = highlighted.replace(
          /(".*?"|'.*?'|`.*?`)/g,
          '<span class="token string">$1</span>'
        );
        
        // Numbers
        highlighted = highlighted.replace(
          /\b(\d+(\.\d+)?)\b/g,
          '<span class="token number">$1</span>'
        );
        
        // Comments
        highlighted = highlighted.replace(
          /(\/\/.*|\/\*[\s\S]*?\*\/)/g,
          '<span class="token comment">$1</span>'
        );
        
        // Functions
        highlighted = highlighted.replace(
          /\b([a-zA-Z_$][a-zA-Z0-9_$]*)\s*\(/g,
          '<span class="token function">$1</span>('
        );
        
        codeEl.innerHTML = highlighted;
      });
    }
    
    function addToHistory(role, text) {
      const historyDiv = document.getElementById('chat-history');
      const entry = document.createElement('div');
      entry.className = `chat-entry ${role}`;
      
      // Format code blocks if this is an assistant message
      let processedText = text;
      if (role === 'assistant') {
        processedText = formatCodeBlocks(text);
      }
      
      entry.innerHTML = `<div class="chat-bubble">${processedText}</div>`;
      historyDiv.appendChild(entry);
      
      // Apply syntax highlighting
      if (role === 'assistant') {
        highlightSyntax(entry);
      }
      
      // Scroll to bottom
      historyDiv.scrollTop = historyDiv.scrollHeight;
      
      // Store in history array
      chatHistory.push({role, text});
    }
    
    // Handle Enter key press
    document.getElementById('question').addEventListener('keypress', function(e) {
      if (e.key === 'Enter') {
        ask();
      }
    });
    
    function showInfo() {
      addToHistory('system', 'ESP32 AI Assistant v2.0 - Enhanced with code highlighting and a modern UI. Ask me about programming, ESP32 features, or AI topics!');
    }
  </script>
</body>
</html>
)rawliteral";

public:
  AIWebServer(int port, KnowledgeBase& knowledgeBase, OpenAIClient& aiClient) 
    : server(port), kb(knowledgeBase), ai(aiClient) {}
  
  void begin() {
    // Set up routes
    server.on("/", HTTP_GET, [this]() {
      handleRoot();
    });
    
    server.on("/ask", HTTP_GET, [this]() {
      handleAsk();
    });
    
    // Start server
    server.begin();
    Serial.println("Web server started on port 80");
  }
  
  void handleClient() {
    server.handleClient();
  }

private:
  void handleRoot() {
    server.send(200, "text/html", mainPageTemplate);
  }
  
  void handleAsk() {
    if (!server.hasArg("q")) {
      server.send(400, "text/plain", "Missing question parameter");
      return;
    }
    
    String question = server.arg("q");
    Serial.println("Question: " + question);
    
    // Get context from knowledge base
    String context = kb.getBestMatch(question);
    Serial.println("Context: " + context);
    
    // Create prompt with context and instructions for code formatting
    String prompt = "Context information: " + context + "\n\n"
                   "Question: " + question + "\n\n"
                   "When providing code examples, please format them using markdown code blocks with language specifiers, like:\n"
                   "```javascript\n// Your JavaScript code here\n```\n"
                   "```cpp\n// Your C++ code here\n```\n"
                   "For inline code, use backticks like `this`.\n\n"
                   "Answer:";
    
    // Get response from OpenAI
    String answer = ai.getResponse(prompt);
    Serial.println("Answer: " + answer);
    
    // Send response
    server.send(200, "text/plain", answer);
  }
};

#endif