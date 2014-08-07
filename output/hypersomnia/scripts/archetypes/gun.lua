for k, v in pairs(world_archetype_groups.guns) do
	world_archetype_callbacks[v] = {
		creation = function(self)
			local new_entity = components.create_components {
				item = {
					physics_table = {
						body_type = Box2D.b2_dynamicBody,
						
						body_info = {
							filter = filters.DROPPED_ITEM,
							shape_type = physics_info.RECT,
							rect_size = vec2(98, 36),
							
							linear_damping = 4,
							angular_damping = 4,
							fixed_rotation = false,
							density = 0.1,
							friction = 0,
							restitution = 0.4,
							sensor = false
						}
					}
				},
			
				weapon = self.owner_scene.weapons[v].weapon_info
			}
			
			new_entity.weapon:create_smoke_group(self.owner_scene.world_object.world)
			
			return new_entity
		end,
		
		construction = function(self, new_entity)
			if new_entity.item.is_owned then
				self.owner_entity_system:post_table("item_ownership", {
					subject = self.object_by_id[new_entity.item.ownership_id],
					item = new_entity,
					pick = true
				})
			end
		end
	}
end