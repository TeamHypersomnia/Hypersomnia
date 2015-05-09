for k, v in pairs(world_archetype_groups.guns) do
	world_archetype_callbacks[v] = {
		creation = function(self)
			local item_table = self.owner_scene.weapons[v]
			
			local new_entity = components.create_components {
				item = item_table.item_info,
				weapon = item_table.weapon_info,
				interpolation = { 
					extrapolate = true,
					
					create_simulation_entity = function() 
						return self.owner_scene.simulation_world:create_entity {
							transform = {},
							physics = item_table.item_info.entity_archetype.physics
						}
					end
				},
				
				particle_response = {
					response = self.owner_scene.particles.gun_response
				}
			}
			
			new_entity.weapon:create_smoke_group(self.owner_scene.world_object.world)
			
			return new_entity
		end,
		
		post_construction = function(self, new_entity, is_object_new)
			-- nothing to do if we're recreating
			if not is_object_new then return end
			
			if new_entity.item.is_wielded then
				self.owner_entity_system:post_table("item_wielder_change", {
					wield = true,
					subject = self.object_by_id[new_entity.item.wielder_id],
					item = new_entity,
					wielding_key = new_entity.item.remote_wielding_key
				})
			else
				new_entity.item.on_wielder_changed(new_entity, nil)
			end
		end
	}
end