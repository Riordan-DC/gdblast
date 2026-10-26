extends Node3D


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
				apply_radial_damage(result.position, 0.2, 0.5, 0.5, 150.0)

func _physics_process(delta: float) -> void:
	if Engine.get_physics_frames() % 2 == 0:
		var result: Dictionary = {}
		var camera: Camera3D = get_viewport().get_camera_3d()
		var mouse_pos: Vector2 = get_viewport().get_mouse_position()
		var from: Vector3 = camera.project_ray_origin(mouse_pos)
		if Input.is_mouse_button_pressed(MOUSE_BUTTON_RIGHT):
			var to: Vector3 = from + camera.project_ray_normal(mouse_pos) * 1000.0
			var dss: PhysicsDirectSpaceState3D = get_world_3d().direct_space_state
			var q: PhysicsRayQueryParameters3D = PhysicsRayQueryParameters3D.new()
			q.from = from
			q.to = to
			q.collision_mask = 0xFFFFFFFF
			result = dss.intersect_ray(q)
		if !result.is_empty():
			apply_radial_damage(result.position, 1.0, 0.1, 0.1, 50.0)

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
	mesh.radius = 0.2
	mesh.height = 0.4
	shape.radius = 0.2
	bullet.continuous_cd = true
	bullet.global_position = from
	bullet.mass = 100.0
	bullet.linear_velocity = from.direction_to(to) * 100.0

func apply_radial_damage(pos: Vector3, damage: float, min_radius: float = 1.0, max_radius: float = 1.0, explosive: float = 10.0) -> void:
	# Position must be local to rigidbody
	var dss: PhysicsDirectSpaceState3D = get_world_3d().direct_space_state
	var q: PhysicsShapeQueryParameters3D = PhysicsShapeQueryParameters3D.new()
	var shape: SphereShape3D = SphereShape3D.new()
	shape.radius = max_radius
	q.shape = shape
	q.transform = Transform3D(Basis.IDENTITY, pos)
	var results: Array[Dictionary] = dss.intersect_shape(q, 4)
	for result: Dictionary in results:
		var rb: PhysicsBody3D = result.collider
		if rb is RigidBody3D:
			var ba: Node = rb.get_parent()
			if ba != null and ba is BlastAssetGD:
				if ba.actors.has(rb):
					var nvba: BlastActor = ba.actors[rb]
					var local_pos: Vector3 = ba.global_position + ( (ba.global_basis * (rb.global_transform.affine_inverse() * pos)) * 0.5 )#(ba.global_position + pos) * 0.5
					#local_pos = ba.global_basis.inverse() * local_pos
					#var marker: MeshInstance3D = MeshInstance3D.new()
					#var box: SphereMesh = SphereMesh.new()
					#marker.mesh = box
					#get_parent().add_child(marker)
					#marker.global_position = local_pos
					
					nvba.apply_radial_damage(local_pos, damage, min_radius, max_radius)
					ba.blast_group.process()
	
	await get_tree().physics_frame
	await get_tree().physics_frame
	
	var explosive_lift: Vector3 = Vector3.UP * explosive
	# Apply explosive after split bodies have been added
	results = dss.intersect_shape(q, 4)
	for result: Dictionary in results:
		var rb: PhysicsBody3D = result.collider
		if rb is RigidBody3D:
			rb.sleeping = false
			var random_nudge: Vector3 = Vector3(randf(), randf(), randf()) * explosive
			rb.apply_central_impulse(pos.direction_to(rb.global_position) * explosive + (explosive_lift * 0.5) + random_nudge)
			rb.apply_torque_impulse(random_nudge * 0.2)
