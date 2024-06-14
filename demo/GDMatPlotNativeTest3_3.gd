extends GDMatPlotNative

const _renderer_period: int = 100

var _lines: Array = []
var _om: float = 1
var _om_inc: float = 1e-2

func test_file(fn: String):
	if _lines.is_empty():
		var file = FileAccess.open("res://" + fn + ".txt", FileAccess.READ)
		while not file.eof_reached():
			var line = file.get_line()
			_lines.append(line)

	var i = 0
	for line in _lines:
		if i == 1:
			run_command("om = %f" % [ _om ])
		run_command(line)

		i += 1

	_om += _om_inc

	if _om > 8 or _om < 0.5:
		_om_inc *= -1

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
	var x = _linspace(-1, 1, 256)
	var y = _cos(x, 2 * PI)

	dataframe.resize(200)
	var s = 0.5
	var rho = 3.5699
	for i in range(dataframe.size() / 2):
		dataframe[2*i+0] = i * 0.1
		dataframe[2*i+1] = 0.7 * sin(2 * PI * i * 0.1)

	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		set_dataframe(dataframe, 2)
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

func _draw_commands():
	if lib_loaded:
		load_dataframe()
		test_file("contour2")

		queue_redraw()

func _process(delta):
	pass
