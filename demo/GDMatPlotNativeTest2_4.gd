extends GDMatPlotNative

const _renderer_period: int = 200

var _lines: Array = []

func test_file(fn: String):
	if _lines.is_empty():
		var file = FileAccess.open("res://" + fn + ".txt", FileAccess.READ)
		while not file.eof_reached():
			var line = file.get_line()
			_lines.append(line)

	for line in _lines:
		run_command(line)

# Called when the node enters the scene tree for the first time.
var dataframe: PackedFloat64Array = PackedFloat64Array()
var lib_loaded: bool = false
func _ready():
	dataframe = PackedFloat64Array([
		10, 20, 60,
		10, 21, 60,
		10, 22, 65,
		10, 23, 60,
		10, 24, 60,

		11, 20, 60,
		11, 21, 60,
		11, 22, 65,
		11, 23, 60,
		11, 24, 60,

		12, 20, 65,
		12, 21, 65,
		12, 22, 65,
		12, 23, 65,
		12, 24, 65,

		13, 20, 60,
		13, 21, 60,
		13, 22, 65,
		13, 23, 60,
		13, 24, 60,

		14, 20, 60,
		14, 21, 60,
		14, 22, 65,
		14, 23, 60,
		14, 24, 60,
	])

	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		set_dataframe(dataframe, 3)
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

var _automata_rule = [ 0, 1, 1, 1, 0, 1, 1, 0 ]
func _update_dataframe():
	var tmp_dataframe = PackedFloat64Array(dataframe)
	for i in range(1, dataframe.size() / 3 - 1):
		var v1 = int((dataframe[3*i+2-3] - 60) / 5)
		var v2 = int((dataframe[3*i+2] - 60) / 5)
		var v3 = int((dataframe[3*i+2+3] - 60) / 5)

		tmp_dataframe[3*i+2] = (_automata_rule[v1+2*v2+4*v3] * 5) + 60

	dataframe = tmp_dataframe
	set_dataframe(dataframe, 3)

func _draw_commands():
	if lib_loaded:
		_update_dataframe()
		test_file("heatmap")

func _process(delta):
	queue_redraw()
