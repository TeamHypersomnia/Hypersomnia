--size_mult = vec2(9, -9)
--my_dungeon = {
--
--vec2(440.3286, 74.7106 ) * size_mult,
--vec2(490.8061, 97.9029 ) * size_mult,
--vec2(477.8457, 177.7119) * size_mult,
--vec2(405.5401, 208.4077) * size_mult,
--vec2(398.7188, 259.5673) * size_mult,
--vec2(483.3027, 242.5141) * size_mult,
--vec2(520.1376, 224.7788) * size_mult,
--vec2(525.5947, 289.5810) * size_mult,
--vec2(481.2563, 329.1444) * size_mult,
--vec2(434.1895, 327.7801) * size_mult,
--vec2(408.9507, 289.5810) * size_mult,
--vec2(375.5265, 302.5414) * size_mult,
--vec2(369.3873, 338.0121) * size_mult,
--vec2(380.3014, 371.4363) * size_mult,
--vec2(439.6465, 353.0189) * size_mult,
--vec2(477.8457, 351.6546) * size_mult,
--vec2(492.1704, 387.1253) * size_mult,
--vec2(477.1636, 411.6819) * size_mult,
--vec2(420.5469, 426.0066) * size_mult,
--vec2(370.0694, 409.6355) * size_mult,
--vec2(338.6915, 398.7215) * size_mult,
--vec2(323.6847, 411.6819) * size_mult,
--vec2(327.7775, 438.2849) * size_mult,
--vec2(357.7911, 443.0598) * size_mult,
--vec2(376.8907, 463.5237) * size_mult,
--vec2(393.9439, 493.5373) * size_mult,
--vec2(431.4610, 485.3518) * size_mult,
--vec2(459.4282, 443.0598) * size_mult,
--vec2(482.6206, 455.3381) * size_mult,
--vec2(491.4883, 496.2658) * size_mult,
--vec2(482.6206, 527.6437) * size_mult,
--vec2(426.0039, 518.7760) * size_mult,
--vec2(371.4337, 518.7760) * size_mult,
--vec2(344.1486, 487.3981) * size_mult,
--vec2(327.7775, 464.2058) * size_mult,
--vec2(304.5851, 468.9807) * size_mult,
--vec2(300.4924, 513.3190) * size_mult,
--vec2(273.8893, 522.1867) * size_mult,
--vec2(233.6438, 520.1403) * size_mult,
--vec2(192.7161, 515.3654) * size_mult,
--vec2(158.6097, 516.7297) * size_mult,
--vec2(124.5033, 511.2726) * size_mult,
--vec2(93.1254, 491.4909 ) * size_mult,
--vec2(73.3436, 453.2917 ) * size_mult,
--vec2(74.0258, 408.9534 ) * size_mult,
--vec2(97.2181, 403.4964 ) * size_mult,
--vec2(115.6356, 441.6956) * size_mult,
--vec2(124.5033, 479.2126) * size_mult,
--vec2(172.2522, 480.5769) * size_mult,
--vec2(239.1008, 471.7092) * size_mult,
--vec2(236.3723, 432.8279) * size_mult,
--vec2(179.0735, 434.8743) * size_mult,
--vec2(173.6165, 388.4895) * size_mult,
--vec2(241.1472, 383.0325) * size_mult,
--vec2(265.7038, 419.8674) * size_mult,
--vec2(295.0353, 406.9070) * size_mult,
--vec2(288.2140, 355.0653) * size_mult,
--vec2(252.0612, 311.4091) * size_mult,
--vec2(204.3123, 325.7338) * size_mult,
--vec2(158.6097, 368.7078) * size_mult,
--vec2(116.3177, 372.8006) * size_mult,
--vec2(101.9930, 327.0980) * size_mult,
--vec2(76.7543, 299.8129 ) * size_mult,
--vec2(95.8539, 239.7856 ) * size_mult,
--vec2(159.2918, 235.6928) * size_mult,
--vec2(143.6029, 291.6273) * size_mult,
--vec2(214.5442, 295.0380) * size_mult,
--vec2(223.4119, 232.2822) * size_mult,
--vec2(288.8962, 208.4077) * size_mult,
--vec2(303.2209, 261.6137) * size_mult,
--vec2(292.9889, 285.4882) * size_mult,
--vec2(316.1813, 332.5550) * size_mult,
--vec2(335.9630, 358.4759) * size_mult,
--vec2(334.5988, 289.5810) * size_mult,
--vec2(357.7911, 218.6396) * size_mult,
--vec2(320.2741, 200.2222) * size_mult,
--vec2(284.1213, 159.9766) * size_mult,
--vec2(232.2795, 191.3545) * size_mult,
--vec2(196.1267, 228.8715) * size_mult,
--vec2(162.0203, 184.5332) * size_mult,
--vec2(101.9930, 180.4404) * size_mult,
--vec2(82.8934, 165.4336 ) * size_mult,
--vec2(84.9398, 134.0557 ) * size_mult,
--vec2(119.0462, 98.5850 ) * size_mult,
--vec2(157.2454, 149.0625) * size_mult,
--vec2(192.7161, 126.5523) * size_mult,
--vec2(164.0667, 93.8101 ) * size_mult,
--vec2(167.4773, 73.3463 ) * size_mult,
--vec2(132.0662, 72.2483) * size_mult,
--vec2(73.2964, 70.4260 ) * size_mult,
--vec2(71.9794, 187.2617 ) * size_mult,
--vec2(121.0926, 207.7256) * size_mult,
--vec2(80.1649, 222.0503 ) * size_mult,
--vec2(56.2904, 224.7788 ) * size_mult,
--vec2(39.9194, 181.1226 ) * size_mult,
--vec2(39.2372, 83.5782  ) * size_mult,
--vec2(50.8334, 59.7037  ) * size_mult,
--vec2(134.7352, 44.6969 ) * size_mult,
--vec2(196.8089, 39.9220 ) * size_mult,
--vec2(196.1267, 77.4391 ) * size_mult,
--vec2(224.0940, 108.1348) * size_mult,
--vec2(232.9617, 141.5591) * size_mult,
--vec2(282.7570, 86.9889 ) * size_mult,
--vec2(322.3205, 66.5250 ) * size_mult,
--vec2(338.6915, 135.4200) * size_mult,
--vec2(350.2877, 164.7515) * size_mult,
--vec2(391.2154, 147.6983) * size_mult,
--vec2(427.3682, 155.8838) * size_mult,
--vec2(404.1758, 112.9097) * size_mult,
--vec2(372.7979, 111.5455) * size_mult,
--vec2(353.0162, 71.2999 ) * size_mult,
--vec2(380.3014, 56.2931 ) * size_mult,
--vec2(428.7325, 56.9752 ) * size_mult,
--vec2(417.8184, 91.0816 ) * size_mult,
--vec2(440.3286, 74.7106 ) * size_mult
--}
--
--environment_poly = create_polygon_with_holes {
--	subject = {
--vec2(45.0325, 535.6137) * size_mult,
--vec2(53.9025, 453.0542) * size_mult,
--vec2(66.1841, 369.8123) * size_mult,
--vec2(52.5379, 327.5090) * size_mult,
--vec2(36.1625, 274.9711) * size_mult,
--vec2(23.8809, 210.8339) * size_mult,
--vec2(25.2455, 137.8267) * size_mult,
--vec2(19.1047, 87.3357 ) * size_mult,
--vec2(31.3863, 53.9025 ) * size_mult,
--vec2(71.6426, 22.5162 ) * size_mult,
--vec2(154.8845, 13.6462 ) * size_mult,
--vec2(230.6209, 17.7401 ) * size_mult,
--vec2(249.0433, 57.9964 ) * size_mult,
--vec2(312.4982, 45.7148 ) * size_mult,
--vec2(409.3863, 27.2924 ) * size_mult,
--vec2(484.4404, 21.8339 ) * size_mult,
--vec2(524.0144, 55.2671 ) * size_mult,
--vec2(524.6968, 113.9458) * size_mult,
--vec2(530.1552, 180.8123) * size_mult,
--vec2(489.2166, 208.1047) * size_mult,
--vec2(540.3899, 219.7040) * size_mult,
--vec2(551.9892, 278.3827) * size_mult,
--vec2(544.4838, 337.7437) * size_mult,
--vec2(519.9206, 375.9531) * size_mult,
--vec2(504.2274, 417.5740) * size_mult,
--vec2(505.5921, 451.6895) * size_mult,
--vec2(557.4477, 474.2058) * size_mult,
--vec2(539.0253, 516.5090) * size_mult,
--vec2(508.3213, 539.7076) * size_mult,
--vec2(421.6679, 549.9422) * size_mult,
--vec2(281.7942, 549.9422) * size_mult,
--vec2(73.0072, 546.5307) * size_mult, 
--vec2(45.0325, 535.6137) * size_mult
--	},
--	
--	holes = {
--		my_dungeon
--	}
--}
--
--ground_poly = create_polygon_with_holes {
--	subject = my_dungeon,
--	
--	holes = {
--	
--	}
--}
--
--map_uv_square(environment_poly, images.metal)
--map_uv_square(ground_poly, images.background)

