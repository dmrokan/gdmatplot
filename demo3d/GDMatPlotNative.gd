extends GDMatPlotNative

var _lines: Array = []
var _param: float = 1
var _param_bounds: Array = [ -INF, +INF ]
var _param_inc = null
var _redraw_period: float = 1e-1
var _gnuplot_script_filename: String = ""
var _disable_draw: bool = true

func test_file(fn: String):
	if _lines.is_empty():
		var file = FileAccess.open("res://" + fn + ".txt", FileAccess.READ)
		while not file.eof_reached():
			var line = file.get_line()
			_lines.append(line)

	var i = 0
	for line in _lines:
		if i == 1:
			run_command("param = %f" % [ _param ])
		run_command(line)

		i += 1

	if _param_inc != null:
		_param += _param_inc

		if _param > _param_bounds[1] or _param < _param_bounds[0]:
			_param_inc *= -1

# Called when the node enters the scene tree for the first time.
var dataframe: PackedFloat64Array = PackedFloat64Array()
var lib_loaded: bool = false
func _load_library():
	var error = load_gnuplot()
	if !error:
		lib_loaded = true
		start_renderer(_draw_commands)

func _draw_commands():
	if not _gnuplot_script_filename.is_empty():
		test_file(_gnuplot_script_filename)

	queue_redraw()

func _ready():
	_load_library()

func _process(delta):
	pass

func set_gnuplot_script_and_params(fn: String, inc: float, bounds: Array, redraw_period: float = 1.0):
	_gnuplot_script_filename = fn
	_param_inc = inc
	_param_bounds[0] = bounds[0]
	_param_bounds[1] = bounds[1]
	_redraw_period = redraw_period

func enable_draw(enable: bool = true):
	_disable_draw = not enable
	if enable:
		set_rendering_period(int(1e3 * _redraw_period))
	else:
		set_rendering_period(1000000000)

