extends BlastAssetGD


@export var tower: bool = false

func _ready() -> void:
	set_process(debug)
	blast_group = BlastGroup.new()
	blast_group.on_actor_split.connect(on_actor_split)
	var blast_actor: BlastActor = BlastActor.new()
	blast_actor.test()
	
	if tower:
		for i: int in range(10):
			var chunk_pos: Vector3 = Vector3(0,i,0)
			chunks.append(blast_actor.add_chunk(UINT32_MAX, chunk_pos, 1.0, true, 0, i==0))
			blast_actor.add_bond(i, i+1, chunk_pos, Vector3.UP, 1.0)

		blast_actor.create_actor()
		
		blast_group.add_actor(blast_actor)
		
		# Create pool
		colliders.resize(chunks.size())
		for i: int in range(chunks.size()):
			var chunk: BlastChunk = chunks[i]
			var collider: CollisionShape3D = CollisionShape3D.new()
			var shape: BoxShape3D = BoxShape3D.new()
			shape.size = Vector3(chunk.volume, chunk.volume, chunk.volume)
			collider.shape = shape
			collider.transform = Transform3D(Basis.IDENTITY, chunk.position)
			
			var mesh: MeshInstance3D = MeshInstance3D.new()
			var box: BoxMesh = BoxMesh.new()
			box.size = shape.size
			mesh.mesh = box
			
			add_child(collider)
			collider.add_child(mesh)
			colliders[i] = collider
	else:
		# Based on children
		var test_body: StaticBody3D = StaticBody3D.new()
		add_child(test_body)
		test_body.collision_layer = 1
		test_body.collision_mask = 1
		for child: Node in get_children():
			if child is CollisionShape3D:
				colliders.append(child)
				var chunk_pos: Vector3 = to_global(child.position) # Position is local to this
				var anchor: bool = chunk_pos.y < 1.0
				chunks.append(blast_actor.add_chunk(UINT32_MAX, chunk_pos, 3.0, true, 0, anchor))
				child.reparent(test_body, true)
		print("Creating Blast Asset with nodes: ", chunks.size())
		mass_per_chunk = total_mass / chunks.size()
		
		await get_tree().physics_frame
		
		var dss: PhysicsDirectSpaceState3D = get_world_3d().direct_space_state
		for i: int in range(colliders.size()):
			# Physics test
			var collider: CollisionShape3D = colliders[i]
			var query: PhysicsShapeQueryParameters3D = PhysicsShapeQueryParameters3D.new()
			query.set_shape(collider.shape)
			query.collision_mask = 0xFFFFFFFFF
			query.transform = collider.global_transform
			query.margin = 0.2 # + 0.2
			query.exclude = [collider]
			var adj_shapes: Array[Dictionary] = dss.intersect_shape(query, 4)
			
			for adj_shape: Dictionary in adj_shapes:
				if adj_shape.collider == test_body:
					var adj_shape_id = adj_shape.shape
					var j: int = adj_shape_id
					if i != j:
						blast_actor.add_bond(i, j, (collider.global_position + adj_shape.collider.global_position) * 0.5, collider.global_position.direction_to(adj_shape.collider.global_position), 1.0 * 2.0)
		
		for child: Node in test_body.get_children():
			child.reparent(self, true)
		
		test_body.queue_free()
		
		blast_actor.create_actor()
		blast_group.add_actor(blast_actor)
	
	on_actor_created(blast_actor, global_transform)

func _input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		var result: Dictionary = {}
		var camera: Camera3D = get_viewport().get_camera_3d()
		var from: Vector3 = camera.project_ray_origin(event.global_position)
		if event.is_pressed():
			var to: Vector3 = from + camera.project_ray_normal(event.global_position) * 1000.0
			var dss: PhysicsDirectSpaceState3D = get_world_3d().direct_space_state
			var q: PhysicsRayQueryParameters3D = PhysicsRayQueryParameters3D.new()
			q.from = from
			q.to = to
			q.collision_mask = 0xFFFFFFFF
			result = dss.intersect_ray(q)
		
		if event.button_index == MOUSE_BUTTON_LEFT and event.is_pressed():
			if !result.is_empty():
				fire_projectile(from, result.position)
		if event.button_index == MOUSE_BUTTON_RIGHT and event.is_pressed():
			if !result.is_empty():
				apply_radial_damage(result.position, 1.0, 1.0, 1.0, 30.0)

func fire_projectile(from: Vector3, to: Vector3) -> void:
	var bullet: RigidBody3D = RigidBody3D.new()
	var mesh: SphereMesh = SphereMesh.new()
	var meshi: MeshInstance3D = MeshInstance3D.new()
	var collider: CollisionShape3D = CollisionShape3D.new()
	var shape: SphereShape3D = SphereShape3D.new()
	collider.shape = shape
	meshi.mesh = mesh
	collider.add_child(meshi)
	bullet.add_child(collider)
	get_parent().add_child(bullet)
	bullet.continuous_cd = true
	bullet.global_position = from
	bullet.mass = 40.0
	bullet.linear_velocity = from.direction_to(to) * 300.0
