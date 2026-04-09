#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// ── Configuracao WiFi ──────────────────────────────────────────────
static const char *WIFI_SSID     = "POCOF5";
static const char *WIFI_PASSWORD = "12345678";

// ── mDNS ───────────────────────────────────────────────────────────
static const char *MDNS_HOSTNAME = "neuronio-aux";

// ── GPIOs ──────────────────────────────────────────────────────────
static const int PIN_MEM2  = 1;   // OUTPUT  — sinal de uso/telemetria
static const int PIN_BLOCK = 4;   // INPUT_PULLDOWN — bloqueio do DUT

// ── Estado dos pinos ───────────────────────────────────────────────
static const int NUM_PINS = PIN_BLOCK + 1;
static int           pinState[NUM_PINS];
static unsigned long pinTimestamp[NUM_PINS];

AsyncWebServer server(80);

// ── Helpers ────────────────────────────────────────────────────────

static void sendJson(AsyncWebServerRequest *request, int code, const char *json) {
    request->send(code, "application/json", json);
}

static void sendError(AsyncWebServerRequest *request, int code, const char *msg) {
    char buf[128];
    snprintf(buf, sizeof(buf), "{\"error\":\"%s\"}", msg);
    sendJson(request, code, buf);
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
            char err[64];
            snprintf(err, sizeof(err), "pin invalido, use %d ou %d", PIN_MEM2, PIN_BLOCK);
            sendError(request, 400, err);
            return;
        }
        char json[96];
        snprintf(json, sizeof(json),
                 "{\"pin\":%d,\"value\":%d,\"timestamp_ms\":%lu}",
                 pin, pinState[pin], pinTimestamp[pin]);
        sendJson(request, 200, json);
    });

    // POST /gpio/write  —  body: {"pin":N,"value":0|1}
    server.on("/gpio/write", HTTP_POST,
        [](AsyncWebServerRequest *request) {
            if (!request->contentType().startsWith("application/json")) {
                sendError(request, 400, "Content-Type: application/json obrigatorio");
            }
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
            if (!doc["pin"].is<int>() || !doc["value"].is<int>()) {
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
            char json[48];
            snprintf(json, sizeof(json), "{\"pin\":%d,\"value\":%d}", pin, val);
            sendJson(request, 200, json);
        }
    );

    // GET /status
    server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        char json[64];
        snprintf(json, sizeof(json),
                 "{\"heap\":%u,\"uptime_ms\":%lu}",
                 ESP.getFreeHeap(), millis());
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
    pinMode(PIN_BLOCK, INPUT_PULLDOWN);

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
    // Monitora mudanca de estado no GPIO4 (polling a cada 10ms)
    static unsigned long lastPoll = 0;
    if (millis() - lastPoll >= 10) {
        lastPoll = millis();
        int current = digitalRead(PIN_BLOCK);
        if (current != pinState[PIN_BLOCK]) {
            pinState[PIN_BLOCK]    = current;
            pinTimestamp[PIN_BLOCK] = millis();
        }
    }
}
