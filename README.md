# ESP32 AI Assistant with Wake Word Detection

This project implements an AI assistant on the ESP32 microcontroller with the following features:

- Web interface for text-based interaction
- Voice input through the web interface
- Wake word detection using an analog microphone
- OpenAI API integration for natural language processing
- Knowledge base for storing and retrieving information
- OTA (Over-The-Air) updates

## Hardware Requirements

- ESP32 development board (e.g., ESP32-DOIT-DEVKIT-V1)
- Analog microphone module (e.g., MAX9814 or similar)
- Optional: Speaker for audio output (not implemented in this version)

## Microphone Connection

Connect your analog microphone to the ESP32 as follows:

- Microphone OUT pin → ESP32 GPIO34 (ADC1_CH6)
- Microphone VCC → ESP32 3.3V
- Microphone GND → ESP32 GND

Note: The default microphone pin is GPIO34, but you can change this in the `config.h` file.

## Wake Word Detection

The system listens for a wake word ("Hey ESP" by default) using the connected microphone. When the wake word is detected, the system activates and listens for a voice command.

### How it works:

1. The system continuously samples audio from the microphone
2. Audio is processed to detect energy levels
3. When energy exceeds a threshold, the wake word is considered detected
4. The system then records a few seconds of audio for the command
5. The command is processed and a response is generated

Note: The current implementation uses a simple energy threshold for wake word detection. A more sophisticated algorithm would use techniques like MFCC features and pattern matching.

## Configuration

All configuration options are in the `lib/config.h` file:

```cpp
// Audio processing configuration
#define AUDIO_ENABLED true           // Enable/disable audio processing
#define MIC_PIN 34                   // Analog pin for microphone input (ADC1_CH6)
#define SAMPLE_RATE 16000            // Audio sample rate in Hz
#define AUDIO_BUFFER_SIZE 512        // Size of audio buffer for processing

// Wake word detection configuration
#define WAKE_WORD_ENABLED true       // Enable/disable wake word detection
#define DEFAULT_WAKE_WORD "hey esp"  // Default wake word
#define WAKE_WORD_THRESHOLD 2000     // Energy threshold for wake word detection
#define WAKE_WORD_TIMEOUT 10000      // Timeout after wake word detection (ms)
```

Adjust these values based on your specific hardware and requirements.

## Web Interface

The system provides a web interface accessible at the ESP32's IP address. The interface allows:

- Text-based interaction with the AI assistant
- Voice input through the browser's microphone
- Code highlighting for programming examples
- Responsive design for mobile and desktop

## Getting Started

1. Clone this repository
2. Update the WiFi credentials and OpenAI API key in `lib/config.h`
3. Connect the microphone to the ESP32
4. Build and upload the project using PlatformIO
5. Open the serial monitor to see the ESP32's IP address
6. Access the web interface at the displayed IP address

## OTA Updates

The system supports Over-The-Air updates. After the initial upload via USB, you can update the firmware wirelessly:

```
pio run -t upload --upload-port [ESP-IP-ADDRESS]
```

The default password for OTA updates is "admin" (change this in production).

## Future Improvements

- Implement more sophisticated wake word detection
- Add audio output for spoken responses
- Improve voice command processing
- Add support for multiple wake words
- Implement local wake word detection without cloud API