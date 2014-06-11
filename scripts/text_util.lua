function format_text(text_entries)
	local output = formatted_text()
	
	for i=1, #text_entries do
		local wstr = towchar_vec(text_entries[i].str)
		local color = text_entries[i].col
			
		for j=0, wstr:size()-1 do
			local newchar = create(formatted_char, {
				r = color.r, g = color.g, b = color.b, a = color.a,
				c = wstr:at(j), font_used = text_entries[i].font
			})
			
			output:add(newchar)
		end	
	end
	
	return output
end