extends GDMatPlotNative

const xsize: float = 640
const ysize: float = 480

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
	var s = 0.05
	var rho = 3.85699
	for i in range(dataframe.size() / 2):
		dataframe[2*i+0] = i * 0.1
		dataframe[2*i+1] = s
		s = rho * s * (1 - s)

	var error = load_gnuplot()
	if !error:
		print("GNUPlot version " + get_gnuplot_version())
		lib_loaded = true
		set_dataframe(dataframe, 2)

var _figure_size: Vector2i = Vector2i(640, 480)
var _figure_size_inc: Vector2i = Vector2i(1, 1)
func _draw():
	if lib_loaded:
		load_dataframe()
		run_command("set terminal gdmp size %d,%d" % [_figure_size.x, _figure_size.y])
		run_command("set xtics 2")
		run_command("set grid y")
		run_command("set grid x")
		run_command("set yrange [-0.1:1.35]")
		run_command("set style fill solid 1.0")
		run_command("set encoding utf8")
		run_command("plot (sin(x)+1)/2 with lines, 'logistic map' with lines pt 'u'")

		_figure_size += _figure_size_inc

		if _figure_size.x > 640 or _figure_size.x < 426:
			_figure_size_inc *= -1

func _process(delta):
	queue_redraw()
