extends TabBar

@onready var tab1 = get_node("%Tab1")
@onready var tab2 = get_node("%Tab2")
@onready var tab3 = get_node("%Tab3")
@onready var tab_list = [ tab1, tab2, tab3 ]

# Called when the node enters the scene tree for the first time.
func _ready():
	_hide_all_tabs_except(get_current_tab())

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass

func _hide_all_tabs_except(index):
	var i = 0
	for tab in tab_list:
		if i == index:
			tab.show()
		else:
			tab.hide()

		i += 1

func _on_tab_changed(tab):
	_hide_all_tabs_except(tab)
