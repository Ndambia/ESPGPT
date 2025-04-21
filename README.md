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
3. When energy exceeds a threshold, the wake word is considered detecteds
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

The system supports Over-The-Air updates, allowing you to update the firmware wirelessly without a USB connection. This is particularly useful for devices that are installed in hard-to-reach locations.

### Initial Setup

1. First upload via USB using: `pio run -t upload`
2. Note the ESP32's IP address from the serial monitor
3. Uncomment the OTA configuration in `platformio.ini`:
   ```ini
   upload_protocol = espota
   upload_port = 192.168.0.100  # Replace with your ESP32's IP address
   upload_flags =
     --auth=admin
   ```
4. Make sure the password in `main.cpp` matches the one in `platformio.ini`:
   ```cpp
   ArduinoOTA.setPassword("admin");  // This must match the --auth value
   ```

### Performing OTA Updates

After the initial setup, you can update the firmware wirelessly using either:

1. **PlatformIO IDE**: Simply click the upload button, and it will use the OTA configuration
2. **Command Line**: Run `pio run -t upload`
3. **Manual Command**: Use `pio run -t upload --upload-port [ESP-IP-ADDRESS] --upload-flags="--auth=admin"`
4. **PowerShell**:` & "$env:USERPROFILE\.platformio\penv\Scripts\pio.exe" run -t upload --upload-port 192.168.0.100`

### Troubleshooting OTA Updates

If you encounter authentication failures:

1. **Check Password Configuration**: Ensure the password in `main.cpp` (`ArduinoOTA.setPassword("admin")`) matches the one in `platformio.ini` (`--auth=admin`)
2. **Verify IP Address**: Make sure the ESP32's IP address in `platformio.ini` is correct and up-to-date
3. **Enable Debug Logs**: Uncomment the debug flags in `platformio.ini`:
   ```ini
   build_flags = -DDEBUG_ESP_OTA -DDEBUG_ESP_PORT=Serial
   ```
4. **Network Issues**: Ensure the ESP32 and your computer are on the same network
5. **Firewall/Router**: Check if your firewall or router is blocking the OTA port (default: 3232)

### Security Considerations

- Change the default password ("admin") in production environments
- Only perform OTA updates on secure networks
- Consider implementing additional security measures like SSL/TLS for sensitive applications

## Project Structure

```
├── include/                  # Header files
│   ├── audio_processor.h     # Audio processing and wake word detection
│   ├── knowledge_base.h      # Local knowledge storage and retrieval
│   ├── openai_client.h       # OpenAI API integration
│   └── web_server.h          # Web interface implementation
├── lib/                      # Libraries and configuration
│   └── config.h              # Project configuration settings
├── src/                      # Source files
│   └── main.cpp              # Main application code
├── platformio.ini            # PlatformIO configuration
└── README.md                 # Project documentation
```

### Key Components

1. **Audio Processor**: Handles microphone input, audio sampling, and wake word detection
2. **Knowledge Base**: Stores and retrieves information locally on the ESP32
3. **OpenAI Client**: Communicates with the OpenAI API for natural language processing
4. **Web Server**: Provides a user interface accessible via web browser
5. **Main Application**: Coordinates all components and handles system initialization

## Implementation Details

### Memory Management

The ESP32 has limited RAM, so the system uses several techniques to optimize memory usage:
- Streaming audio processing instead of buffering large chunks
- Efficient string handling to minimize heap fragmentation
- Careful management of JSON parsing to avoid memory leaks

### Power Considerations

For battery-powered applications, consider:
- Implementing deep sleep between wake word detections
- Reducing the sampling rate for audio processing
- Using ESP32's power management features
- Implementing a timeout for WiFi and web server when not in use

### Performance Optimization

- The system uses the ESP32's dual cores effectively
- Audio processing runs on Core 0
- Network and web server operations run on Core 1
- Critical sections are protected with mutexes to prevent race conditions

## Future Improvements

- Implement more sophisticated wake word detection using MFCC and neural networks
- Add audio output for spoken responses through a connected speaker
- Improve voice command processing with local keyword spotting
- Add support for multiple wake words and user profiles
- Implement local wake word detection without cloud API
- Add SSL/TLS support for secure web interface
- Implement a mobile app companion for easier configuration

## Troubleshooting Guide

### Common Issues and Solutions

#### WiFi Connection Problems

**Symptoms**: ESP32 keeps restarting, serial monitor shows "WiFi connection timeout!"

**Solutions**:
1. Verify WiFi credentials in `config.h`
2. Check if your router supports 2.4GHz (ESP32 doesn't support 5GHz)
3. Ensure the ESP32 is within range of your WiFi router
4. Try reducing WiFi power-saving settings in your router

#### OTA Update Authentication Failures

**Symptoms**: "Authenticating...FAIL", "Authentication Failed" during OTA updates

**Solutions**:
1. Ensure `ArduinoOTA.setPassword("admin")` is uncommented in `main.cpp`
2. Verify that the password matches the `--auth=admin` flag in `platformio.ini`
3. Check that `upload_protocol = espota` is uncommented in `platformio.ini`
4. Verify the correct IP address is set in `upload_port` in `platformio.ini`
5. Try restarting the ESP32 before attempting the OTA update

#### Microphone Not Detecting Wake Word

**Symptoms**: Wake word detection doesn't trigger, or triggers randomly

**Solutions**:
1. Adjust the `WAKE_WORD_THRESHOLD` value in `config.h`
2. Check microphone connections and power supply
3. Try a different GPIO pin and update `MIC_PIN` in `config.h`
4. Ensure the microphone module has proper gain settings

#### Web Server Not Accessible

**Symptoms**: Cannot access the web interface at the ESP32's IP address

**Solutions**:
1. Verify the ESP32 is connected to WiFi (check serial monitor)
2. Ensure your computer is on the same network as the ESP32
3. Try accessing with both IP address and port (e.g., 192.168.0.100:80)
4. Check if any firewall is blocking the connection
5. Try restarting the ESP32 and your router

#### OpenAI API Issues

**Symptoms**: AI responses not working, timeout errors

**Solutions**:
1. Verify your OpenAI API key in `config.h`
2. Check if you have sufficient credits in your OpenAI account
3. Ensure the ESP32 has a stable internet connection
4. Try increasing timeout values for HTTP requests

### Debugging Techniques

1. **Serial Monitoring**: Enable detailed logging with `Serial.println()` statements
2. **Memory Debugging**: Use `ESP.getFreeHeap()` to monitor memory usage
3. **Network Debugging**: Use tools like Wireshark to analyze network traffic
4. **Logic Analyzer**: For timing-sensitive issues, use a logic analyzer on GPIO pins

### Getting Help

If you're still experiencing issues:
1. Open an issue on the GitHub repository with detailed information
2. Include serial monitor output, hardware details, and steps to reproduce
3. Share any modifications you've made to the code
