#include "item_button.h"

void item_button::draw_triangles(draw_info info) {

}

void item_button::consume_gui_event(event_info info) {
	detector.update_appearance(info);
}