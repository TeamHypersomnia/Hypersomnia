for k, v in pairs(world_archetype_groups.guns) do
	world_archetype_callbacks[v] = {
		creation = function(self)
			local new_entity = components.create_components {
				item = self.owner_scene.weapons[v].item_info,
				weapon = self.owner_scene.weapons[v].weapon_info,
				interpolation = {}
			}
			
			new_entity.weapon:create_smoke_group(self.owner_scene.world_object.world)
			
			return new_entity
		end,
		
		construction = function(self, new_entity, is_object_new)
			-- nothing to do if we're recreating
			if not is_object_new then return end
			
			print "construction!"
			if new_entity.item.is_wielded then
				print "construction!"
				self.owner_entity_system:post_table("wield_item", {
					subject = self.object_by_id[new_entity.item.wielder_id],
					item = new_entity,
					wielding_key = components.wield.keys.PRIMARY_WEAPON
				})
			else
				new_entity.item.on_drop(new_entity)
			end
		end
	}
end