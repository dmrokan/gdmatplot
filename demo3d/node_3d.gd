extends Node3D

var _gnuplot_scene = preload("res://gui_panel_3d.tscn")
var _rooms: Dictionary = {}

func _ready():
	generate_plots()

func _input(event):
	if event.is_action_pressed("toggle_occlusion_culling"):
		get_viewport().use_occlusion_culling = not get_viewport().use_occlusion_culling
		update_labels()
	if event.is_action_pressed("toggle_mesh_lod"):
		get_viewport().mesh_lod_threshold = 1.0 if is_zero_approx(get_viewport().mesh_lod_threshold) else 0.0
		update_labels()
	if event.is_action_pressed("cycle_draw_mode"):
		get_viewport().debug_draw = wrapi(get_viewport().debug_draw + 1, 0, 5)
		update_labels()


func _process(delta):
	$Performance.text = """%d FPS (%.2f mspf)

Currently rendering:
%d objects
%dK primitive indices
%d draw calls
""" % [
	Engine.get_frames_per_second(),
	1000.0 / Engine.get_frames_per_second(),
	RenderingServer.get_rendering_info(RenderingServer.RENDERING_INFO_TOTAL_OBJECTS_IN_FRAME),
	RenderingServer.get_rendering_info(RenderingServer.RENDERING_INFO_TOTAL_PRIMITIVES_IN_FRAME) * 0.001,
	RenderingServer.get_rendering_info(RenderingServer.RENDERING_INFO_TOTAL_DRAW_CALLS_IN_FRAME),
]


func update_labels():
	$OcclusionCulling.text = "Occlusion culling: %s" % ("Enabled" if get_viewport().use_occlusion_culling else "Disabled")
	$MeshLOD.text = "Mesh LOD: %s" % ("Enabled" if not is_zero_approx(get_viewport().mesh_lod_threshold) else "Disabled")
	$DrawMode.text = "Draw mode: %s" % get_draw_mode_string(get_viewport().debug_draw)


func get_draw_mode_string(draw_mode):
	match draw_mode:
		0:
			return "Normal"
		1:
			return "Unshaded"
		2:
			return "Lighting"
		3:
			return "Overdraw"
		4:
			return "Wireframe"

