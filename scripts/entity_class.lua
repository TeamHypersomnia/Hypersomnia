entity_class = inherits_from {}

-- to be called exclusively by entity_system:create_entity_table
function entity_class:constructor() end

function get_self(entity)
	return entity.scriptable.script_data
end

-- some specific entity may want to provide its own behaviour besides the basic process_all_entity_modules functions
-- in such a case these two functions should be called
function entity_class:loop()
	--self:all_modules("loop")
end

function entity_class:substep()
	--self:all_modules("substep")
	--print "substepping unoverrided character loop func"
end

-- perform a given method on all modules (components) of this entity
-- note: most of the components in scripts will have component:system 1:1 relationship
-- so it is an overkill to create separate system classes
-- I am calling these "modules" to distinguish them from basic components in my C++ core
function entity_class:all_modules(method, ...)
	for k, v in pairs(self) do
		if type(self[k]) == "table" and type(self[k][method]) == "function" then
			self[k][method](self[k], ...)
		end
	end
end

function entity_class:try_module_method(module_name, method_name, ...)
	-- do not process only if it is explicitly disabled,
	-- i.e. if no "enabled" flag was set for this module, or if it was set to true, then process it
	local m = self[module_name]
	
	if 
	-- this module exists
	m ~= nil
		and 
	-- this function in this module exists
	m[method_name] ~= nil 
		and
	-- module not explicitly disabled
	(m.enabled == nil or m.enabled == true) 
		then
	-- call method
		m[method_name](m, ...)
	end
end
