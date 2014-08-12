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
	local msgs = world_object:get_messages_filter_components("intent_message", { "wield" } )
	
	for i=1, #msgs do
		if msgs[i].state_flag and msgs[i].intent == custom_intents.PICK_REQUEST then
			local client_sys = self.owner_entity_system.all_systems["client"]
		
			client_sys.net_channel:post_reliable("PICK_REQUEST", {})
		end
	end
end

function wield_system:receive_item_ownership()
	local msgs = self.owner_entity_system.messages["ITEM_DROPPED"]
	local replication = self.owner_entity_system.all_systems["replication"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		
		self.owner_entity_system:post_table("item_ownership", {
			subject = replication.object_by_id[msg.data.subject_id],
			drop = true
		})
	end
	
	msgs = self.owner_entity_system.messages["ITEM_PICKED"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		
		self.owner_entity_system:post_table("item_ownership", {
			subject = replication.object_by_id[msg.data.subject_id],
			item = replication.object_by_id[msg.data.item_id],
			pick = true
		})
	end
end

function wield_system:update()
	local msgs = self.owner_entity_system.messages["item_ownership"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		local subject = msg.subject
		local wield = subject.wield
		
		-- check subject validity
		if wield ~= nil	then
			-- pick item
			if msg.pick == true then
				local item = msg.item
				-- check if it is not already picked by somebody else
				if item.item ~= nil and item.item.ownership == nil then
					msg.succeeded = true
					item.item:set_ownership(subject)
					wield.wielded_item = item
				end
				
			-- drop item
			elseif msg.drop == true then
				local item = wield.wielded_item 
				
				if item ~= nil and item.item ~= nil then
					msg.dropped_item = item
					msg.succeeded = true
					item.item:set_ownership(nil)
					wield.wielded_item = nil
				end
			end
			
		end
	end
end

function wield_system:ownership_callbacks()
	local msgs = self.owner_entity_system.messages["item_ownership"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		
		if msg.succeeded then
			if msg.pick then
				if msg.subject.wield.on_pick then
					msg.subject.wield.on_pick(msg.subject)
				end
				
				if msg.item.item.on_pick then
					msg.item.item.on_pick(msg.item)
				end
			elseif msg.drop then
				if msg.subject.wield.on_drop then
					msg.subject.wield.on_drop(msg.subject, msg.dropped_item)
				end
				
				if msg.dropped_item.item.on_drop then
					msg.dropped_item.item.on_drop(msg.dropped_item)
				end
			end
		end
	end
end
