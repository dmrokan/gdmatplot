extends Node3D


func _ready():
	var mesh = $Quad.get_mesh()
	$VisibleOnScreenNotifier3D.aabb = mesh.get_aabb()

	$VisibleOnScreenNotifier3D.screen_entered.connect(func(): $SubViewport/GDMatPlotNative.enable_draw())
	$VisibleOnScreenNotifier3D.screen_exited.connect(func(): $SubViewport/GDMatPlotNative.enable_draw(false))

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass

func set_gnuplot_script_and_params(fn: String, inc: float, bounds: Array, redraw_period: float = -1.0):
	$SubViewport/GDMatPlotNative.set_gnuplot_script_and_params(fn, inc, bounds, redraw_period)
