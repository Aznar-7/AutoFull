#include <WiFi.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>
#include <LittleFS.h>



WebServer server(80);

String obstacleData = "000";   // valor inicial para evitar respuestas vacías

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  //RX Y TX respectivamente
  WiFi.mode(WIFI_STA);


  WiFiManager wm;

  bool response;

  response = wm.autoConnect("AutoAp", "Auto2308");

  if (!response) {
    Serial.print("No conectado");
  } else {
    Serial.print("Conectado");
    Serial.println("Direccion IP: ");
    Serial.print(WiFi.localIP());
  }

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS fallo");
  }


  server.serveStatic("/", LittleFS, "/index.html");
  server.serveStatic("/index.html", LittleFS, "/index.html");
  server.serveStatic("/script.js", LittleFS, "/script.js");
  server.serveStatic("/style.css", LittleFS, "/style.css");


  //Endpoints
  server.on("/command", HTTP_GET, []() {
    String command = server.arg("cmd");

    if (command == "forward") {
      Serial2.print("F\n");
    } else if (command == "backward") {
      Serial2.print("B\n");
    } else if (command == "left") {
      Serial2.print("L\n");
    } else if (command == "right") {
      Serial2.print("R\n");
    } else if (command == "stop") {
      Serial2.print("S\n");
    }

    Serial.println(command);

    server.send(200, "text/plain", "Command sent!" + command);
  });

  server.on("/status", HTTP_GET, []() {
    server.send(200, "text/plain", obstacleData);
  });

  MDNS.begin("auto");

  server.begin();
  Serial.println("Server HTTP iniciado");
}

void loop() {
  // put your main code here, to run repeatedly:
  /*Creamos un puente entre el uno y el esp

  if (Serial.available()) {
    char c = Serial.read();
    Serial2.write(c);
  }

  
  if (Serial2.available()) {
    char c = Serial2.read();
    Serial.write(c);
  }*/

  server.handleClient();

  // Lectura NO bloqueante del UNO (mejor reacción)
  static String buffer = "";
  while (Serial2.available()) {
    char c = (char)Serial2.read();

    if (c == '\n') {
      buffer.trim();                // limpiamos espacios
      if (buffer.length() > 0) {
        obstacleData = buffer;      // actualizamos estado sensores
        //Serial.println("SENS: " + obstacleData); // debug opcional
      }
      buffer = "";
    } else {
      buffer += c;
      if (buffer.length() > 32) {   // protección ante basura
        buffer = "";
      }
    }
  }
}
