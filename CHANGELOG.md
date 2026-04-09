# Changelog

Todas as mudanças notáveis neste projeto serão documentadas neste arquivo.

O formato é baseado em [Keep a Changelog](https://keepachangelog.com/pt-BR/1.1.0/),
e este projeto adere ao [Versionamento Semântico](https://semver.org/lang/pt-BR/).

## [Unreleased]

## [0.1.2] - 2026-04-08

### Alterado

- Estrutura do projeto movida de `neuronio-aux/` para a raiz do repositório.
- Board trocado para `blips_esp32s3` customizado com tabela de partições `2_otas.csv`.
- Adicionado `ArduinoJson` como dependência explícita no `platformio.ini`.
- Ignoradas libs incompatíveis com ESP32-S3 (`RPAsyncTCP`, `ESPAsyncTCP`).
- Substituído `containsKey()` por `doc["key"].is<T>()` (API ArduinoJson v7).
- `PIN_BLOCK` (GPIO 4) alterado de `INPUT_PULLUP` para `INPUT_PULLDOWN`.
- Polling no `loop()` agora roda a cada 10ms em vez de tight loop.

## [0.1.1] - 2026-04-08

### Alterado

- Substituído uso de `String` por `char[]` + `snprintf` em todos os handlers HTTP para evitar fragmentação de heap.
- Tamanho dos arrays `pinState`/`pinTimestamp` derivado de constante `NUM_PINS` em vez de magic number.
- Mensagem de erro de pin inválido agora reflete os valores das constantes dinamicamente.

## [0.1.0] - 2026-04-08

### Adicionado

- Firmware inicial do módulo auxiliar (neuronio-aux) para ESP32-S3.
- Conexão WiFi em modo STA com reconexão automática.
- Descoberta via mDNS (`neuronio-aux.local`).
- Servidor HTTP assíncrono na porta 80.
- Endpoint `GET /gpio/read?pin=N` — leitura dos GPIOs 1 (MEM2) e 4 (BLOCK).
- Endpoint `POST /gpio/write` — escrita no GPIO 1 (MEM2) via JSON.
- Endpoint `GET /status` — heap livre e uptime.
- Monitoramento contínuo de mudança de estado no GPIO 4 (BLOCK).
- Configuração PlatformIO para `esp32-s3-devkitc-1`.
