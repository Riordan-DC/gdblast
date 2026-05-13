class_name BlastAssetGD
extends Node3D


@export var is_static: bool = true
@export var anchor_ground: bool = true
@export var facade: Node3D = null
@export var total_mass: float = 10.0
@export var debug: bool = false : 
	set(value):
		debug = value
		set_process(debug)

const UINT32_MAX: int = 0xffffffff

var actors: Dictionary = {}
var colliders: Array[CollisionShape3D] = []
var chunks: Array[BlastChunk] = []
var blast_group: BlastGroup
var debug_drawer: ImmediateMesh
var mass_per_chunk: float = 0.0

var health: float = 20.0
## Ignore damage below health * min_damage
var min_damage: float = 0.1
## Less than 1.0 means it will take at minimum 2 hits to break
var max_damage: float = 1.0

func _ready() -> void:
	if facade: facade.hide()
	
	set_process(debug)
	blast_group = BlastGroup.new()
	blast_group.on_actor_split.connect(on_actor_split)
	var blast_actor: BlastActor = BlastActor.new()
	blast_actor.test()
	
	# Based on children
	var test_body: StaticBody3D = StaticBody3D.new()
	add_child(test_body)
	test_body.collision_layer = 1
	test_body.collision_mask = 1
	for child: Node in get_children():
		if child is CollisionShape3D:
			colliders.append(child)
			var chunk_pos: Vector3 = child.position # Position is local to this
			var child_mesh: MeshInstance3D = child.get_child(0)
			var anchor: bool = chunk_pos.y < 0.1 if anchor_ground else false
			anchor = anchor or child_mesh.name.contains("anchor")
			chunks.append(blast_actor.add_chunk(UINT32_MAX, chunk_pos, 1.0, true, 0, anchor))
			child.reparent(test_body, true)
	mass_per_chunk = total_mass / chunks.size()
	
	await get_tree().physics_frame
	
	var bond_normal: Vector3 = Vector3(1,0,0) # +x
	var dss: PhysicsDirectSpaceState3D = get_world_3d().direct_space_state
	for i: int in range(colliders.size()):
		# Physics test
		var collider: CollisionShape3D = colliders[i]
		var query: PhysicsShapeQueryParameters3D = PhysicsShapeQueryParameters3D.new()
		query.set_shape(collider.shape)
		query.collision_mask = 0xFFFFFFFFF
		query.transform = collider.global_transform
		query.margin = 0.2 # + 0.2
		#query.exclude = [collider]
		var adj_shapes: Array[Dictionary] = dss.intersect_shape(query, 4)
		
		for adj_shape: Dictionary in adj_shapes:
			if adj_shape.collider == test_body:
				var adj_shape_id = adj_shape.shape
				var j: int = adj_shape_id
				if i != j:
					blast_actor.add_bond(i, j, 
						collider.global_position.lerp(adj_shape.collider.global_position, 0.5), 
						bond_normal, 
						1.0)
	
	for child: Node in test_body.get_children():
		child.reparent(self, true)
	
	test_body.queue_free()
	
	blast_actor.create_actor()
	blast_group.add_actor(blast_actor)
	
	on_actor_created(blast_actor, global_transform)

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

func on_actor_created(actor: BlastActor, pose: Transform3D, linear_velocity: Vector3 = Vector3.ZERO, angular_velocity: Vector3 = Vector3.ZERO) -> void:
	var rigidbody: RigidBody3D = RigidBody3D.new()
	rigidbody.set_center_of_mass_mode(RigidBody3D.CENTER_OF_MASS_MODE_AUTO)
	add_child(rigidbody)
	rigidbody.top_level = true
	rigidbody.global_transform = pose
	rigidbody.linear_velocity = linear_velocity
	rigidbody.angular_velocity = angular_velocity
	rigidbody.freeze_mode = RigidBody3D.FREEZE_MODE_KINEMATIC

	actor.set_body(rigidbody)
	actor.stress_actor_created(actor)
	
	# Reparent chunks
	var chunk_indices: PackedInt32Array = actor.get_visible_chunk_indices()
	#var centroid: Vector3 = Vector3.ZERO
	var new_color: Color = Color(randf(), randf(), randf())
	for node: int in chunk_indices:
		var collider: CollisionShape3D = colliders[node]
		var chunk: BlastChunk = chunks[node]
		var mesh: MeshInstance3D = collider.get_child(0)
		if chunk.is_static():
			rigidbody.freeze = true
			if mesh:
				var static_mat: StandardMaterial3D = StandardMaterial3D.new()
				static_mat.albedo_color = Color.RED
				mesh.material_override = static_mat
		else:
			if mesh:
				var static_mat: StandardMaterial3D = StandardMaterial3D.new()
				static_mat.albedo_color = new_color
				mesh.material_override = static_mat

		#centroid += collider.global_position
		collider.reparent(rigidbody, true)
	
	if chunk_indices.size() > 1:
		rigidbody.custom_integrator = true
		rigidbody.contact_monitor = true
		rigidbody.max_contacts_reported = 128
		PhysicsServer3D.body_set_force_integration_callback(rigidbody.get_rid(), integrate_forces, rigidbody)
	
	rigidbody.mass = chunks.size() * mass_per_chunk
	#rigidbody.center_of_mass = (centroid / chunk_indices.size())
	
	rigidbody.sleeping = false
	actor.set_userdata(rigidbody.get_instance_id())
	actor.set_material(health, min_damage, max_damage)
	actors[rigidbody] = actor

