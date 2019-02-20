//NodeMCU 1.0
#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#define PIN D1
#define RELE D2
#define PRESENCE A0
#define NUM_PIXELS  24

const char* ssid = "TNCAP3483C9";
const char* password = "";
bool ligthStatus = false;

// Create NeoPixel object
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800 + NEO_KHZ400);

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void lightStrip(int r, int g, int b) {
  for ( int i = 0; i < NUM_PIXELS; i++) {
    strip.setPixelColor(i, r, g, b); strip.show();
  }
}

// clearStrip
void clearStrip() {
  lightStrip(0, 0, 0);
}


void redStrip() {
  lightStrip(255, 0, 0);
}


void greenStrip() {
  lightStrip(0, 255, 0);
}

void blueStrip() {
  lightStrip(0, 0, 255);
}

void whiteStrip() {
  lightStrip(255, 255, 255);
}

uint32_t dimColor(uint32_t color, uint8_t width) {
  return (((color & 0xFF0000) / width) & 0xFF0000) + (((color & 0x00FF00) / width) & 0x00FF00) + (((color & 0x0000FF) / width) & 0x0000FF);
}

// Using a counter and for() loop, input a value 0 to 251 to get a color value.
// The colors transition like: red - org - ylw - grn - cyn - blue - vio - mag - back to red.
// Entering 255 will give you white, if you need it.
uint32_t colorWheel(byte WheelPos) {
  byte state = WheelPos / 21;
  switch (state) {
    case 0: return strip.Color(255, 0, 255 - ((((WheelPos % 21) + 1) * 6) + 127)); break;
    case 1: return strip.Color(255, ((WheelPos % 21) + 1) * 6, 0); break;
    case 2: return strip.Color(255, (((WheelPos % 21) + 1) * 6) + 127, 0); break;
    case 3: return strip.Color(255 - (((WheelPos % 21) + 1) * 6), 255, 0); break;
    case 4: return strip.Color(255 - (((WheelPos % 21) + 1) * 6) + 127, 255, 0); break;
    case 5: return strip.Color(0, 255, ((WheelPos % 21) + 1) * 6); break;
    case 6: return strip.Color(0, 255, (((WheelPos % 21) + 1) * 6) + 127); break;
    case 7: return strip.Color(0, 255 - (((WheelPos % 21) + 1) * 6), 255); break;
    case 8: return strip.Color(0, 255 - ((((WheelPos % 21) + 1) * 6) + 127), 255); break;
    case 9: return strip.Color(((WheelPos % 21) + 1) * 6, 0, 255); break;
    case 10: return strip.Color((((WheelPos % 21) + 1) * 6) + 127, 0, 255); break;
    case 11: return strip.Color(255, 0, 255 - (((WheelPos % 21) + 1) * 6)); break;
    default: return strip.Color(0, 0, 0); break;
  }
}
// Cycles - one cycle is scanning through all pixels left then right (or right then left)
// Speed - how fast one cycle is (32 with 16 pixels is default KnightRider speed)
// Width - how wide the trail effect is on the fading out LEDs.  The original display used
//         light bulbs, so they have a persistance when turning off.  This creates a trail.
//         Effective range is 2 - 8, 4 is default for 16 pixels.  Play with this.
// Color - 32-bit packed RGB color value.  All pixels will be this color.
// knightRider(cycles, speed, width, color);
void knightRider(uint16_t cycles, uint16_t speed, uint8_t width, uint32_t color) {
  uint32_t old_val[NUM_PIXELS]; // up to 256 lights!
  // Larson time baby!
  for (int i = 0; i < cycles; i++) {
    for (int count = 1; count < NUM_PIXELS; count++) {
      strip.setPixelColor(count, color);
      old_val[count] = color;
      for (int x = count; x > 0; x--) {
        old_val[x - 1] = dimColor(old_val[x - 1], width);
        strip.setPixelColor(x - 1, old_val[x - 1]);
      }
      strip.show();
      delay(speed);
    }
    for (int count = NUM_PIXELS - 1; count >= 0; count--) {
      strip.setPixelColor(count, color);
      old_val[count] = color;
      for (int x = count; x <= NUM_PIXELS ; x++) {
        old_val[x - 1] = dimColor(old_val[x - 1], width);
        strip.setPixelColor(x + 1, old_val[x + 1]);
      }
      strip.show();
      delay(speed);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.print("Begin ! ");
  // Initialize the rele variables as outputs
  pinMode(RELE, OUTPUT);
  //pinMode(PRESENCE, INPUT_PULLUP);
  //digitalWrite(PRESENCE,LOW);
  digitalWrite(RELE, HIGH); // Main light off
  Serial.print("Rele ok ! ");
  strip.begin();
  clearStrip(); // Initialize all pixels to 'off'
  delay(1000);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  delay(30);
  int v=analogRead(PRESENCE);

  if(v>500){
    digitalWrite(RELE, LOW);
    Serial.println("Presence detected.");
    
    }else{
    digitalWrite(RELE, HIGH);
    // Gestire stato
    }
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /rele/on") >= 0) {
              Serial.println("Rele on");
              ligthStatus=true;
              digitalWrite(RELE, LOW);
              
            } else if (header.indexOf("GET /rele/off") >= 0) {
              Serial.println("Rele off");
              ligthStatus=false;
              digitalWrite(RELE, HIGH);
              
            } else if (header.indexOf("GET /led/red/on") >= 0) {
              Serial.println("Led RED on");
              redStrip();

            } else if (header.indexOf("GET /led/off") >= 0) {
              Serial.println("Led off");
              clearStrip();

            } else if (header.indexOf("GET /led/green/on") >= 0) {
              Serial.println("Led Green ON");
              greenStrip();

            } else if (header.indexOf("GET /led/blue/on") >= 0) {
              Serial.println("Led Blue ON");
              blueStrip();
              
            } else if (header.indexOf("GET /led/white/on") >= 0) {
              Serial.println("Led White ON");
              whiteStrip();
              
            } else if (header.indexOf("GET /led/blue/knight") >= 0) {
              Serial.println("Led Blue Knight");
              knightRider(3, 32, 2, 0x0000FF); // Cycles, Speed, Width, RGB Color (blue)

            } else if (header.indexOf("GET /led/red/knight") >= 0) {
              Serial.println("Led Red Knight");
              knightRider(3, 32, 5, 0xFF0000); // Cycles, Speed, Width, RGB Color (red)

            } else if (header.indexOf("GET /led/green/knight") >= 0) {
              Serial.println("Led Green Knight");
              knightRider(3, 32, 5, 0x00FF00); // Cycles, Speed, Width, RGB Color (green)
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1> OK");


            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
