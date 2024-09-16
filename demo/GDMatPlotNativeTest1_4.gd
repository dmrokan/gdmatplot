extends GDMatPlotNative

const _renderer_period: int = 10

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
		1, 4300, 1000, 1, 7000, 2500, 1, 2200, 1600, 2, 4500, 1700,
		2, 6000, 2750, 2, 6100, 3000, 2, 6200, 4000, 3, 7000, 7800,
		4, 5000, 4500, 5, 5100, 4000, 5, 9000, 3700,
	])

	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		set_dataframe(dataframe, 3)
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

func _draw_commands():
	if lib_loaded:
		load_dataframe()
		test_file("test6")

func _process(delta):
	queue_redraw()
