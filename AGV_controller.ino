#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = ""; //SSID del wifi
const char* password = ""; //Password del wifi 
const char* apiEndpoint = ""; //inserire l'endpoint AWS

const int ledPin1 = 17;  // LED 1: in stazione/navigazione
const int ledPin2 = 5;   // LED 2: attesa carico
const int ledPin3 = 18;  // LED 3: attesa scarico
const int ledPin4 = 19;  // LED 4: emergenza
const int pulsante1 = 4;  // Pulsante 1: gestisce la logica del sistema
const int pulsante2 = 16;   // Pulsante 2: emergenza
const int potenziometro = 34;  // Pin del potenziometro
const int threshold = 4000;    // Valore massimo del potenziometro

DynamicJsonDocument doc(1024);
char previousState[32] = "in_stazione";  // Stato iniziale

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
}

void loop() {
  String response = "";
  const char* newState = "";
  
  // Stato "in_stazione" - LED 1 acceso
  if (strcmp(previousState, "in_stazione") == 0) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, LOW);
  }

  // Gestione emergenza con Pulsante 2
  if (digitalRead(pulsante2) == 1) {
    Serial.println("Emergenza attivata!");
    strcpy(previousState, "emergenza");
    digitalWrite(ledPin4, HIGH);  // Accendi il LED 4
    sendEventToAPIGateway("in_stazione", "emergenza");
    delay(1000);  // Evita rimbalzi
  }

  // Stato "emergenza" - LED 4 acceso
  if (strcmp(previousState, "emergenza") == 0) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin3, LOW);
    digitalWrite(ledPin4, HIGH);

    // Se viene premuto il pulsante 1, si ritorna a "in_stazione"
    if (digitalRead(pulsante1) == 1) {
      Serial.println("Uscita dallo stato di emergenza, ritorno a 'in_stazione'");
      response = sendEventToAPIGateway("emergenza", "azione");
      handleApiResponse(response, "in_stazione");
    }
  }

  // Se pulsante premuto in "in_stazione"
  if (digitalRead(pulsante1) == 1 && strcmp(previousState, "in_stazione") == 0) {
    Serial.println("Transizione a 'navigazione'");
    response = sendEventToAPIGateway(previousState, "azione");
    handleApiResponse(response, "navigazione");
  }

  // Stato "navigazione" con LED 1 lampeggiante
  if (strcmp(previousState, "navigazione") == 0) {
    blinkLed(ledPin1);

    if (digitalRead(pulsante1) == 1) {
      Serial.println("Transizione a 'attesa_carico'");
      response = sendEventToAPIGateway(previousState, "azione");
      handleApiResponse(response, "attesa_carico");
    }
  }

  // Stato "attesa_carico" - LED 2 lampeggiante
  if (strcmp(previousState, "attesa_carico") == 0) {
    blinkLed(ledPin2);
    digitalWrite(ledPin1, LOW);

    int potValue = analogRead(potenziometro);
    Serial.println(potValue);
    if (potValue >= threshold) {
      Serial.println("Transizione a 'caricato'");
      response = sendEventToAPIGateway(previousState, "pot_up");
      handleApiResponse(response, "caricato");
    }
  }

  // Stato "caricato" - LED 2 fisso
  if (strcmp(previousState, "caricato") == 0) {
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin3, LOW);

    if (digitalRead(pulsante1) == 1) {
      Serial.println("Transizione a 'attesa_scarico'");
      response = sendEventToAPIGateway(previousState, "azione");
      handleApiResponse(response, "attesa_scarico");
    }
  }

  // Stato "attesa_scarico" - LED 3 fisso
  if (strcmp(previousState, "attesa_scarico") == 0) {
    digitalWrite(ledPin3, HIGH);
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);

    int potValue = analogRead(potenziometro);
    if (potValue == 0) {
      Serial.println("Transizione a 'in_stazione'");
      response = sendEventToAPIGateway(previousState, "pot_down");
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
    strcpy(previousState, newState);
    Serial.print("Nuovo stato: ");
    Serial.println(newState);
  } else {
    Serial.println("Lo stato non è cambiato.");
  }
}

String sendEventToAPIGateway(const char* currentState, const char* inputEvent) {
  String response = "";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = String(apiEndpoint) + "?current_state=" + currentState + "&input_event=" + inputEvent;
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

void blinkLed(int pin) {
  digitalWrite(pin, HIGH);
  delay(500);
  digitalWrite(pin, LOW);
  delay(500);
}
