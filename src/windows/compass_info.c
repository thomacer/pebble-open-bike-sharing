#include "compass_info.h"

#include "../globals.h"
#include "../libs/pebble-assist.h"

static Layer* s_direction_layer = NULL;
static TextLayer* s_text_layer_calib_state = NULL;
static GPath* s_arrow = NULL;
static GPoint center;

static void direction_handler (CompassHeadingData heading_data) {
    double tmp = 0;
    if (Stations) {
        double tmp = ((double) TRIG_MAX_ANGLE) * ((double) Stations[current_index].angle / 360);
        LOG("COMPASS redraw : %ld and angle %ld (%lf)", heading_data.magnetic_heading, Stations[current_index].angle, tmp);
    }
    gpath_rotate_to(s_arrow, heading_data.magnetic_heading + (int) tmp);

    GRect alert_bounds;
    if(heading_data.compass_status == CompassStatusDataInvalid) {
        // Tell user to move their arm
        /* alert_bounds = GRect(0, 0, bounds.size.w, bounds.size.h); */
        text_layer_set_background_color(s_text_layer_calib_state, GColorBlack);
        text_layer_set_text_color(s_text_layer_calib_state, GColorWhite);
        text_layer_set_font(s_text_layer_calib_state, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
        text_layer_set_text(s_text_layer_calib_state, "Compass is calibrating!\n\nMove your arm to aid calibration.");
    } else if (heading_data.compass_status == CompassStatusCalibrating) {
        // Show status at the top
        /* alert_bounds = GRect(0, -3, bounds.size.w, bounds.size.h / 7); */
        text_layer_set_background_color(s_text_layer_calib_state, GColorClear);
        text_layer_set_text_color(s_text_layer_calib_state, GColorBlack);
        text_layer_set_font(s_text_layer_calib_state, fonts_get_system_font(FONT_KEY_GOTHIC_18));
        text_layer_set_text(s_text_layer_calib_state, "Tuning...");
    }
    text_layer_set_text_alignment(s_text_layer_calib_state, GTextAlignmentCenter);
    /* layer_set_frame(text_layer_get_layer(s_text_layer_calib_state), alert_bounds); */

    layer_mark_dirty(s_direction_layer);
}

static void direction_update_proc(Layer* layer, GContext* ctx) {
    gpath_draw_filled(ctx, s_arrow);
}

static void draw_needle (Layer* window_layer, GRect bounds) {
    /* Layer for the needle update. */
    s_direction_layer = layer_create(bounds);
    layer_set_update_proc(s_direction_layer, direction_update_proc);
    layer_add_child(window_layer, s_direction_layer);
    /* Needle to show the direction. */
    center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
    s_arrow = gpath_create(&INDICATION_ARROW);
    gpath_move_to(s_arrow, center);
}

void destroy_compass() {
    compass_service_unsubscribe();
}

void create_compass(Layer* window_layer, GRect bounds) {
    s_text_layer_calib_state = text_layer_create(GRect(bounds.size.w / 2, (5 * bounds.size.h) / 8, bounds.size.w / 2, bounds.size.h / 4));
    draw_needle(window_layer, bounds);

    compass_service_set_heading_filter(DEG_TO_TRIGANGLE(2));
    compass_service_subscribe(&direction_handler);
}
