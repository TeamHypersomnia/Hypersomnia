wield_system = inherits_from (processing_system)

--function wield_system:constructor(world_object)
--	self.world_object = world_object
--	processing_system.constructor(self)
--end

--function wield_system:add_entity(new_entity)
--	if new_entity.replication ~= nil then
--		new_entity.replication.
--	end
--end

function wield_system:remove_entity(removed_entity)
	-- if we wield an item, we may need to drop it
	
	-- THIS IS THE GAME LOGIC THAT SHOULD DECIDE IF THE OBJECT IS TO BE DROPPED,
	-- AND IF SO, IT SHOULD POST A DROP MESSAGE BEFORE POSTING DELETION OF THE OBJECT
	
	-- IF THE ITEM ISN'T DROPPED, IT IS SIMPLY DELETED
	for k, v in pairs(removed_entity.wield.wielded_items) do
		self.owner_entity_system:remove_entity(v)
	end
	
	--processing_system.remove_entity(self, removed_entity)
end
--
function wield_system:get_required_components()
	return { "wield" }
end

function wield_system:send_pick_requests(world_object)
	--local msgs = world_object:get_messages_filter_components("intent_message", { "wield" } )
	--
	--for i=1, #msgs do
	--	if msgs[i].state_flag and msgs[i].intent == custom_intents.PICK_REQUEST then
	--		local client_sys = self.owner_entity_system.all_systems["client"]
	--	
	--		client_sys.net_channel:post_reliable("PICK_REQUEST", {})
	--	end
	--end
end

function wield_system:receive_item_selections()
	--local msgs = self.owner_entity_system.messages["ITEM_DROPPED"]
	--local replication = self.owner_entity_system.all_systems["replication"]
	--
	--for i=1, #msgs do
	--	local msg = msgs[i]
	--	
	--	self.owner_entity_system:post_table("drop_item", {
	--		subject = replication.object_by_id[msg.data.subject_id],
	--		drop = true
	--	})
	--end
	--
	--msgs = self.owner_entity_system.messages["ITEM_PICKED"]
	--
	--for i=1, #msgs do
	--	local msg = msgs[i]
	--	
	--	self.owner_entity_system:post_table("wield_item", {
	--		subject = replication.object_by_id[msg.data.subject_id],
	--		item = replication.object_by_id[msg.data.item_id],
	--		pick = true
	--	})
	--end
end

components.wield.wield_item = function(wielder, new_item, wielding_key)
	local self = wielder.wield
	
	local old_item = self.wielded_items[wielding_key]
	self.wielded_items[wielding_key] = new_item
	new_item.item.wielding_key = wielding_key
	
	if old_item then
		old_item.item:set_wielder(nil)
	end
	
	if self.on_item_wielded then
		self.on_item_wielded(wielder, new_item, old_item, wielding_key)
	end
end

components.wield.unwield_item = function(wielder, old_item, wielding_key_hint)
	local found = false
	local self = wielder.wield
	
	if wielding_key_hint ~= nil then
		old_item = self.wielded_items[wielding_key_hint]
		
		if old_item then
			found = true
		end
		
		self.wielded_items[wielding_key_hint] = nil
	else
		for k, v in pairs(self.wielded_items) do
			if v == old_item then
				wielding_key_hint = k
				self.wielded_items[k] = nil
				found = true
				break
			end
		end
	end
	
	if found and self.on_item_unwielded then
		self.on_item_unwielded(wielder, old_item, wielding_key_hint)
	end
	
	return found
end

function wield_system:update()
	local msgs = self.owner_entity_system.messages["unwield_item"]
	-- subject
	-- item OR wielding_key
	
	for i=1, #msgs do
		local msg = msgs[i]
		local subject = msg.subject
		
		-- check subject validity
		if subject.wield ~= nil	then
			local item = components.wield.unwield_item(subject, msg.item, msg.wielding_key)

			if item then
				msg.succeeded = true
				msg.item = item
				item.item:set_wielder(nil)
			end
		end
	end
	
	msgs = self.owner_entity_system.messages["wield_item"]
	-- subject
	-- item 
	-- wielding_key
	
	for i=1, #msgs do
		local msg = msgs[i]
		local subject = msg.subject
		
		-- check subject validity
		if subject.wield ~= nil	then
			local item = msg.item
			
			if item.item.wielder == nil then
				msg.succeeded = true
				print "wielding!"
				components.wield.wield_item(subject, msg.item, msg.wielding_key)
				item.item:set_wielder(subject)
			end
		end
	end
end
