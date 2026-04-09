#ifndef GPIO_LOGIC_H
#define GPIO_LOGIC_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ── Pin constants ──────────────────────────────────────────────────
#define PIN_MEM2   1
#define PIN_BLOCK  4
#define NUM_PINS   (PIN_BLOCK + 1)

// ── Pin validation ─────────────────────────────────────────────────
bool pin_is_valid(int pin);
bool pin_is_writable(int pin);

// ── Pin state management ───────────────────────────────────────────
typedef struct {
    int           value[NUM_PINS];
    unsigned long timestamp_ms[NUM_PINS];
} PinStateStore;

void pin_state_init(PinStateStore *store);
bool pin_state_update(PinStateStore *store, int pin, int new_value, unsigned long now_ms);
void pin_state_set(PinStateStore *store, int pin, int value, unsigned long now_ms);

// ── JSON formatters (return bytes needed, excl. NUL) ───────────────
int fmt_json_read(char *buf, size_t buf_size, int pin, int value, unsigned long timestamp_ms);
int fmt_json_write(char *buf, size_t buf_size, int pin, int value);
int fmt_json_error(char *buf, size_t buf_size, const char *msg);
int fmt_json_status(char *buf, size_t buf_size, unsigned int heap, unsigned long uptime_ms);

// ── Write-request validation ───────────────────────────────────────
typedef enum {
    WRITE_OK = 0,
    WRITE_ERR_PIN_MISSING,
    WRITE_ERR_VALUE_MISSING,
    WRITE_ERR_PIN_NOT_WRITABLE
} WriteValidation;

WriteValidation validate_write_params(bool has_pin, bool has_value, int pin);

#ifdef __cplusplus
}
#endif

#endif // GPIO_LOGIC_H
