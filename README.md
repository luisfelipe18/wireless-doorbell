# IoT Doorbell System with ESP8266

A distributed wireless doorbell system based on ESP8266 (ESP-01S) using MQTT for device communication. The system allows you to have a doorbell at the main entrance and multiple receivers at different locations in the house, each with its own web interface for monitoring.

## Features

- 🔔 Distributed wireless doorbell system
- 🌐 WiFi and MQTT connectivity
- 📱 Real-time web monitoring interface
- 🔊 Customizable melodies (Ding-dong and Traditional)
- 📝 Logging system with circular buffer
- 🏷️ mDNS support (access via hostname.local)
- 🔄 Automatic WiFi and MQTT reconnection
- 📊 Connection status monitoring

## Hardware Requirements

- ESP-01S (one for doorbell and one for each receiver)
- USB-TTL adapter (for programming)
- Passive buzzer (for receivers)
- Push button (for transmitter)
- 3.3V power supply for each ESP-01S
- Server for MQTT (can be a PC or Raspberry Pi)

## Software Requirements

### For ESP8266
- Arduino IDE
- Required libraries:
  - ESP8266WiFi
  - PubSubClient
  - ESP8266mDNS
  - ESP8266WebServer
  - CircularBuffer

### For Server
- Mosquitto MQTT Broker

## Installation

1. **Arduino IDE Setup**
   - Add ESP8266 board manager URL:
     ```
     http://arduino.esp8266.com/stable/package_esp8266com_index.json
     ```
   - Install ESP8266 support from Board Manager
   - Select "Generic ESP8266 Module" as board

2. **Install Libraries**
   - Open Library Manager in Arduino IDE
   - Install all libraries mentioned in requirements

3. **Hardware Setup**
   - **For Transmitter (Doorbell)**
     ```
     GPIO0 -> Button -> GND
     ```
   - **For Receivers**
     ```
     GPIO2 -> Buzzer -> GND
     ```

## Configuration

1. **Code Configuration**
   ```cpp
   // WiFi Configuration
   const char* ssid = "WIFI-SSID";
   const char* password = "WIFI-PWD";

   // MQTT Configuration
   const char* mqtt_server = "MOSQUITTO-HOST";
   const char* mqtt_user = "MOSQUITTO-USER";
   const char* mqtt_password = "MOSQUITTO-PWD";

   // Device Configuration
   const char* hostname = "doorbell";  // Or "receiver1", "receiver2"
   bool isTransmitter = true;  // true for doorbell, false for receivers
   ```

2. **MQTT Server Setup**
   - Install Mosquitto
   - Configure users and passwords
   - Enable port 1883
   - Configure firewall rules if needed

## Usage

1. **Web Access**
   - Transmitter: `http://doorbell.local`
   - Receivers: `http://receiver1.local`, `http://receiver2.local`
   - Or use IP addresses directly

2. **Monitoring**
   - Web interface shows last 100 log messages
   - Auto-refreshes every 5 seconds
   - Displays WiFi and MQTT connection status

3. **Available Melodies**
   - Ding-dong: Two-tone descending melody
   - Traditional: Series of alternating tones
   - To change the melody, modify in code:
     ```cpp
     playDingDong();     // For ding-dong melody
     playTraditionalBell(); // For traditional doorbell
     ```

## Troubleshooting

1. **WiFi Connection Issues**
   - Verify WiFi credentials
   - Check WiFi coverage
   - Review serial console logs

2. **MQTT Connection Issues**
   - Verify server address
   - Check MQTT credentials
   - Verify Mosquitto server is running
   - Check firewall rules

3. **Doorbell Not Ringing**
   - Verify isTransmitter mode
   - Check buzzer connection
   - Verify correct topic subscription

## Contributing

To contribute to the project:
1. Fork the repository
2. Create your feature branch: `git checkout -b feature/new-feature`
3. Commit your changes: `git commit -m 'feature(main change): your explanation for changess'`
4. Push to the branch: `git push origin feature/new-feature`
5. Submit a Pull Request

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.

## Acknowledgments

- ESP8266 Community for libraries and documentation
- Mosquitto Project for the MQTT broker
- Roberto Lo Giacco for the CircularBuffer library

## Author

Luis Felipe Villavicencio Lopez

## Project Status

🟢 Active - In development and maintenance

## Technical Details

### Network Architecture
```
[Doorbell ESP-01S] ---\
                       \
                        [MQTT Broker] --- [Receiver1 ESP-01S]
                       /                  [Receiver2 ESP-01S]
[WiFi Router] --------/                  [ReceiverN ESP-01S]
```

### MQTT Topics
- Main topic: `home/doorbell` (modify on code as your needs)
- Message format: Simple "ring" message
- QoS Level: 0 (At most once delivery)

### Web Interface Features
- Real-time log display
- Connection status monitoring
- Signal strength indication
- Auto-refresh functionality
- Mobile-responsive design

### Power Consumption
- Sleep mode: ~20mA
- Active mode: ~80mA
- Transmission: ~170mA peak

### Security Considerations
- WPA2 WiFi encryption
- MQTT authentication
- No exposure to public internet recommended
- Regular password updates advised

## Future Enhancements

1. **Planned Features**
   - Battery level monitoring
   - Multiple melody options
   - Mobile app integration
   - Push notifications

2. **Improvement Ideas**
   - Deep sleep implementation
   - OTA updates
   - Web-based configuration
   - Historical data logging
   - Integration with home automation systems
   - Solar charging for outdoor emitter.

3. **Security Enhancements**
   - TLS/SSL support
   - Certificate-based authentication
   - Rate limiting
   - Input validation

## Support

For support, please:
1. Check the Troubleshooting section
2. Review open/closed issues
3. Create a new issue with:
   - Detailed problem description
   - Log outputs
   - Hardware configuration
   - Network setup details
