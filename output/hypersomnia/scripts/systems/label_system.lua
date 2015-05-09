label_system = inherits_from (processing_system)

function label_system:constructor(owner_gui)
	self.owner_gui = owner_gui
	
	processing_system.constructor(self)
end

function label_system:draw_damage_indicators(camera_draw_input)
	for i=1, #self.targets do
		local target = self.targets[i]
		local label = target.label
		
		for j=1, #label.text do
			if not label.text[j].font then
				label.text[j].font = label.default_font
			end
		end

		local formatted = format_text(label.text)
		local formatted_black = format_text(label.text)

		for j=0, formatted_black:size()-1 do
			local my_char = formatted_black:at(j)
			
			my_char.r = 0.0;
			my_char.g = 0.0;
			my_char.b = 0.0;
			my_char.a = 255.0;
		end

		if label.position == components.label.positioning.DAMAGE_INDICATOR then
			if not label.indicator_timer then
				label.indicator_timer = timer()
			end

			local screen_pos = target.pos - camera_draw_input.camera_transform.pos		
			
			screen_pos.x = screen_pos.x +0.5*config_table.resolution_w
			screen_pos.y = (screen_pos.y +0.5*config_table.resolution_h) - math.sqrt(label.indicator_timer:get_seconds()) * label.rising_speed


			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				screen_pos + vec2(0, 1),
				0, nil
			)
			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				screen_pos + vec2(0, -1),
				0, nil
			)

			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				screen_pos + vec2(-1, 0),
				0, nil
			)
			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				screen_pos + vec2(1, 0),
				0, nil
			)

			quick_print_text(
				camera_draw_input, 
				formatted, 
				screen_pos,
				0, nil
			)

			if label.indicator_timer:get_seconds() > 0.7 then
				self.owner_entity_system:post_remove(target)
			end
		end
	end
end

function label_system:draw_labels(camera_draw_input)
	for i=1, #self.targets do
		local target = self.targets[i]
		local label = target.label
		
		for j=1, #label.text do
			if not label.text[j].font then
				label.text[j].font = label.default_font
			end
		end
		
		local formatted = format_text(label.text)
		local formatted_black = format_text(label.text)
		
		local outline_color = rgba(0, 0, 0, 255);
		--if target.health then
		--	local target_color = target.health.health_bar_sprite.color
		--	
		--	for j=0, formatted:size()-1 do
		--		local my_char = formatted:at(j)
		--		
		--		my_char.r = target_color.r
		--		my_char.g = target_color.g
		--		my_char.b = target_color.b
		--		my_char.a = target_color.a
		--	end
		--end
		
		if target.health and target.health.last_mult then
			local mult = 1-target.health.last_mult
			--outline_color = rgba(255*(1-mult*mult), 255*mult*mult, 255*mult, 255)
		end

		for j=0, formatted_black:size()-1 do
			local my_char = formatted_black:at(j)
			
			my_char.r = outline_color.r;
			my_char.g = outline_color.g;
			my_char.b = outline_color.b;
			my_char.a = outline_color.a;
		end

		local label_bbox = get_text_bbox(formatted, 0)
		
		if label.position == components.label.positioning.OVER_HEALTH_BAR and target.health and target.health.under_bar_entity.render.was_drawn then
			local label_pos = target.health.under_bar_entity.render.last_screen_pos - vec2(label_bbox.x/2, 5 + label_bbox.y)

			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				label_pos + vec2(0, 1),
				0, nil
			)
			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				label_pos + vec2(0, -1),
				0, nil
			)

			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				label_pos + vec2(-1, 0),
				0, nil
			)
			quick_print_text(
				camera_draw_input, 
				formatted_black, 
				label_pos + vec2(1, 0),
				0, nil
			)

			quick_print_text(
				camera_draw_input, 
				formatted, 
				label_pos,
				0, nil
			)
		end
		
		local overlay = label.messages_overlay
		
		if overlay and target.cpp_entity.render and target.cpp_entity.render.was_drawn then
			local object_pos = target.cpp_entity.render.last_screen_pos
			local still_has_messages, was_removed = overlay.recent_messages:loop()
			
			overlay.group:draw_call(camera_draw_input)
			
			local current_bbox = vec2(overlay.recent_textbox:get_text_bbox())
			local previous_bbox = overlay.previous_bbox
			
			if not previous_bbox then previous_bbox = vec2(0, 0) end
			
			if previous_bbox.x ~= current_bbox.x or previous_bbox.y ~= current_bbox.y then
				overlay.size_w_animator = value_animator(previous_bbox.x, current_bbox.x, 300)
				overlay.size_h_animator = value_animator(previous_bbox.y, current_bbox.y, 300)
				overlay.size_w_animator:set_exponential()
				overlay.size_h_animator:set_exponential()
				overlay.size_w_animator:start()
				overlay.size_h_animator:start()
				
				overlay.update_animator = nil
			end
			
			overlay.previous_bbox = current_bbox
			
			if overlay.size_w_animator and overlay.size_h_animator then
				overlay.recent_textbox:set_area(rect_xywh(object_pos.x+50, object_pos.y - 25, 
					overlay.size_w_animator:get_animated(),  overlay.size_h_animator:get_animated()))
				
				overlay.recent_textbox:set_wrapping_width(350)
			end
			
			if not still_has_messages and overlay.size_w_animator:has_finished() and overlay.size_h_animator:has_finished() then
				label.messages_overlay = nil
			end
		end
	end
end

function label_system:get_required_components()
	return { "label" }
end