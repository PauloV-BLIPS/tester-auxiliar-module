#include "gpio_logic.h"
#include <stdio.h>
#include <string.h>

bool pin_is_valid(int pin) {
    return (pin == PIN_MEM2 || pin == PIN_BLOCK);
}

bool pin_is_writable(int pin) {
    return (pin == PIN_MEM2);
}

void pin_state_init(PinStateStore *store) {
    memset(store, 0, sizeof(*store));
}

bool pin_state_update(PinStateStore *store, int pin, int new_value, unsigned long now_ms) {
    if (pin < 0 || pin >= NUM_PINS) return false;
    if (store->value[pin] == new_value) return false;
    store->value[pin] = new_value;
    store->timestamp_ms[pin] = now_ms;
    return true;
}

void pin_state_set(PinStateStore *store, int pin, int value, unsigned long now_ms) {
    if (pin < 0 || pin >= NUM_PINS) return;
    store->value[pin] = value;
    store->timestamp_ms[pin] = now_ms;
}

int fmt_json_read(char *buf, size_t buf_size, int pin, int value, unsigned long timestamp_ms) {
    return snprintf(buf, buf_size,
        "{\"pin\":%d,\"value\":%d,\"timestamp_ms\":%lu}",
        pin, value, timestamp_ms);
}

int fmt_json_write(char *buf, size_t buf_size, int pin, int value) {
    return snprintf(buf, buf_size,
        "{\"pin\":%d,\"value\":%d}", pin, value);
}

int fmt_json_error(char *buf, size_t buf_size, const char *msg) {
    return snprintf(buf, buf_size,
        "{\"error\":\"%s\"}", msg);
}

int fmt_json_status(char *buf, size_t buf_size, unsigned int heap, unsigned long uptime_ms) {
    return snprintf(buf, buf_size,
        "{\"heap\":%u,\"uptime_ms\":%lu}",
        heap, uptime_ms);
}

WriteValidation validate_write_params(bool has_pin, bool has_value, int pin) {
    if (!has_pin)              return WRITE_ERR_PIN_MISSING;
    if (!has_value)            return WRITE_ERR_VALUE_MISSING;
    if (!pin_is_writable(pin)) return WRITE_ERR_PIN_NOT_WRITABLE;
    return WRITE_OK;
}