func generate_plots():
	const draw_period: float = 1e-1

	var transform1 = Transform3D($Rooms/Room/GUIPanel3D.transform)
	var transform2 = Transform3D($Rooms/Room/GUIPanel3D2.transform)
	var transform3 = Transform3D($Rooms/Room/GUIPanel3D3.transform)
	var transform4 = Transform3D($Rooms/Room/GUIPanel3D4.transform)

	const scripts = [
		{ "name": "splot2", "redraw_period": 1e-1, "param_bounds": [ 0.5, 3 ], "param_increment": 3e-2  },
		{ "name": "splot1", "redraw_period": 1e-1, "param_bounds": [ 0.5, 3 ], "param_increment": 3e-2 },
		{ "name": "airfoil1", "redraw_period": INF, "param_bounds": [ 0.5, 3 ], "param_increment": 3e-2 },
		{ "name": "contour2", "redraw_period": 1e-1, "param_bounds": [ 0.5, 4 ], "param_increment": 3e-2 },
		{ "name": "test7", "redraw_period": 1e-1, "param_bounds": [ 0.5, 8 ], "param_increment": 1e-2 },
		{ "name": "test8", "redraw_period": 1e-1, "param_bounds": [ 1.1, 24 ], "param_increment": 1e-1 },
		]

	const visible_rooms = [ 2, 4, 30, 56 ]
	for room in $Rooms.get_children():
		var name = room.get_name().erase(0, 4)

		var i = int(name)
		if i not in visible_rooms:
			if i != 0:
				room.visible = false
			continue

		var plots: Array = []

		var plot = _gnuplot_scene.instantiate()
		plot.transform = Transform3D(transform1)
		plot.transform = plot.transform.rotated_local(Vector3(0, 1, 0), 3 * PI / 4)
		room.add_child(plot)
		var script_index = randi_range(0, scripts.size() - 1)
		var script_name = scripts[script_index]["name"]
		var redraw_period = scripts[script_index]["redraw_period"]
		plot.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)
		plots.append(plot)

		plot =_gnuplot_scene.instantiate()
		plot.transform = Transform3D(transform2)
		plot.transform = plot.transform.rotated_local(Vector3(0, 1, 0), 5 * PI / 4)
		room.add_child(plot)
		script_index = randi_range(0, scripts.size() - 1)
		script_name = scripts[script_index]["name"]
		redraw_period = scripts[script_index]["redraw_period"]
		plot.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)
		plots.append(plot)

		plot =_gnuplot_scene.instantiate()
		plot.transform = Transform3D(transform3)
		plot.transform = plot.transform.rotated_local(Vector3(0, 1, 0), 7 * PI / 4)
		room.add_child(plot)
		script_index = randi_range(0, scripts.size() - 1)
		script_name = scripts[script_index]["name"]
		redraw_period = scripts[script_index]["redraw_period"]
		plot.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)
		plots.append(plot)

		plot =_gnuplot_scene.instantiate()
		plot.transform = Transform3D(transform4)
		plot.transform = plot.transform.rotated_local(Vector3(0, 1, 0), 9 * PI / 4)
		room.add_child(plot)
		script_index = randi_range(0, scripts.size() - 1)
		script_name = scripts[script_index]["name"]
		redraw_period = scripts[script_index]["redraw_period"]
		plot.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)
		plots.append(plot)

		_rooms[room] = plots

	$Rooms/Room/GUIPanel3D.transform = transform1.rotated_local(Vector3(0, 1, 0), 3 * PI / 4)
	$Rooms/Room/GUIPanel3D2.transform = transform2.rotated_local(Vector3(0, 1, 0), 5 * PI / 4)
	$Rooms/Room/GUIPanel3D3.transform = transform3.rotated_local(Vector3(0, 1, 0), 7 * PI / 4)
	$Rooms/Room/GUIPanel3D4.transform = transform4.rotated_local(Vector3(0, 1, 0), 9 * PI / 4)

	var script_index = randi_range(0, scripts.size() - 1)
	var script_name = scripts[script_index]["name"]
	var redraw_period = scripts[script_index]["redraw_period"]
	$Rooms/Room/GUIPanel3D.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)

	script_index = randi_range(0, scripts.size() - 1)
	script_name = scripts[script_index]["name"]
	redraw_period = scripts[script_index]["redraw_period"]
	$Rooms/Room/GUIPanel3D2.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)

	script_index = randi_range(0, scripts.size() - 1)
	script_name = scripts[script_index]["name"]
	redraw_period = scripts[script_index]["redraw_period"]
	$Rooms/Room/GUIPanel3D3.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)

	script_index = randi_range(0, scripts.size() - 1)
	script_name = scripts[script_index]["name"]
	redraw_period = scripts[script_index]["redraw_period"]
	$Rooms/Room/GUIPanel3D4.set_gnuplot_script_and_params(script_name, scripts[script_index]["param_increment"], scripts[script_index]["param_bounds"], redraw_period)

	_rooms[$Rooms/Room] = [ $Rooms/Room/GUIPanel3D, $Rooms/Room/GUIPanel3D2, $Rooms/Room/GUIPanel3D3, $Rooms/Room/GUIPanel3D4 ]

func check_closest_room(pos: Vector3):
	var i: int = 0
	var min_dist: float = INF
	var min_dis_room = null
	for room in _rooms:
		var room_pos = Vector3(room.position)
		var dist = (room_pos - pos).length()
		if dist < min_dist:
			min_dist = dist
			min_dis_room = room

		for p in _rooms[room]:
			p.enable_draw(false)
		i += 1

	if min_dis_room:
		for p in _rooms[min_dis_room]:
			p.enable_draw()
