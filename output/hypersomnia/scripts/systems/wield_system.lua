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
	if removed_entity.wield.wielded_item ~= nil then
		self.owner_entity_system:remove_entity(removed_entity.wield.wielded_item)
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

components.wield.select_item = function(wielder, new_item)
	local self = wielder.wield
	
	local old_item = self.wielded_item
	self.wielded_item = new_item
	
	if self.on_item_selected then
		print "callback"
		self.on_item_selected(wielder, new_item, old_item)
	end
end

function wield_system:update()
	local msgs = self.owner_entity_system.messages["wield_item"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		local subject = msg.subject
		local wield = subject.wield
		
		-- check subject validity
		if wield ~= nil	then
			local item = msg.item
			
			local previous_item = wield.wielded_item
			
			-- unwielding
			if item == nil then
				msg.succeeded = true
				components.wield.select_item(subject, nil)
			-- we can only wield an unwielded item
			elseif item.item.wielder == nil then
				msg.succeeded = true
				components.wield.select_item(subject, item)
				item.item:set_wielder(subject)
			end
			
			-- we wield only one item at a time
			if msg.succeeded and previous_item then
				previous_item.item:set_wielder(nil)
			end
		end
	end
end
