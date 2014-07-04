sync_modules.crosshair = inherits_from ()

function sync_modules.crosshair:constructor()
	self.position = vec2(1, 1)
	
	self.pos_sig = {
		"Vec2", "position"
	}
 end

function sync_modules.crosshair:read_stream(input)
	protocol.read_sig(self.pos_sig, self, input) 
	--print ("reading crosshair.." .. self.crosshair_pos.x .. "|" .. self.crosshair_pos.y )
	
	--print (input.read_report) 
end

function sync_modules.crosshair:read_state(input) end
function sync_modules.crosshair:update_game_object(object) end

-- SERVER CODE
function sync_modules.crosshair:write_stream(object, output)
	protocol.write_sig(self.pos_sig, self, output)
end

function sync_modules.crosshair:write_state(output) end