--environment = create_entity {
--	render = {
--		model = sample_prostokat,
--		layer = render_layers.OBJECTS
--	},
--	
--	physics = {
--		body_type = Box2D.b2_staticBody,
--		
--		body_info = {
--			shape_type = physics_info.POLYGON,
--			filter = filter_static_objects,
--			friction = 0
--		}
--	},
--	
--	particle_emitter = {
--		available_particle_effects = metal_effects
--	},
--	
--	transform = {
--		pos = vec2(-1000, 1000)
--	}
--}

--ground = create_entity {
--	render = {
--		model = ground_poly,
--		layer = render_layers.GROUND
--	},
--	
--	transform = {
--		pos = vec2(-1000, 1000)
--	}
--}

floor_archetype = {
	transform = {
		pos = vec2(0, 0),
		rotation = 0
	},
	
	render = {
		--layer = render_layers.GROUND
	}
}

wall_archetype = archetyped(floor_archetype, {
	particle_emitter = {
			available_particle_effects = metal_effects
	},
	
	physics = {
		body_type = Box2D.b2_staticBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			--vertices = sample_prostokat,
			filter = filter_static_objects,
			friction = 0
		}
	},
	
	render = {
		--layer = render_layers.OBJECTS
	}
})

global_all_polys = {}

function create_poly (poly_vertices, image_to_map, pick_archetype, reverse_order)
	reverse_order = reverse_order or false
	
	local out_verts = {}
	
	if reverse_order then
		for i = #poly_vertices, 1, -1 do
			table.insert(out_verts, poly_vertices[i])
		end
	
		poly_vertices = out_verts
	end
	
	local my_poly = simple_create_polygon (poly_vertices)
	table.insert(global_all_polys, my_poly)
	
	map_uv_square(my_poly, image_to_map)
	
	my_entity = create_entity (archetyped(pick_archetype, {
		render = {
			model = my_poly
		}
	}))
end

function create_wall(poly_vertices, image_to_map, reverse_order, rlayer)	
	if rlayer == nil then
		rlayer = render_layers.OBJECTS
	end
	
	create_poly (poly_vertices, image_to_map,  archetyped(wall_archetype, {render = {layer = rlayer } }), reverse_order)
end

function create_floor(poly_vertices, image_to_map, reverse_order, rlayer)
	if rlayer == nil then
		rlayer = render_layers.GROUND
	end
	create_poly (poly_vertices, image_to_map, archetyped(floor_archetype, {render = { layer = rlayer } }), reverse_order)
end



create_wall( {
		vec2(0, 0),
		vec2(300, 0),
		vec2(300, 300),
		vec2(0, 300)
		},
		images.crate, true)
		
		
create_floor( {
		vec2(-100, -100) + vec2(0, 0),
		vec2(-100, -100) + vec2(500, 0),
		vec2(-100, -100) + vec2(500, 500),
		vec2(-100, -100) + vec2(0, 500)
		},
		images.background, false)

		
