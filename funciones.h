int ObtenerNivelConcentracionCO2(u_int8_t MQPIN){
  int ADCvalue = (float) analogRead(MQPIN);

  const float Vmax = 3.3;
  const float Vout = (ADCvalue / 4095.0) * Vmax;

  if( debug ){
    Serial.print( "MQ135 (voltaje) -> " );
    Serial.println( Vout );
  }

  // Rs = Resistencia del sensor en presencia de gas.
  const float Rs= ((Vmax / Vout) - 1) * RL;
  
  if( debug ){
    Serial.print( "MQ135 (Rs) -> " );
    Serial.println( Rs );
  }

  // Se aplica la fórmula para estimar el nivel de concentración.
  const float concentracion = A * ( pow((Rs/Ro),B) );

  if( debug ){
    Serial.print( "MQ135 (concentracion) -> " );
    Serial.println( concentracion );
  }

  return (int)concentracion;
}

void ImprimirNiveldeCO2(int NivelCO2){
  // Mostrar el Nivel de CO2
  display.println( "CO2 (PPM):" );
  display.println( "\n\n" );

  // Pintar la Barra Exterior
  display.drawRect(0, 19, ANCHO_DISPLAY - 1, 10, SSD1306_WHITE);

  int co2BarWidth = map(NivelCO2, 0, 2000, 0, ANCHO_DISPLAY - 2);
  // Pintar sobre la Barra el Progreso
  display.fillRect(1, 20, co2BarWidth, 8, SSD1306_WHITE);
}

void ImprimirNiveldeTemperatura(int NivelTemp){
  display.print("Temp (C): ");
  display.println(NivelTemp);
}

void ImprimirNiveldeHumedad(int NivelHumedad) {
  display.print("Hum (%): ");
  display.println(NivelHumedad);
}

//====================================================
const char* SSID= "NOMBRE-RED-WIFI"; // sustituir el dato por uno real
const char* PASS = "CONTRASEÑA"; // sustituir el dato por uno real
const char* DIRECCION = "https://posticous-powder.000webhostapp.com/enviardatos_sensor.php"; // Cambia esto por tu URL del servidor MySQL
bool HayConexion = false;
short timeout = 15;
short count = 0;
//====================================================

bool ConectarseAWiFi(){
  // Conectarse a red WiFi
  WiFi.begin(SSID, PASS);

  // Esperar hasta que la conexión sea exitosa
  while(WiFi.status() != WL_CONNECTED){
    count++;
    delay(1000);
    Serial.print(".");

    if( WiFi.status() == WL_CONNECTED ){
      HayConexion = true;
      break;
    }else{
      if( count == timeout ){
        HayConexion = false;
        break;
      } 
    }
  }

  Serial.print("\n");

  return HayConexion;
}

void EnviarDatosAlServidor(int co2, int temperature, int humidity){
  if( debug ){
    Serial.print("DIRECCION DE ENVIO DE DATOS -> ");
    Serial.println(DIRECCION);
  }

  // Crear un objeto JSON
  DynamicJsonDocument doc(1024);
  doc["co2"] = co2;
  doc["temp"] = temperature;
  doc["hum"] = humidity;

  // Serializar el objeto JSON a una cadena
  String jsonString;
  serializeJson(doc, jsonString);

  // Crear una instancia de HTTPClient
  HTTPClient http;

  // Configurar la dirección del servidor y el punto de enlace
  String serverAddress = String(DIRECCION);
  http.begin(serverAddress);

  // Establecer encabezados
  http.addHeader("Content-Type", "application/json");

  // Realizar la solicitud POST con los datos JSON
  int httpResponseCode = http.POST(jsonString);

  // Verificar la respuesta del servidor
  if (httpResponseCode > 0) {
    Serial.println("> Se han enviado los datos al servidor.");
  } else {
    Serial.print("> Hubo un error en la solicitud al servidor. Código de error: ");
    Serial.println(httpResponseCode);
  }

  // Cerrar la conexión
  http.end();
}