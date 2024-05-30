extends GDMatPlotNative

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
		1, 25, 5, 2, 20, 1, 3, 10, 2,
	])

	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		set_dataframe(dataframe, 3)

func _draw():
	if lib_loaded:
		load_dataframe()
		test_file("errorbars")

func _process(delta):
	queue_redraw()
