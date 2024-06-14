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

func _linspace(start: float, end: float, N: int):
	var x: PackedFloat64Array = PackedFloat64Array()
	x.resize(N)

	var dx: float = (end - start) / (N - 1)
	var xi: float = start
	for i in range(N - 1):
		x[i] = xi
		xi += dx

	x[N-1] = end

	return x

func _cos(x: PackedFloat64Array, om: float = 1, A: float = 1):
	var y: PackedFloat64Array = PackedFloat64Array(x)

	for i in range(x.size()):
		y[i] = A * cos(om * x[i])

	return y

# Called when the node enters the scene tree for the first time.
var dataframe: PackedFloat64Array = PackedFloat64Array()
var lib_loaded: bool = false
func _ready():
	dataframe = PackedFloat64Array([
		2, 5, 2, 3, 10,
		3, 2, 10, 2, 6,
		4, 1, 12, 1, 6,
	])

	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		set_dataframe(dataframe, 5)
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

func _draw_commands():
	if lib_loaded:
		load_dataframe()
		test_file("histogram1")

		queue_redraw()

func _process(delta):
	pass
