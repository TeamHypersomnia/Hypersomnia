label_system = inherits_from (processing_system)

function label_system:constructor(owner_gui)
	self.owner_gui = owner_gui
	
	processing_system.constructor(self)
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
		
		if target.health then
			local target_color = target.health.health_bar_sprite.color
			
			for j=0, formatted:size()-1 do
				local my_char = formatted:at(j)
				
				my_char.r = target_color.r
				my_char.g = target_color.g
				my_char.b = target_color.b
				my_char.a = target_color.a
			end
		end
		
		local label_bbox = get_text_bbox(formatted, 0)
		
		if label.position == components.label.positioning.OVER_HEALTH_BAR and target.health and target.health.under_bar_entity.render.was_drawn then
			quick_print_text(
				camera_draw_input, 
				formatted, 
				target.health.under_bar_entity.render.last_screen_pos - vec2(label_bbox.x/2, 10 + label_bbox.y),
				0, nil
			)
		end
	end
end

function label_system:get_required_components()
	return { "label" }
end