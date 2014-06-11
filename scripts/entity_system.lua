entity_system = inherits_from {}

function entity_system:constructor(owner_world)
	self.entity_table = {}
	self.message_table = {}
	
	self.message_table[scriptable_component.DAMAGE_MESSAGE] = {}
	self.message_table[scriptable_component.INTENT_MESSAGE] = {}
	 
	self.entity_basic_scriptable_info = create_scriptable_info {
		scripted_events = {
			[scriptable_component.LOOP] = function (subject, is_substepping)
				local my_self = get_self(subject)
				
				if is_substepping then
					my_self:substep()
				else
					my_self:loop()
				end
			end,
			
			[scriptable_component.DAMAGE_MESSAGE] = function (message)
				table.insert(self.message_table[scriptable_component.DAMAGE_MESSAGE], message)
			end,
			
			[scriptable_component.INTENT_MESSAGE] = function (message)
				table.insert(self.message_table[scriptable_component.INTENT_MESSAGE], message)
			end
		}
	}
	 
	-- user variables
	self.enable_loop = true
	self.registered_modules = {}
	
	self.loop_script_info = create_scriptable_info {
		scripted_events = {
			[scriptable_component.LOOP] = function(subject, is_substepping)	
				local this = get_self(subject)
				
				if this.enable_loop then
					this:tick(is_substepping, this.registered_modules)
				end
			end
		}
	}
	
	self.loop_script_entity = owner_world:create_entity {
		scriptable = {
			available_scripts = self.loop_script_info,
			script_data = self
		}
	}
	
	self.owner_world = owner_world
end

-- merely generates an entity object inside scriptable
function entity_system:create_entity_table(what_entity, what_class, ...)
	if what_class == nil then what_class = entity_class end
	
	-- if scriptable component does not exist, add a new one
	if what_entity:get().scriptable == nil then what_entity:get():add(scriptable_component()) end
	--group_table = override( { body = { scriptable = { } } }, group_table )
	
	--print (table.inspect(group_table))
	--local my_new_entity_group = ptr_create_entity_group (group_table)
	local new_entity_script_data = what_class:create(...)
	
	new_entity_script_data.parent_entity = what_entity
	new_entity_script_data.parent_group = self.owner_world:get_group_by_entity(what_entity:get())
	
	-- there is no need to override the basic scriptable info as it provides entities with all needed functionality
	-- and in fact should not be modified
	--local new_scriptable_info = create_scriptable_info (scriptable_table)
	
	what_entity:get().scriptable.script_data = new_entity_script_data
	what_entity:get().scriptable.available_scripts = self.entity_basic_scriptable_info
	
	table.insert(self.entity_table, new_entity_script_data)

	return new_entity_script_data
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