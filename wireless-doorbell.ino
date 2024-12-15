#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <CircularBuffer.hpp>

// WiFi Configuration
const char* ssid = "WIFI-SSID";
const char* password = "WIFI-PWD";

// MQTT Configuration
const int mqtt_port = 1883;
const char* mqtt_server = "MOSQUITTO-HOST";
const char* mqtt_user = "MOSQUITTO-USER";
const char* mqtt_password = "MOSQUITTO-PWD";

  // Configuración de hostname y servidor web
const char* hostname = "timbre";  // Cambiar a "timbre", "receptor1" o "receptor2" según corresponda
ESP8266WebServer server(80);

// Buffer circular para almacenar logs
CircularBuffer<String, 100> logBuffer;  // Almacena últimos 100 mensajes

// Tópicos MQTT
const char* doorbell_topic = "casa/timbre";

// Configuración de pines
#define BUTTON_PIN 0
#define BUZZER_PIN 2

// Variables globales
bool isTransmitter = true;  // Cambiar según el dispositivo
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 200;
unsigned long lastMsg = 0;

// Melodías para el timbre
void playDingDong() {
    // Primer tono (ding)
    for(int i = 988; i > 494; i--) {  // Si5 a Si4
        tone(BUZZER_PIN, i);
        delay(1);
    }
    noTone(BUZZER_PIN);
    delay(100);
    
    // Segundo tono (dong)
    for(int i = 740; i > 370; i--) {  // Fa#5 a Fa#4
        tone(BUZZER_PIN, i);
        delay(1);
    }
    noTone(BUZZER_PIN);
}

void playTraditionalBell() {
    for(int i = 0; i < 3; i++) {  // 3 repeticiones
        tone(BUZZER_PIN, 988);  // Si5
        delay(150);
        tone(BUZZER_PIN, 494);  // Si4
        delay(150);
    }
    noTone(BUZZER_PIN);
}

// Función para agregar logs al buffer
void addLog(String message) {
  String timestamp = String(millis()/1000);
  String logMessage = "[" + timestamp + "s] " + message;
  Serial.println(logMessage);
  logBuffer.push(logMessage);
}

void setupWebServer() {
  // Página principal
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<title>Logs - " + String(hostname) + "</title>";
    html += "<meta http-equiv='refresh' content='5'>"; // Actualizar cada 5 segundos
    html += "<style>";
    html += "body { font-family: monospace; margin: 20px; }";
    html += ".log { white-space: pre-wrap; }";
    html += "</style></head><body>";
    html += "<h1>Logs del dispositivo: " + String(hostname) + "</h1>";
    html += "<div class='log'>";
    
    // Mostrar logs del buffer
    for(int i = 0; i < logBuffer.size(); i++) {
      html += logBuffer[i] + "<br>";
    }
    
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  });

  server.begin();
  addLog("Servidor web iniciado en http://" + String(hostname) + ".local");
}

void printMQTTState(int state) {
  String stateMsg;
  switch(state) {
    case -4: stateMsg = "MQTT_CONNECTION_TIMEOUT"; break;
    case -3: stateMsg = "MQTT_CONNECTION_LOST"; break;
    case -2: stateMsg = "MQTT_CONNECT_FAILED"; break;
    case -1: stateMsg = "MQTT_DISCONNECTED"; break;
    case 0: stateMsg = "MQTT_CONNECTED"; break;
    case 1: stateMsg = "MQTT_CONNECT_BAD_PROTOCOL"; break;
    case 2: stateMsg = "MQTT_CONNECT_BAD_CLIENT_ID"; break;
    case 3: stateMsg = "MQTT_CONNECT_UNAVAILABLE"; break;
    case 4: stateMsg = "MQTT_CONNECT_BAD_CREDENTIALS"; break;
    case 5: stateMsg = "MQTT_CONNECT_UNAUTHORIZED"; break;
    default: stateMsg = "MQTT_UNKNOWN_STATE"; break;
  }
  addLog("Estado MQTT: " + stateMsg);
}

void setup_wifi() {
  delay(10);
  addLog("Iniciando conexión WiFi...");
  addLog("Conectando a red: " + String(ssid));
  
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  addLog("¡WiFi conectado exitosamente!");
  addLog("Dirección IP: " + WiFi.localIP().toString());
  addLog("Hostname: " + String(hostname));
  addLog("RSSI: " + String(WiFi.RSSI()) + " dBm");

  // Iniciar mDNS
  if (MDNS.begin(hostname)) {
    addLog("mDNS responder iniciado");
    MDNS.addService("http", "tcp", 80);
  } else {
    addLog("Error iniciando mDNS!");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    
    addLog("Mensaje MQTT recibido - Tópico: " + String(topic) + ", Mensaje: " + message);

    if (!isTransmitter && strcmp(topic, doorbell_topic) == 0) {
        addLog("¡Timbre activado! Haciendo sonar timbre...");
        playTraditionalBell();  // Usa playDingDong() para sonido tipo "ding-dong"
        // o playTraditionalBell(); // Para sonido de timbre tradicional
        addLog("Timbre finalizado");
    }
}


void reconnect() {
  int retries = 0;
  while (!client.connected() && retries < 5) {
    addLog("Intentando conexión MQTT...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    addLog("ID de Cliente: " + clientId);
    
    printMQTTState(client.state());
    
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      addLog("¡Conectado exitosamente al broker MQTT!");
      
      if (!isTransmitter) {
        client.subscribe(doorbell_topic);
        addLog("Suscrito al tópico: " + String(doorbell_topic));
      } else {
        addLog("Modo transmisor - Listo para publicar");
      }
    } else {
      addLog("Falló conexión MQTT");
      printMQTTState(client.state());
      retries++;
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  addLog("=== Sistema de Timbre ESP-01S ===");
  addLog("Modo: " + String(isTransmitter ? "TRANSMISOR" : "RECEPTOR"));
  
  if (isTransmitter) {
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    addLog("GPIO0 configurado como entrada para botón");
  } else {
    pinMode(BUZZER_PIN, OUTPUT);
    addLog("GPIO2 configurado como salida para buzzer");
  }
  
  setup_wifi();
  setupWebServer();
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  server.handleClient();
  MDNS.update();

  // Debug de estado cada 30 segundos
  if (millis() - lastMsg > 30000) {
    lastMsg = millis();
    printMQTTState(client.state());
    addLog("WiFi RSSI: " + String(WiFi.RSSI()) + " dBm");
  }

  if (isTransmitter) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      if ((millis() - lastDebounceTime) > debounceDelay) {
        addLog("Botón presionado");
        printMQTTState(client.state());
        
        addLog("Publicando mensaje en tópico: " + String(doorbell_topic));
        
        if (client.publish(doorbell_topic, "ring")) {
          addLog("¡Mensaje publicado exitosamente!");
        } else {
          addLog("Error al publicar mensaje");
          printMQTTState(client.state());
        }
        
        lastDebounceTime = millis();
      }
    }
  }
}