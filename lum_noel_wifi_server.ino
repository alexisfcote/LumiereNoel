#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>   // Include the SPIFFS library
#include "wifiindentity.h" // put your ssid and password here

void set_color(int r, int g, int b);

const int ledVert = 4;
const int ledRouge = 14;
const int ledBleu = 16;
const int MAXVAL = 900;

int valueVert  = 0;
int valueRouge = 0;
int valueBleu  = 0;

bool ison = true;

bool blinking  = false;
unsigned long last_blink = 0;
bool isblink = false;

bool pulse     = false;
float  pulsevalue = 0.5;
float pulsedir = 1.0;

// HTTP server will listen at port 80
ESP8266WebServer server(80);

void handle_on() {
  ison = (bool)server.arg("state").toInt();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handle_color() {
  String color = server.arg("color");
  if (color.indexOf("rouge") != -1) {
    set_color(MAXVAL, 0, 0);

  }
  if (color.indexOf("vert") != -1) {
    set_color(0, MAXVAL, 0);
  }
  if (color.indexOf("bleu") != -1) {
    set_color(0, 0, MAXVAL);
  }
  if (color.indexOf("turquoise") != -1) {
    set_color(0, 800, 900);
  }
  if (color.indexOf("jaune") != -1) {
    set_color(800, 300, 0);
  }
  if (color.indexOf("orange") != -1) {
    set_color(MAXVAL, 250, 0);
  }
  if (color.indexOf("blanc") != -1) {
    set_color(900, 500, 350);
  }
  if (color.indexOf("mauve") != -1) {
    set_color(300, 0, 900);
  }
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handle_led() {
  // get the value of request argument "..." and convert it to an int
  valueRouge = _min(server.arg("rouge").toInt(), MAXVAL);
  valueVert = _min(server.arg("vert").toInt(), MAXVAL);
  valueBleu = _min(server.arg("bleu").toInt(), MAXVAL);

  server.send ( 200, "text/plain", "");
}

void handle_blinking() {
  blinking = (bool)server.arg("state").toInt();
  pulse = false;
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handle_pulse() {
  pulse = (bool)server.arg("state").toInt();
  blinking = false;
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("");
  delay(10);

  pinMode(ledVert, OUTPUT);
  pinMode(ledRouge, OUTPUT);
  pinMode(ledBleu, OUTPUT);
  digitalWrite(ledVert, 0);

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(ledVert, !valueVert);
    valueVert = !valueVert;
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting filesystem");
  SPIFFS.begin();

  // Set up the endpoints for HTTP server
  //
  // Endpoints can be written as inline functions:
  server.on("/", []() {
    File file = SPIFFS.open("/lumiere_noel.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  // And as regular external functions:
  server.on("/on", handle_on);
  server.on("/color", handle_color);
  server.on("/led", handle_led);
  server.on("/blink", handle_blinking);
  server.on("/pulse", handle_pulse);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
  set_color(0, 800, 900);
}

void do_blinking() {
  if (!blinking) {
    return;
  }
  if (millis() > (last_blink + 500)) {
    isblink = !isblink;
    last_blink = millis();
  }
  if (last_blink > millis()) { // handle overflow
    last_blink = 0;
  }

  if (isblink) {
    analogWrite(ledRouge, 0);
    analogWrite(ledVert, 0);
    analogWrite(ledBleu, 0);
  }
  else {
    analogWrite(ledRouge, valueRouge);
    analogWrite(ledVert, valueVert);
    analogWrite(ledBleu, valueBleu);
  }

}

void do_pulse() {
  if (!pulse) {
    return;
  }
  if (pulsevalue > 1) {
    pulsedir = -pulsedir;
  }
  if (pulsevalue < 0.1) {
    pulsedir = -pulsedir;
  }
  pulsevalue += pulsedir / 1000.0;

  analogWrite(ledRouge, (int) (pulsevalue * valueRouge));
  analogWrite(ledVert, (int) (pulsevalue * valueVert));
  analogWrite(ledBleu, (int) (pulsevalue * valueBleu));
}

void set_color(int r, int g, int b) {
  valueRouge = r;
  valueVert  = g;
  valueBleu  = b;
}

void loop(void) {
  // check for incomming client connections frequently in the main loop:
  server.handleClient();
  if (ison) {
    analogWrite(ledRouge, valueRouge);
    analogWrite(ledVert, valueVert);
    analogWrite(ledBleu, valueBleu);
    do_blinking();
    do_pulse();
  }
  else {
    analogWrite(ledRouge, 0);
    analogWrite(ledVert, 0);
    analogWrite(ledBleu, 0);
  }

  delay(5);
}
