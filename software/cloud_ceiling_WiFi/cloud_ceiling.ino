#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>

#define NUM_LEDS 134
#define DATA_PIN D2 //2 
#define COLOR_ORDER BRG
#define MAX_BRIGHTNESS 150
#define FRAMES_PER_SECOND 100   //100
#define NUM_EFFECTS 7
#define clientID "zja8724na"    //Tiene que tener entre 8 y 12 carácteres

//WIFI
const char* ssid = "NuestraCasa2.4G"; // Nombre del Wifi
//*****************************
const char* password =  "soloparaamigos"; // Contraseña del Wifi
//MQTT
const char* mqtt_server = "industrial.api.ubidots.com";
const int PORT = 1883;
//********************************
const char* usernameUbidots = "BBFF-zUkukH3XmH1vLBJtDFK3un10yOMfRT";   //Segunda cuenta de ubidots
const char* passUbidots = "";    //La contraseña se deja en blanco
//MQTT TOPICS
//******************************************
//***Recuerda: /v1.6/devices/nombre de tu devive/nombre variable/lv
const char* topic_red = "/v1.6/devices/cloudceiling/red/lv";
const char* topic_green = "/v1.6/devices/cloudceiling/green/lv";
const char* topic_blue = "/v1.6/devices/cloudceiling/blue/lv";
const char* topic_brightness = "/v1.6/devices/cloudceiling/brightness/lv";
const char* topic_effects = "/v1.6/devices/cloudceiling/effects/lv";

//LED MATRIX
bool color_changed = false;
bool brightness_changed = false;
bool effect_changed = false;
int red_color = 0;
int blue_color = 0;
int green_color = 0;
unsigned int brightness = 100;
int effect = 0;

//Pacifica
CRGBPalette16 pacifica_palette_1 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50
};
CRGBPalette16 pacifica_palette_2 =
{ 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117,
  0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F
};
CRGBPalette16 pacifica_palette_3 =
{ 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33,
  0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF
};

//ColorPalette
CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = LINEARBLEND;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;



WiFiClient espClient;
PubSubClient client(espClient);
CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  mqttSetup();
  matrixSetup();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    reconnect();
  }

  if (color_changed == true) {
    color_changed = false;
    effect_changed = false;
    updateMatrixColor();
  }

  if (brightness_changed == true) {
    brightness_changed = false;
    FastLED.setBrightness(brightness);
    FastLED.show();
  }

  if (effect_changed == true) {
    static uint8_t startIndex = 0;
    currentBlending = LINEARBLEND;
    switch (effect) {
      case 1:
        pacifica_loop();
        break;

      case 2:
        startIndex = startIndex + 1; /* motion speed */
        currentPalette = RainbowColors_p;
        FillLEDsFromPaletteColors( startIndex);
        FastLED.show();
        break;
      case 3:
        startIndex = startIndex + 1; /* motion speed */
        currentPalette = CloudColors_p;
        FillLEDsFromPaletteColors( startIndex);
        FastLED.show();
        break;
      case 4:
        startIndex = startIndex + 1; /* motion speed */
        currentPalette = OceanColors_p;
        FillLEDsFromPaletteColors( startIndex);
        FastLED.show();
        break;
      case 5:
        startIndex = startIndex + 1; /* motion speed */
        currentPalette = ForestColors_p;
        FillLEDsFromPaletteColors( startIndex);
        FastLED.show();
        break;
      case 6:
        startIndex = startIndex + 1; /* motion speed */
        currentPalette = LavaColors_p;
        FillLEDsFromPaletteColors( startIndex);
        FastLED.show();
        break;

      case 7:
        thunder();
        break;
    }
  }

  //FastLED.show();
  client.loop();
  delay(1000 / FRAMES_PER_SECOND);
}

void mqttSetup() {
  //Nos conectamos al WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to Wifi...");
  }

  Serial.println("Connected to the Wifi network");
  //Establecemos el servidor, Ubidotots - No nos conectamos
  client.setServer(mqtt_server, PORT);
  //Establecemos la función que se ejecuta cuando recibimos un
  //mensaje MQTT
  client.setCallback(callback);
}

void matrixSetup() {
  FastLED.addLeds<WS2811, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].r = 255;
    leds[dot].g = 0;
    leds[dot].b = 0;

  }
  FastLED.show();
}

void updateMatrixColor() {
  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].r = red_color;
    leds[dot].g = green_color;
    leds[dot].b = blue_color;
  }
  FastLED.show();
}

