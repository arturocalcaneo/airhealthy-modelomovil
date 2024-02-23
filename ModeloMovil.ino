#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
//====================================
#define MQ_PIN 34
#define A 116.6020682
#define B -2.769034857
#define Ro 28000.0
#define RL 10000.0
#define CO2_ATMOSFERICO 420
//====================================
#define DHT_PIN 4
#define DHTTYPE DHT11

DHT dht(DHT_PIN, DHTTYPE);
//====================================
#define ANCHO_DISPLAY 128
#define ALTO_DISPLAY 64

Adafruit_SSD1306 display(ANCHO_DISPLAY, ALTO_DISPLAY, &Wire, -1);
//====================================
const bool debug = true;
bool ContinuarMedicion = true;
bool DisplayOledFuncionando = false;
bool ResultoConexion = false;
//====================================
#include "funciones.h"
//====================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(2200);

  ResultoConexion = ConectarseAWiFi();

  if( ResultoConexion ){
    Serial.print("> Se establecio una conexion WiFi con ");
    Serial.print( SSID );
    Serial.println(".");
  }else{
    Serial.print("> No se pudo establecer conexion WiFi con ");
    Serial.print( SSID );
    Serial.println(".");
  }
  
  // Establecer lecturas de ADC en rango de 4095.0
  analogReadResolution(12);
  // Inicializar sensor DHT(11)
  dht.begin();
  // Inicializar Display OLED
  if( display.begin(SSD1306_SWITCHCAPVCC, 0x3C) ){
    DisplayOledFuncionando = true;

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.display();
  }
}

void loop() {
  // Verificar que el Display OLED se encuentre en linea.
  if( !DisplayOledFuncionando ){
    Serial.println("> No hay un dispositivo conectado por I2C al microcontrolador, se esperaba un display oled.");
  }

  float ADCvalue = (float) analogRead(MQ_PIN);
  int temp, hum;

  if( !isnan(dht.readTemperature()) ){
    temp = dht.readTemperature();
  }else{
    temp = 0;
  }
  if( !isnan(dht.readHumidity()) ){
    hum = dht.readHumidity();
  }else{
    hum = 0;
  }

  if( debug ){
    Serial.print("\n");

    if( ADCvalue > 0.00 ){
      Serial.print( "MQ135 (ADC) -> " );
      Serial.println( ADCvalue );
    }

    if( temp > 0 && hum > 0 ){
      Serial.print( "DHT11 (temperatura) ->" );
      Serial.println( temp );

      Serial.print( "DHT11 (humedad) ->" );
      Serial.println( hum );
    }
  }

  if( ContinuarMedicion ){
    if( ObtenerNivelConcentracionCO2(MQ_PIN) == 0.00 ){
      ContinuarMedicion = false;
    }else{
      const int ConcentracionCO2 = (int) ObtenerNivelConcentracionCO2(MQ_PIN);
      const int NivelCO2 = CO2_ATMOSFERICO + ConcentracionCO2;

      if( debug ){
        Serial.print( "MQ135 (CO2 ppm) -> " );
        Serial.println( NivelCO2 );
      }

      // Imprimir Nivel de CO2 en display oled
      if( DisplayOledFuncionando ){
        ImprimirNiveldeCO2(NivelCO2);
        // Imprimir niveles de temperatura
        ImprimirNiveldeTemperatura(temp);
        // Imprimir niveles de humedad
        ImprimirNiveldeHumedad(hum);

        display.display(); // Muestra la pantalla
        display.clearDisplay(); // Limpia la pantalla para la próxima actualización
      }

      if( ResultoConexion ){
        // Subir datos al servidor
        EnviarDatosAlServidor(NivelCO2, temp, hum);
      }else{
        Serial.println("> No se envio datos al servidor.");
      }
    }
  }

  delay(5000);
}