func on_actor_destroyed(instance_id: int) -> Array:
	var last_xform: Transform3D = Transform3D.IDENTITY
	var rigidbody: RigidBody3D = instance_from_id(instance_id)
	var actor: BlastActor = actors[rigidbody]
	var chunk_indices: PackedInt32Array = actor.get_visible_chunk_indices()
	
	for node: int in chunk_indices:
		var collider: CollisionShape3D = colliders[node]
		collider.reparent(self, true)
	
	#for node: Node in rigidbody.get_children():
		#if node is CollisionShape3D:
			#node.reparent(self, true)

	last_xform = rigidbody.global_transform
	var lin_vel: Vector3 = rigidbody.linear_velocity
	var ang_vel: Vector3 = rigidbody.angular_velocity
	actors.erase(rigidbody);
	rigidbody.queue_free()
	return [last_xform, lin_vel, ang_vel]

func on_actor_split(old_userdata: Variant, new_actors: Array) -> void:
	var actor_ids: Array[int] = []
	for actor: BlastActor in actors.values():
		actor_ids.push_back(actor.get_instance_id())
	
	var new_actor_ids: Array[int] = []
	for actor: BlastActor in new_actors:
		new_actor_ids.push_back(actor.get_instance_id())
	#print("split, actor ids: ", actor_ids, " old_actor: ", old_userdata, " new_actors:", new_actor_ids)
	
	# Kill old body
	var state: Array = on_actor_destroyed(old_userdata)
	
	# Create new bodies
	for new_actor: BlastActor in new_actors:
		on_actor_created(new_actor, state[0], state[1], state[2])

func global_to_blast(global_pos: Vector3, ba: BlastAssetGD, rb: RigidBody3D) -> Vector3:
	return ba.global_position + ( (ba.global_basis * (rb.global_transform.affine_inverse() * global_pos)) * 0.5 )

var damaged: bool = false
var force_threshold: float = 10.0 #5.0

func integrate_forces(state: PhysicsDirectBodyState3D, rigidbody: RigidBody3D) -> void:
	if !state.sleeping:
		if actors.has(rigidbody):
			var blast_actor: BlastActor = actors[rigidbody]
			var contact_count: int = state.get_contact_count()
			
			for contact: int in range(contact_count):
				var velocity_delta: Vector3 = state.get_contact_local_velocity_at_position(contact) - state.get_contact_collider_velocity_at_position(contact)#rigidbody.linear_velocity + rigidbody.angular_velocity
				var colliding_body: Object = state.get_contact_collider_object(contact)
				
				var mass: float = rigidbody.mass
				if colliding_body is RigidBody3D:
					mass += colliding_body.mass
				
				var contact_normal: Vector3 = state.get_contact_local_normal(contact)
				var force_vector: Vector3 = -contact_normal * velocity_delta * mass * state.step
				var force_power: float = force_vector.length() * Engine.time_scale
				
				if force_power > force_threshold:
					damaged = true
					var pos: Vector3 = global_to_blast(state.get_contact_collider_position(contact), self, rigidbody)
					var max_force: float = 100.0
					var damage: float = force_power * 0.2
					var max_range: float = 0.8
					var range: float = 1.0 #(max_force / clamp(damage, 0.0, max_force)) * max_range
					blast_actor.apply_spread_damage(pos, damage, 0.5, range)
					DebugDraw.draw_box(state.get_contact_collider_position(contact), Vector3.ONE * range, Color.RED, 60)
	
	if damaged:
		damaged = false
		blast_group.process()
	
	state.integrate_forces()
