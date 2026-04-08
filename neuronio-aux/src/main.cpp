#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// ── Configuracao WiFi ──────────────────────────────────────────────
static const char *WIFI_SSID     = "SEU_SSID";
static const char *WIFI_PASSWORD = "SUA_SENHA";

// ── mDNS ───────────────────────────────────────────────────────────
static const char *MDNS_HOSTNAME = "neuronio-aux";

// ── GPIOs ──────────────────────────────────────────────────────────
static const int PIN_MEM2  = 1;   // OUTPUT  — sinal de uso/telemetria
static const int PIN_BLOCK = 4;   // INPUT_PULLUP — bloqueio do DUT

// ── Estado dos pinos ───────────────────────────────────────────────
static int      pinState[5];          // indice = numero do pino
static unsigned long pinTimestamp[5];  // millis() da ultima mudanca

AsyncWebServer server(80);

// ── Helpers ────────────────────────────────────────────────────────

static void sendJson(AsyncWebServerRequest *request, int code, const String &json) {
    request->send(code, "application/json", json);
}

static void sendError(AsyncWebServerRequest *request, int code, const char *msg) {
    String json = "{\"error\":\"";
    json += msg;
    json += "\"}";
    sendJson(request, code, json);
}

// ── Setup WiFi ─────────────────────────────────────────────────────

static void setupWiFi() {
    Serial.printf("Conectando a %s", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.printf("\nConectado! IP: %s\n", WiFi.localIP().toString().c_str());
}

// ── Setup mDNS ─────────────────────────────────────────────────────

static void setupMDNS() {
    if (MDNS.begin(MDNS_HOSTNAME)) {
        Serial.printf("mDNS: %s.local\n", MDNS_HOSTNAME);
        MDNS.addService("http", "tcp", 80);
    } else {
        Serial.println("Erro ao iniciar mDNS");
    }
}

// ── Setup Endpoints ────────────────────────────────────────────────

static void setupServer() {

    // GET /gpio/read?pin=N
    server.on("/gpio/read", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("pin")) {
            sendError(request, 400, "parametro 'pin' ausente");
            return;
        }
        int pin = request->getParam("pin")->value().toInt();
        if (pin != PIN_MEM2 && pin != PIN_BLOCK) {
            sendError(request, 400, "pin invalido, use 1 ou 4");
            return;
        }
        int val = digitalRead(pin);
        String json = "{\"pin\":" + String(pin)
                    + ",\"value\":" + String(val)
                    + ",\"timestamp_ms\":" + String(pinTimestamp[pin])
                    + "}";
        sendJson(request, 200, json);
    });

    // POST /gpio/write  —  body: {"pin":N,"value":0|1}
    server.on("/gpio/write", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            // resposta enviada no onBody
        },
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len,
           size_t /*index*/, size_t /*total*/) {
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);
            if (err) {
                sendError(request, 400, "JSON invalido");
                return;
            }
            if (!doc.containsKey("pin") || !doc.containsKey("value")) {
                sendError(request, 400, "campos 'pin' e 'value' obrigatorios");
                return;
            }
            int pin = doc["pin"].as<int>();
            if (pin != PIN_MEM2) {
                sendError(request, 400, "apenas pin 1 e gravavel");
                return;
            }
            int val = doc["value"].as<int>() ? 1 : 0;
            digitalWrite(pin, val);
            pinState[pin]     = val;
            pinTimestamp[pin]  = millis();
            String json = "{\"pin\":" + String(pin)
                        + ",\"value\":" + String(val)
                        + "}";
            sendJson(request, 200, json);
        }
    );

    // GET /status
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{\"heap\":" + String(ESP.getFreeHeap())
                    + ",\"uptime_ms\":" + String(millis())
                    + "}";
        sendJson(request, 200, json);
    });

    // Qualquer outra rota → 404
    server.onNotFound([](AsyncWebServerRequest *request) {
        sendError(request, 404, "endpoint nao encontrado");
    });

    server.begin();
    Serial.println("Servidor HTTP iniciado na porta 80");
}

// ── Arduino setup / loop ───────────────────────────────────────────

void setup() {
    Serial.begin(115200);
    delay(500);

    // GPIOs
    pinMode(PIN_MEM2, OUTPUT);
    digitalWrite(PIN_MEM2, LOW);
    pinMode(PIN_BLOCK, INPUT_PULLUP);

    // Estado inicial
    pinState[PIN_MEM2]      = LOW;
    pinTimestamp[PIN_MEM2]   = millis();
    pinState[PIN_BLOCK]     = digitalRead(PIN_BLOCK);
    pinTimestamp[PIN_BLOCK]  = millis();

    setupWiFi();
    setupMDNS();
    setupServer();
}

void loop() {
    // Monitora mudanca de estado no GPIO4
    int current = digitalRead(PIN_BLOCK);
    if (current != pinState[PIN_BLOCK]) {
        pinState[PIN_BLOCK]    = current;
        pinTimestamp[PIN_BLOCK] = millis();
    }
}
