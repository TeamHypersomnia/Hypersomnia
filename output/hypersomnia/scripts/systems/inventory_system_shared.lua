function inventory_system:holster_item(subject_inventory, wielder, item)
	-- liberate the wielded object
	self.owner_entity_system:post_table("item_wielder_change", { 
		unwield = true,
		subject = wielder,
		wielding_key = components.wield.keys.PRIMARY_WEAPON
	})
	
	-- and hide it to the inventory
	self.owner_entity_system:post_table("item_wielder_change", { 
		wield = true,
		subject = subject_inventory,
		["item"] = item,
		wielding_key = item.replication.id
	})
end

function inventory_system:select_item(subject_inventory, wielder, item)
	-- liberate the selected object from the inventory
	self.owner_entity_system:post_table("item_wielder_change", { 
		unwield = true,
		subject = subject_inventory,
		wielding_key = item.replication.id
	})
	
	local previously_worn = wielder.wield.wielded_items[components.wield.keys.PRIMARY_WEAPON]
	
	if previously_worn then
		-- liberate the previously wielded object
		self.owner_entity_system:post_table("item_wielder_change", { 
			unwield = true,
			subject = wielder,
			wielding_key = components.wield.keys.PRIMARY_WEAPON
		})
	
		-- and the previously worn item should be hidden back into the inventory
		-- first we should check if it actually was in the inventory, by checking for the existence
		-- of the "inventory_slot" field
		self.owner_entity_system:post_table("item_wielder_change", { 
			wield = true,
			subject = subject_inventory,
			["item"] = previously_worn,
			wielding_key = previously_worn.replication.id
		})
	end
	
	-- let the newly selected item be held by the character
	self.owner_entity_system:post_table("item_wielder_change", { 
		wield = true,
		subject = wielder,
		["item"] = item,
		wielding_key = components.wield.keys.PRIMARY_WEAPON
	})
end