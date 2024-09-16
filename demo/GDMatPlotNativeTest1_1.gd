extends GDMatPlotNative

const _renderer_period: int = 10

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
	print("GDMatPlot version %08X" % get_version())
	print("GNUPlot version " + get_gnuplot_version())
	var x = _linspace(-1, 1, 256)
	var y = _cos(x, 2 * PI)

	dataframe.resize(400)
	var val = 0
	var val_inc = 0.05
	for i in range(dataframe.size() / 2):
		dataframe[2*i+0] = i * 0.1
		dataframe[2*i+1] = val
		val += val_inc
		if val >= 1 or val <= 0:
			val_inc *= -1

	var error = load_gnuplot()
	if !error:
		print("GNUPlot version " + get_gnuplot_version())
		lib_loaded = true
		set_dataframe(dataframe, 2)
		start_renderer(_draw_commands)
		set_rendering_period(_renderer_period)

var _figure_size: Vector2i = Vector2i(640, 480)
var _figure_size_inc: Vector2i = Vector2i(2, 2)
func _draw_commands():
	if lib_loaded:
		load_dataframe()
		run_command("set terminal gdmp size %d,%d" % [_figure_size.x, _figure_size.y])
		run_command("set xtics 2")
		run_command("set grid y")
		run_command("set grid x")
		run_command("set yrange [-0.1:1.35]")
		run_command("set style fill solid 1.0")
		run_command("set encoding utf8")
		run_command("plot (sin(x)+1)/2 with lines lw 2, 'sawtooth wave' with lines lw 4")

		_figure_size += _figure_size_inc

		if _figure_size.x > 640 or _figure_size.x < 426:
			_figure_size_inc *= -1

func _process(delta):
	queue_redraw()
