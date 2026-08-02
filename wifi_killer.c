#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

static bool running = true;

static void input_callback(InputEvent* input, void* ctx) {
    UNUSED(ctx);
    if(input->key == InputKeyBack && input->type == InputTypePress) {
        running = false;
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_draw_str(canvas, 5, 20, "Hello World");
    canvas_draw_str(canvas, 5, 40, "Press BACK to exit");
}

int32_t wifi_killer_app(void* p) {
    UNUSED(p);
    
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    while(running) {
        furi_delay_ms(50);
    }
    
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    return 0;
}