void reconnect() {
  //Comprueba si estamos conectados al Wifi
  //Si no es así se conecta
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.println("Connecting to Wifi...");
    }
  }
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect(clientID, usernameUbidots, passUbidots)) {
      Serial.println("connected");
      //Suscription to topics
      client.subscribe(topic_red);
      client.subscribe(topic_green);
      client.subscribe(topic_blue);
      client.subscribe(topic_brightness);
      client.subscribe(topic_effects);
    }

    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  char msg[length];
  int ipayload;     //Int payload

  for (int i = 0; i < length; i++) {
    msg[i] = (char)payload[i];
  }

  ipayload = atoi(msg);
  Serial.println(ipayload);

  //No se puede hacer topic == TRed porque son punteros
  //Hay que usar strcmp para ver si son iguales
  if (strcmp(topic, topic_red) == 0) {
    color_changed = true;
    red_color = ipayload;
  }

  else if (strcmp(topic, topic_green) == 0) {
    color_changed = true;
    green_color = ipayload;
  }

  else if (strcmp(topic, topic_blue) == 0) {
    color_changed = true;
    blue_color = ipayload;
  }

  else if (strcmp(topic, topic_brightness) == 0) {
    if (ipayload <= MAX_BRIGHTNESS) {
      brightness_changed = true;
      brightness = ipayload;
    }
  }

  else if (strcmp(topic, topic_effects) == 0) {
    if (ipayload <= NUM_EFFECTS) {
      effect_changed = true;
      effect = ipayload;
    }
  }
}





/////////////////////////////////////////
////////////////EFFECTS//////////////////
/////////////////////////////////////////

//THUNDER

void thunder() {
  FastLED.clear();
  FastLED.show();

  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].r = 45;
    leds[dot].g = 220;
    leds[dot].b = 255;
    delay(20);
    FastLED.show();
  }

  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].r = 255;
    leds[dot].g = 232;
    leds[dot].b = 60;
    delay(20);
    FastLED.show();
  }

  for (int dot = 0; dot < NUM_LEDS; dot++) {
    leds[dot].r = 0;
    leds[dot].g = 0;
    leds[dot].b = 0;
    delay(15);
    FastLED.show();
  } 
}


//PACIFICA
void pacifica_loop()
{
  // Increment the four "color index start" counters, one for each wave layer.
  // Each is incremented at a different speed, and the speeds vary over time.
  static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
  static uint32_t sLastms = 0;
  uint32_t ms = GET_MILLIS();
  uint32_t deltams = ms - sLastms;
  sLastms = ms;
  uint16_t speedfactor1 = beatsin16(3, 179, 269);
  uint16_t speedfactor2 = beatsin16(4, 179, 269);
  uint32_t deltams1 = (deltams * speedfactor1) / 256;
  uint32_t deltams2 = (deltams * speedfactor2) / 256;
  uint32_t deltams21 = (deltams1 + deltams2) / 2;
  sCIStart1 += (deltams1 * beatsin88(1011, 10, 13));
  sCIStart2 -= (deltams21 * beatsin88(777, 8, 11));
  sCIStart3 -= (deltams1 * beatsin88(501, 5, 7));
  sCIStart4 -= (deltams2 * beatsin88(257, 4, 6));

  // Clear out the LED array to a dim background blue-green
  fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));

  // Render each of four layers, with different scales and speeds, that vary over time
  pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0 - beat16( 301) );
  pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
  pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10, 38), 0 - beat16(503));
  pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10, 28), beat16(601));

  // Add brighter 'whitecaps' where the waves lines up more
  pacifica_add_whitecaps();

  // Deepen the blues and greens a bit
  pacifica_deepen_colors();
  FastLED.show();
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );

  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if ( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for ( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145);
    leds[i].green = scale8( leds[i].green, 200);
    leds[i] |= CRGB( 2, 5, 7);
  }
}


//COLOR PALETTE
void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}

void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if ( lastSecond != secondHand) {
    lastSecond = secondHand;
    if ( secondHand ==  0)  {
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 10)  {
      currentPalette = RainbowStripeColors_p;
      currentBlending = NOBLEND;
    }
    if ( secondHand == 15)  {
      currentPalette = RainbowStripeColors_p;
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 20)  {
      SetupPurpleAndGreenPalette();
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 25)  {
      SetupTotallyRandomPalette();
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 30)  {
      SetupBlackAndWhiteStripedPalette();
      currentBlending = NOBLEND;
    }
    if ( secondHand == 35)  {
      SetupBlackAndWhiteStripedPalette();
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 40)  {
      currentPalette = CloudColors_p;
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 45)  {
      currentPalette = PartyColors_p;
      currentBlending = LINEARBLEND;
    }
    if ( secondHand == 50)  {
      currentPalette = myRedWhiteBluePalette_p;
      currentBlending = NOBLEND;
    }
    if ( secondHand == 55)  {
      currentPalette = myRedWhiteBluePalette_p;
      currentBlending = LINEARBLEND;
    }
  }
}

void SetupTotallyRandomPalette()
{
  for ( int i = 0; i < 16; ++i) {
    currentPalette[i] = CHSV( random8(), 255, random8());
  }
}

void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green,  green,  black,  black,
                     purple, purple, black,  black,
                     green,  green,  black,  black,
                     purple, purple, black,  black );
}

const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
  CRGB::Red,
  CRGB::Gray, // 'white' is too bright compared to red and blue
  CRGB::Blue,
  CRGB::Black,

  CRGB::Red,
  CRGB::Gray,
  CRGB::Blue,
  CRGB::Black,

  CRGB::Red,
  CRGB::Red,
  CRGB::Gray,
  CRGB::Gray,
  CRGB::Blue,
  CRGB::Blue,
  CRGB::Black,
  CRGB::Black
};
