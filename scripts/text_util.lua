function format_text(text_entries, append_newline)
	local output = formatted_text()
	
	for i=1, #text_entries do
		local wstr = text_entries[i].wstr
		
		if not wstr then
			wstr = towchar_vec(text_entries[i].str)
		end
		
		
		local color = text_entries[i].color
			
		for j=0, wstr:size()-1 do
			local newchar = create(formatted_char, {
				r = color.r, g = color.g, b = color.b, a = color.a,
				c = wstr:at(j), font_used = text_entries[i].font
			})
			
			output:add(newchar)
			
			if j == wstr:size()-1 and i == #text_entries and append_newline then
				newchar.c = 13
				output:add(newchar)
			end
		end	
	end
	
	return output
end

function wstr_eq(a, b)
	if a:size() ~= b:size() then return false end
	
	for i=0, a:size()-1 do
		if a:at(i) ~= b:at(i) then return false end 
	end
	
	return true
end