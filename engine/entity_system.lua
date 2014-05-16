entity_system = inherits_from {}

function entity_system:constructor()
	self.entity_table = {}
	self.scriptables_table = {}
	self.message_table = {}
	
	self.message_table[scriptable_component.DAMAGE_MESSAGE] = {}
	self.message_table[scriptable_component.INTENT_MESSAGE] = {}
end

function entity_system:process_all_entities(callback)
	for i=1, #self.entity_table do
		callback(self.entity_table[i])
	end
end

function entity_system:process_all_entity_modules(module_name, method_name, ...)
	for i=1, #self.entity_table do
		self.entity_table[i]:try_module_method(module_name, method_name, ...)
	end
end

function entity_system:flush_message_tables()
	for k, v in pairs(self.message_table) do
		self.message_table[k] = {}
	end
end

function entity_system:tick(is_substepping, module_names)
	-- process entities
	
	local method_name = "loop"
	
	if is_substepping then
		method_name = "substep"
	end
	
	self:process_all_entities(
	function(e)
		for i=1, #module_names do
			e:try_module_method(module_names[i], method_name)
		end
	end
	)
	
	local name_map = {
		[scriptable_component.DAMAGE_MESSAGE] = "damage_message",
		[scriptable_component.INTENT_MESSAGE] = "intent_message"
	}
	
	-- send all events to entities globally
	for msg_key, msg_table in pairs(self.message_table) do
		for k, msg in pairs(self.message_table[msg_key]) do
			local entity_self = get_self(msg.subject)
			
			-- callback
			local callback = entity_self[name_map[msg_key]]
			
			if callback ~= nil then
				callback(entity_self, msg)
			end
			
			-- by the way, send every single event to all interested modules of this entity respectively
			entity_self:all_modules(name_map[msg_key], msg)
		end
	end
	
	-- messages processed, clear tables
	self:flush_message_tables()
end