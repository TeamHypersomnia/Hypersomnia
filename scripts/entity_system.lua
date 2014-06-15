entity_system = inherits_from {}

function entity_system:constructor(owner_world) 
	-- user variables
	self.enable_loop = true
	self.registered_modules = {}
	
	self.owner_world = owner_world
end

function entity_system:create_entity_table(what_entity, what_class, ...)
	if what_class == nil then what_class = entity_class end
	
	-- if scriptable component does not exist, add a new one
	if what_entity:get().scriptable == nil then what_entity:get():add(scriptable_component()) end
	--group_table = override( { body = { scriptable = { } } }, group_table )
	
	--print (table.inspect(group_table))
	--local my_new_entity_group = ptr_create_entity_group (group_table)
	local new_entity_table = what_class:create(...)
	
	new_entity_table.parent_entity = what_entity
	new_entity_table.parent_group = self.owner_world:get_group_by_entity(what_entity:get())
	
	what_entity:get().scriptable.script_data = new_entity_table

	return new_entity_table
end

function entity_system:process_all_entities(callback)
	local entities = self.owner_world.script_system:get_entities_vector()

	for i=0, entities:size()-1 do
		callback(entities:at(i))
	end
end

function entity_system:process_all_entity_modules(module_name, method_name, ...)
	local entities = self.owner_world.script_system:get_entities_vector()
	
	for i=0, entities:size()-1 do
		entities:at(i):try_module_method(module_name, method_name, ...)
	end
end

function entity_system:handle_messages(world, is_substepping)
	local message_names = {
		"damage_message",
		"intent_message",
		"collision_message",
		"shot_message",
		"destroy_message"
	}
	
	-- send all events to entities globally
	for i=1, #message_names do
		local message_vector = _G["get_" .. message_names[i] .. "_queue"](self.owner_world.world)
		
		if message_vector:size() > 0 then
			for j=0, message_vector:size()-1 do
				local msg = message_vector:at(j)
				
				local entity_self = get_self(msg.subject)
		
				local message_callback = entity_self[message_names[i]]
				
				if message_callback  ~= nil then
					message_callback (entity_self, msg)
				end
				
				-- by the way, send every single event to all interested modules of this entity respectively
				entity_self:all_modules(message_names[i], msg)
			end
		end
	end
	
end

function entity_system:tick(method_name)
	self:process_all_entities(
	function(ent)
		ent[method_name](ent)
	
		for i=1, #self.registered_modules do
			ent:try_module_method(self.registered_modules[i], method_name)
		end
	end
	)
end