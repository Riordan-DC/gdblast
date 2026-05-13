@tool # Needed so it runs in editor.
extends EditorScenePostImport

func _post_import(scene: Node) -> Object:
	
	for child: Node in scene.get_children():
		if child.name.contains("blast"):
			try_make_asset(scene, child)
	
	return scene

func try_make_asset(scene: Node, node: Node) -> void:
	var children: Array[Node] = node.get_children()
	
	var body: BlastAssetGD = BlastAssetGD.new()
	body.position = node.position
	body.name = node.name
	scene.add_child(body, true)
	node.reparent(body)
	body.facade = node
	body.owner = scene
	
	var i: int = 0
	for child: Node in children:
		if child is MeshInstance3D:
			# Make collider for each mesh
			var shape: CollisionShape3D = CollisionShape3D.new()
			body.add_child(shape, true)
			shape.name = str(i)
			i += 1
			shape.shape = child.mesh.create_convex_shape(true, true)
			shape.position = child.position
			shape.owner = scene
			child.reparent(shape)
