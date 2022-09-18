/*
 * ESP8266 LED STRIPE for Photobooth > https://github.com/andi34/photobooth
 * Raphael Schib (https://github.com/flighter18)
 */

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif


//
// U S E R S E T U P  S T A R T S  H E R E
//

// NeoPixels connected pin
#define LED_PIN D4

// NeoPixels count
#define LED_COUNT 60

// ADD SSID AND PASSWORD
// *****************************
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";

// ADD NETWORK SETTINGS
IPAddress ip(xx,xx,xx,xx);         // where xx is the desired IP Address
IPAddress gateway(xx,xx,xx,xx);  // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network

// NeoPixels changing on Photo Countdown (value = LED_COUNT / Countdown in seconds), Photbooth defaults to 10 seconds
int cntdwnPhoto = 6;
// NeoPixels changing on Collage Countdown (value = LED_COUNT / Countdown in seconds), Photbooth defaults to 5 seconds
int cntdwnCollage = 12;
// Set BRIGHTNESS to about 1/5 (max = 255)
int brightness = 150;

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
// LED Colors
uint32_t color = strip.Color(0,   0,   0);
uint32_t fillColor = strip.Color(255, 255, 255);

//
// U S E R S E T U P  E N D S  H E R E
//

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.print(F("Setting static ip to : "));
  Serial.println(ip);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet); 
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());

  // reconnect automatically after lost connection
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);

  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.println("");

  // LED STRIP
  Serial.print("LED Count: ");
  Serial.println(LED_COUNT);
  Serial.print("Turn off count for photo: ");
  Serial.println(cntdwnPhoto);
  Serial.print("Turn off count for collage: ");
  Serial.println(cntdwnCollage);
  Serial.println("");

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  // END of Trinket-specific code.

  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.begin();
  // Turn OFF all pixels ASAP
  strip.show();
  strip.setBrightness(brightness);
}

void setCrossOrigin() {
  server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
  server.sendHeader(F("Access-Control-Max-Age"), F("600"));
  server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
  server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  int value = LOW;

  // Match the request
  if (request.indexOf("/CNTDWNPHOTO") != -1) {
    Serial.println("PHOTO COUNTDOWN");
    photoled(1000, cntdwnPhoto);
  } 
  if (request.indexOf("/CNTDWNCOLLAGE") != -1) {
    Serial.println("COLLAGE COUNTDOWN");
    photoled(1000, cntdwnCollage);
  } 
  if (request.indexOf("/collage") != -1){
    Serial.println("COLLAGE"); 
    theaterChaseRainbow(30);
  }
  if (request.indexOf("/photo") != -1){
    Serial.println("PHOTO");
    rainbow(4);
  }
  if (request.indexOf("/chroma") != -1){
    Serial.println("CHROMA");
    rainbow(4);
  }
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.println("<br><br>");
  client.println("Click <a href=\"/CNTDWNPHOTO\">here</a> to start the animation visible for photo countdown<br>");
  client.println("Click <a href=\"/CNTDWNCOLLAGE\">here</a> to start the animation visible for collage countdown<br>");
  client.println("Click <a href=\"/collage\">here</a> to start the animation visible once a collage was processed<br>");
  client.println("Click <a href=\"/photo\">here</a> to start the animation visible once a photo was processed<br>");
  client.println("Click <a href=\"/chroma\">here</a> to start the animation visible once a chroma photo was processed<br>");
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disconnected.");
  Serial.println("");
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         // Set pixel's color (in RAM)
    strip.show();                          // Update strip to match
    delay(wait);                           // Pause for a moment
  }
}

void photoled(int wait, int ledChange) {
  int ledLimit = ledChange * 2;
  int p = LED_COUNT;

  strip.fill(fillColor,2, LED_COUNT);
  strip.show();
  delay(1000);

  // For each pixel in strip...
  for(int i=0;i<strip.numPixels();) {
    if (ledLimit > p) {
      strip.fill(color,i + p ,p);
      i = i + p;
    } else {
      strip.fill(color,i,ledChange);
    }

    // Update strip to match
    strip.show();
    if (i>=LED_COUNT){
      delay(100);
      break;
    }
    i = i + ledChange;
    p = p - ledChange;
    delay(wait);
  }

  stripClear();
}

void rainbow(int wait) {
   strip.clear();
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 1*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
  stripClear();
}

void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<20; a++) {  // Repeat 20 times...
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
  stripClear();
}


void stripClear() {
  for (int ii = 0; ii < strip.numPixels(); ++ii) {  
    strip.setPixelColor(ii, 0, 0, 0);  
  }
  strip.show();
}
