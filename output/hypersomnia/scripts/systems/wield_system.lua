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

function wield_system:receive_item_wieldings()
	local msgs = self.owner_entity_system.messages["ITEM_UNWIELDED"]
	
	for i=1, #msgs do
		local data = msgs[i].data
		
		self.owner_entity_system:post_table("item_wielder_change", {
			subject = replication.object_by_id[data.subject_id],
			wielding_key = data.wielding_key,
			unwield = true
		})
	end
	
	msgs = self.owner_entity_system.messages["ITEM_WIELDED"]
	local replication = self.owner_entity_system.all_systems["replication"]
	
	for i=1, #msgs do
		local data = msgs[i].data
		print "ITEM_WIELDED"
		self.owner_entity_system:post_table("item_wielder_change", {
			subject = replication.object_by_id[data.subject_id],
			item = replication.object_by_id[data.item_id],
			wielding_key = data.wielding_key,
			wield = true
		})
	end
end

components.wield.wield_item = function(wielder, new_item, wielding_key)
	local self = wielder.wield
	
	local old_item = self.wielded_items[wielding_key]
	self.wielded_items[wielding_key] = new_item
	new_item.item.wielding_key = wielding_key
	
	-- this should actually never happen
	-- as the wielding in the place of an old item
	-- should be preceded by the unwield of this old item
	if old_item then
		old_item.item:set_wielder(nil)
	end
	
	new_item.item:set_wielder(wielder)
	
	if self.on_item_wielded then
		self.on_item_wielded(wielder, new_item, old_item, wielding_key)
	end
end

components.wield.unwield_item = function(wielder, old_item, wielding_key_hint)
	local found_item;
	local self = wielder.wield
	
	if wielding_key_hint ~= nil then
		found_item = self.wielded_items[wielding_key_hint]
		
		self.wielded_items[wielding_key_hint] = nil
	else
		for k, v in pairs(self.wielded_items) do
			if v == old_item then
				wielding_key_hint = k
				self.wielded_items[k] = nil
				found_item = old_item
				break
			end
		end
	end
	
	if found_item then
		found_item.item:set_wielder(nil)
	end
	
	if found_item and self.on_item_unwielded then
		self.on_item_unwielded(wielder, found_item, wielding_key_hint)
	end
	
	return found_item
end

function wield_system:update()
	local msgs = self.owner_entity_system.messages["item_wielder_change"]
	-- unwield = true
	-- subject
	-- item OR wielding_key
	
	-- wield = true
	-- subject
	-- item 
	-- wielding_key
	
	for i=1, #msgs do
		local msg = msgs[i]
		local subject = msg.subject
		
		-- check subject validity
		if not msg.succeeded and subject.wield ~= nil then
			if msg.unwield then
				local item = components.wield.unwield_item(subject, msg.item, msg.wielding_key)
	
				if item then
					msg.succeeded = true
					msg.item = item
				end
			elseif msg.wield then
				local item = msg.item
				if not msg.wield_if_unwielded or item.item.wielder == nil then
					print "WIELDING..."
					msg.succeeded = true
					components.wield.wield_item(subject, msg.item, msg.wielding_key)
				end
			end
		end
	end
end
