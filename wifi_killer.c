#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <furi_hal_gpio.h>

static bool running = true;
static const GpioPin* trigger_pin = &gpio_pin_15;

static void input_callback(InputEvent* input, void* ctx) {
    UNUSED(ctx);
    if(input->type == InputTypePress && input->key == InputKeyBack) {
        running = false;
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 5, 20, "WiFi Killer 100m");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 5, 40, "KILLING ALL WIFI");
    canvas_draw_str(canvas, 5, 55, "Press BACK to stop");
    canvas_draw_str(canvas, 5, 70, "GPIO Pin 15: HIGH");
}

int32_t wifi_killer_app(void* p) {
    UNUSED(p);
    
    // Set pin as output, default LOW
    furi_hal_gpio_init(trigger_pin, GpioModeOutputPushPull, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_write(trigger_pin, false);
    
    // GUI
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Set pin HIGH - trigger ESP32 to start killing
    furi_hal_gpio_write(trigger_pin, true);
    furi_delay_ms(100);
    
    // Main loop - wait for BACK
    while(running) {
        furi_delay_ms(50);
    }
    
    // Set pin LOW - stop ESP32
    furi_hal_gpio_write(trigger_pin, false);
    
    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    return 0;
}
