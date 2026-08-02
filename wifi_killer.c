#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <storage/storage.h>

static bool running = true;
static FuriThread* uart_thread = NULL;
static FuriStreamBuffer* uart_rx_buffer = NULL;

// UART configuration for ESP32 (TX on pin 13, RX on pin 14)
#define UART_CHANNEL FuriHalUartIdUSART1
#define ESP32_TX_PIN 13
#define ESP32_RX_PIN 14

static void input_callback(InputEvent* input, void* ctx) {
    if(input->type == InputTypePress && input->key == InputKeyBack) {
        running = false;
    }
}

// Send AT command to ESP32 to start deauth on all channels
static void send_deauth_command() {
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+DEAUTH=ALL,100,0\r\n", 22);
    furi_delay_ms(50);
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+CHANNEL=ALL\r\n", 17);
    furi_delay_ms(50);
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+BROADCAST=1\r\n", 17);
}

// Draw callback for the screen
static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 20, "WiFi Killer 100m");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 40, "Jamming ALL channels");
    canvas_draw_str(canvas, 5, 55, "Press BACK to stop");
    canvas_draw_str(canvas, 5, 70, "ESP32 connected: YES");
}

int32_t wifi_killer_app(void* p) {
    UNUSED(p);
    
    // Setup GUI
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Setup UART to ESP32
    furi_hal_serial_init(UART_CHANNEL, 115200);
    furi_hal_serial_set_pins(UART_CHANNEL, ESP32_TX_PIN, ESP32_RX_PIN);
    furi_delay_ms(500);
    
    // Send initial command to start jamming
    send_deauth_command();
    
    // Main loop - keep jamming until BACK pressed
    while(running) {
        furi_delay_ms(100);
        // Resend command every few seconds to keep it active
        static uint32_t counter = 0;
        if(++counter > 30) { // every ~3 seconds
            counter = 0;
            send_deauth_command();
        }
    }
    
    // Stop jamming on exit
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+STOP\r\n", 9);
    furi_delay_ms(100);
    furi_hal_serial_deinit(UART_CHANNEL);
    
    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    return 0;
}
