#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "";
const char* password = "";
const char* apiEndpoint = "";
const char* s3apiEndpoint = "";

const int ledPin1 = 17;  // LED 1: in stazione/navigazione
const int ledPin2 = 5;   // LED 2: attesa carico
const int ledPin3 = 18;  // LED 3: attesa scarico
const int ledPin4 = 19;  // LED 4: emergenza
const int pulsante1 = 4;  // Pulsante 1: gestisce la logica del sistema
const int pulsante2 = 16; // Pulsante 2: emergenza
const int potenziometro = 34;  // Pin del potenziometro
const int threshold = 4000;    // Valore massimo del potenziometro

DynamicJsonDocument doc(1024);
char currentState[32] = "";

unsigned long previousMillis = 0;
unsigned long ledBlinkInterval = 500;  // Intervallo di lampeggio (500 ms)
bool ledState = LOW;

void setup() {
  
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(pulsante1, INPUT);
  pinMode(pulsante2, INPUT);
  pinMode(potenziometro, INPUT);

  Serial.begin(115200);

  // Connessione Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connessione a Wi-Fi...");
  }
  Serial.println("Connesso a Wi-Fi");

  for(int i = 0; i< 5; i++){
    strcpy(currentState, getLastState().c_str());
    if(strcmp(currentState, "e") != 0) 
      break;
  }
  delay(3000);
  Serial.println("Current state: " + String(currentState));
}

void loop() {
  String response = "";
  const char* newState = "";

  unsigned long currentMillis = millis();

  // Stato "in_stazione" - LED 1 acceso
  if (strcmp(currentState, "in_stazione") == 0) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }

  // Gestione emergenza con Pulsante 2
  if (digitalRead(pulsante2) == 1) {
    Serial.println("Emergenza attivata!");
    strcpy(currentState, "emergenza");
    digitalWrite(ledPin4, HIGH);  // Accendi il LED 4
    sendEventToAPIGateway("emergenza");

  }

  // Stato "emergenza" - LED 4 acceso
  if (strcmp(currentState, "emergenza") == 0) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, HIGH);

    // Se viene premuto il pulsante 1, si ritorna a "in_stazione"
    if (digitalRead(pulsante1) == 1) {
      Serial.println("Uscita dallo stato di emergenza, ritorno a 'in_stazione'");
      response = sendEventToAPIGateway("azione");
      handleApiResponse(response, "in_stazione");
    }
  }

  // Se pulsante premuto in "in_stazione"
  if (digitalRead(pulsante1) == 1 && strcmp(currentState, "in_stazione") == 0) {
    Serial.println("Transizione a 'navigazione'");
    response = sendEventToAPIGateway("azione");
    handleApiResponse(response, "navigazione");
  }

  // Stato "navigazione" con LED 1 lampeggiante
  if (strcmp(currentState, "navigazione") == 0) {
    blinkLed(ledPin1, currentMillis);

    if (digitalRead(pulsante1) == 1) {
      Serial.println("Transizione a 'attesa_carico'");
      response = sendEventToAPIGateway("azione");
      handleApiResponse(response, "attesa_carico");
    }
  }

  // Stato "attesa_carico" - LED 2 lampeggiante
  if (strcmp(currentState, "attesa_carico") == 0) {
    blinkLed(ledPin2, currentMillis);
    digitalWrite(ledPin1, LOW);

    int potValue = analogRead(potenziometro);
    if (potValue >= threshold) {
      Serial.println("Transizione a 'caricato'");
      response = sendEventToAPIGateway("pot_up");
      handleApiResponse(response, "caricato");
    }
  }

  // Stato "caricato" - LED 2 fisso
  if (strcmp(currentState, "caricato") == 0) {
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin3, LOW);

    if (digitalRead(pulsante1) == 1) {
      Serial.println("Transizione a 'attesa_scarico'");
      response = sendEventToAPIGateway("azione");
      handleApiResponse(response, "attesa_scarico");
    }
  }

  // Stato "attesa_scarico" - LED 3 fisso
  if (strcmp(currentState, "attesa_scarico") == 0) {
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);

    int potValue = analogRead(potenziometro);
    if (potValue == 0) {
      Serial.println("Transizione a 'in_stazione'");
      response = sendEventToAPIGateway("pot_down");
      handleApiResponse(response, "in_stazione");
    }
  }
}

void handleApiResponse(String response, const char* expectedState) {
  DeserializationError error = deserializeJson(doc, response);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  const char* newState = doc["new_state"];
  if (strcmp(newState, expectedState) == 0) {
    strcpy(currentState, newState);
    Serial.print("Nuovo stato: ");
    Serial.println(newState);
  } else {
    Serial.println("Lo stato non è cambiato.");
  }
}

String sendEventToAPIGateway(const char* inputEvent) {
  String response = "";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = String(apiEndpoint) + "?input_event=" + inputEvent;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST("");
    if (httpResponseCode > 0) {
      response = http.getString();
      Serial.println("Risposta API: " + response);
    } else {
      Serial.println("Errore POST: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("Non connesso a Wi-Fi");
  }
  return response;
}

void blinkLed(int pin, unsigned long currentMillis) {
  if (currentMillis - previousMillis >= ledBlinkInterval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(pin, ledState);
  }
}

String getLastState(){
  String response = "";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = String(s3apiEndpoint);
    // Inizia la connessione HTTP
    http.begin(url);
    
    // Effettua la richiesta GET
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Alloca il buffer per il documento JSON
        StaticJsonDocument<512> doc;
        
        // Parsing del JSON
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
          // Estrai i valori e salvali nelle variabili
          response = doc["last_log"]["new_state"].as<String>();
          
        } else {
          Serial.print("Errore nel parsing JSON: ");
          Serial.println(error.c_str());
          return "e";
        }
      }
    } else {
      Serial.print("Errore nella richiesta HTTP: ");
      Serial.println(httpCode);
      return "e";
    }
    
    http.end();
  }
  return response;
}
