wield_system = inherits_from (processing_system)

--function wield_system:constructor(world_object)
--	self.world_object = world_object
--	processing_system.constructor(self)
--end

function wield_system:remove_entity(removed_entity)
	-- if we wield an item, we may need to drop it
	
	-- THIS IS THE GAME LOGIC THAT SHOULD DECIDE IF THE OBJECT IS TO BE DROPPED,
	-- AND IF SO, IT SHOULD POST A DROP MESSAGE BEFORE POSTING DELETION OF THE OBJECT
	
	-- IF THE ITEM ISN'T DROPPED, IT IS SIMPLY DELETED
	print "REMOVING WIELDED"
	if removed_entity.wield.wielded_item ~= nil then
		self.owner_entity_system:remove_entity(removed_entity.wield.wielded_item)
	end
	
	processing_system.remove_entity(self, removed_entity)
end
--
function wield_system:get_required_components()
	return { "wield" }
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
					-- only server-side
					if subject.client_controller ~= nil then
						item.replication:switch_group_for_client(subject.client_controller.owner_client, "OWNER")
						item.replication:switch_public_group("PUBLIC")
					end
					
					item.item:set_ownership(subject)
					wield.wielded_item = item
				end
				
			-- drop item
			elseif msg.drop == true then
				local item = wield.wielded_item 
				
				if item ~= nil and item.item ~= nil then
					-- unmap that item		
					-- only server-side		
					if subject.client_controller ~= nil then
						item.replication:switch_public_group("DROPPED_PUBLIC")
						item.replication:switch_group_for_client(subject.client_controller.owner_client, "DROPPED_PUBLIC")
						-- subject.client_controller.owner_client.client.group_by_id[item.replication.id] = nil
					end
				
					item.item:set_ownership(nil)
					wield.wielded_item = nil
				end
			end
			
		end
	end
end


function wield_system:broadcast_item_ownership()
	local msgs = self.owner_entity_system.messages["item_ownership"]
	
	for i=1, #msgs do
		
	
	end
end
