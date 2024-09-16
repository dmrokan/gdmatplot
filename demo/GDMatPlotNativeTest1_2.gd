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

var lib_loaded: bool = false
func _ready():
	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

func _draw_commands():
	if lib_loaded:
		test_file("airfoil1")

func _process(delta):
	queue_redraw()
