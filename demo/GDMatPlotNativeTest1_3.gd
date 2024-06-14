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
		1, 85, 2, 90, 3, 110, 4, 250, 5, 1000, 6, 250, 7, 100, 8, 110, 9, 950, 10, 80, 11, 85, 12, 80, 13, 90,
	])

	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		set_dataframe(dataframe, 2)
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

func _draw_commands():
	if lib_loaded:
		load_dataframe()
		test_file("test5")

		queue_redraw()

func _process(delta):
	pass
