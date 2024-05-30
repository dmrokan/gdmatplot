/**************************************************************************/
/*  gdmatplot_backend.inc                                                 */
/**************************************************************************/
/* Copyright (c) 2024 Okan Demir                                          */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

void gd_matplot_options(void *self) {
	return SELF->options();
}

void gd_matplot_init(void *self) {
	return SELF->init();
}

void gd_matplot_reset(void *self) {
	return SELF->reset();
}

void gd_matplot_text(void *self) {
	return SELF->text();
}

int gd_matplot_scale(void *self, double x, double y) {
	return SELF->scale(x, y);
}

void gd_matplot_graphics(void *self) {
	return SELF->graphics();
}

void gd_matplot_move(void *self, unsigned int x, unsigned int y) {
	return SELF->move(x, y);
}

void gd_matplot_vector(void *self, unsigned int x, unsigned int y) {
	return SELF->vector(x, y);
}

void gd_matplot_linetype(void *self, int type) {
	return SELF->linetype(type);
}

void gd_matplot_put_text(void *self, unsigned int x, unsigned int y, const char *str) {
	return SELF->put_text(x, y, str);
}

int gd_matplot_text_angle(void *self, float angle) {
	return SELF->text_angle(angle);
}

int gd_matplot_justify_text(void *self, int type) {
	return SELF->justify_text(type);
}

void gd_matplot_point(void *self, unsigned int x, unsigned int y, int val) {
	return SELF->point(x, y, val);
}

void gd_matplot_arrow(void *self, unsigned int x, unsigned int y, unsigned int z,
		unsigned int val, int style) {
	return SELF->arrow(x, y, z, val, style);
}

int gd_matplot_set_font(void *self, const char *name, double size) {
	return SELF->set_font(name, size);
}

void gd_matplot_pointsize(void *self, double size) {
	return SELF->pointsize(size);
}

void gd_matplot_suspend(void *self) {
	return SELF->suspend();
}

void gd_matplot_resume(void *self) {
	return SELF->resume();
}

void gd_matplot_fillbox(void *self, int val, unsigned int x1, unsigned int y1,
		unsigned int width, unsigned int height) {
	return SELF->fillbox(val, x1, y1, width, height);
}

void gd_matplot_linewidth(void *self, double linewidth) {
	return SELF->linewidth(linewidth);
}

int gd_matplot_make_palette(void *self, void *palette) {
	return SELF->make_palette(palette);
}

void gd_matplot_previous_palette(void *self) {
	return SELF->previous_palette();
}

void gd_matplot_set_color(void *self, unsigned int color) {
	return SELF->set_color(color);
}

void gd_matplot_filled_polygon(void *self, int points, void *corners, int style) {
	return SELF->filled_polygon(points, corners, style);
}

void gd_matplot_image(void *self, unsigned int m, unsigned int n, void *coords,
		void *corners, unsigned int color_mode) {
	return SELF->image(m, n, coords, corners, color_mode);
}

void gd_matplot_enhanced_open(void *self, char *fontname, double fontsize,
		double base, char widthflag, char showflag, int overprint) {
	return SELF->enhanced_open(fontname, fontsize, base, widthflag, showflag, overprint);
}

void gd_matplot_enhanced_flush(void *self) {
	return SELF->enhanced_flush();
}

void gd_matplot_enhanced_writec(void *self, int c) {
	return SELF->enhanced_writec(c);
}

void gd_matplot_layer(void *self, unsigned int layer) {
	return SELF->layer(layer);
}

void gd_matplot_path(void *self, int p) {
	return SELF->path(p);
}

void gd_matplot_hypertext(void *self, int type, const char *text) {
	return SELF->hypertext(type, text);
}

void gd_matplot_boxed_text(void *self, unsigned int x, unsigned int y, int type) {
	return SELF->boxed_text(x, y, type);
}

void gd_matplot_modify_plots(void *self, unsigned int operations, int plotno) {
	return SELF->modify_plots(operations, plotno);
}

void gd_matplot_dashtype(void *self, int type, void *custom_dash_pattern) {
	return SELF->dashtype(type, custom_dash_pattern);
}

void gd_matplot_set_xmax(void *self, unsigned int val) {
	return SELF->set_xmax(val);
}

void gd_matplot_set_ymax(void *self, unsigned int val) {
	return SELF->set_ymax(val);
}

void gd_matplot_set_h_tic(void *self, unsigned int val) {
	return SELF->set_h_tic(val);
}

void gd_matplot_set_v_tic(void *self, unsigned int val) {
	return SELF->set_v_tic(val);
}

void gd_matplot_set_color_mode(void *self, unsigned char val) {
	return SELF->set_color_mode(val);
}

void gd_matplot_set_linetype(void *self, int val) {
	return SELF->set_linetype(val);
}

void gd_matplot_set_dashpattern(void *self, const char *val) {
	return SELF->set_dashpattern(val);
}

void gd_matplot_set_h_char(void *self, unsigned int val) {
	return SELF->set_h_char(val);
}

void gd_matplot_set_v_char(void *self, unsigned int val) {
	return SELF->set_v_char(val);
}

void gd_matplot_set_gridline(void *self, unsigned int val) {
	return SELF->set_gridline(val);
}

void gd_matplot_set_hasgrid(void *self, unsigned int val) {
	return SELF->set_hasgrid(val);
}

void gd_matplot_set_plotno(void *self, unsigned int val) {
	return SELF->set_plotno(val);
}

void gd_matplot_set_fill_pattern(void *self, int val) {
	return SELF->set_fill_pattern(val);
}

void gd_matplot_set_fill_pattern_index(void *self, unsigned int val) {
	return SELF->set_fill_pattern_index(val);
}

void gd_matplot_set_rgb(void *self, unsigned int val) {
	return SELF->set_rgb(val);
}

void gd_matplot_set_patterncolor(void *self, unsigned int *val) {
	return SELF->set_patterncolor(val);
}

void gd_matplot_set_group_filled_is_open(void *self, unsigned char val) {
	return SELF->set_group_filled_is_open(val);
}

void gd_matplot_set_in_textbox(void *self, unsigned char val) {
	return SELF->set_in_textbox(val);
}

void gd_matplot_set_xsize(void *self, unsigned int val) {
	return SELF->set_xsize(val);
}

void gd_matplot_set_ysize(void *self, unsigned int val) {
	return SELF->set_ysize(val);
}

void gd_matplot_set_xlast(void *self, unsigned int val) {
	return SELF->set_xlast(val);
}

void gd_matplot_set_ylast(void *self, unsigned int val) {
	return SELF->set_ylast(val);
}

void gd_matplot_set_linecap(void *self, int val) {
	return SELF->set_linecap(val);
}

void gd_matplot_set_group_is_open(void *self, unsigned char val) {
	return SELF->set_group_is_open(val);
}

void gd_matplot_set_path_is_open(void *self, unsigned char val) {
	return SELF->set_path_is_open(val);
}

void gd_matplot_set_fontscale(void *self, double val) {
	return SELF->set_fontscale(val);
}

void gd_matplot_set_dashlength(void *self, double val) {
	return SELF->set_dashlength(val);
}

void gd_matplot_set_name(void *self, const char *val) {
	return SELF->set_name(val);
}

void gd_matplot_set_linewidth_factor(void *self, double val) {
	return SELF->set_linewidth_factor(val);
}

void gd_matplot_set_background(void *self, int val) {
	return SELF->set_background(val);
}

void gd_matplot_set_linecolor(void *self, const char *val) {
	return SELF->set_linecolor(val);
}

void gd_matplot_set_alpha(void *self, double val) {
	return SELF->set_alpha(val);
}

void gd_matplot_set_term_pointsize(void *self, double val) {
	return SELF->set_term_pointsize(val);
}

void gd_matplot_set_stroke_width(void *self, double val) {
	return SELF->set_stroke_width(val);
}

void gd_matplot_set_pen(void *self, unsigned int index, unsigned int color, double width) {
	return SELF->set_pen(index, color, width);
}

struct GDMatPlotMethods {
	void (*options)(void *);
	void (*init)(void *);
	void (*reset)(void *);
	void (*text)(void *);
	int (*scale)(void *, double, double);
	void (*graphics)(void *);
	void (*move)(void *, unsigned int, unsigned int);
	void (*vector)(void *, unsigned int, unsigned int);
	void (*linetype)(void *, int);
	void (*put_text)(void *, unsigned int, unsigned int, const char *);
	/* the following are optional. set term ensures they are not NULL */
	int (*text_angle)(void *, float); /* Changed from int to double in version 5.5 */
	int (*justify_text)(void *, int);
	void (*point)(void *, unsigned int, unsigned int, int);
	void (*arrow)(void *, unsigned int, unsigned int, unsigned int, unsigned int, int headstyle);
	int (*set_font)(void *, const char *, double);
	void (*pointsize)(void *, double); /* change pointsize */
	void (*suspend)(void *); /* called after one plot of multiplot */
	void (*resume)(void *); /* called before plots of multiplot */
	void (*fillbox)(void *, int, unsigned int, unsigned int, unsigned int, unsigned int); /* clear in multiplot mode */
	void (*linewidth)(void *, double linewidth);

	int (*make_palette)(void *, void *palette);
	/* 1. if palette==NULL, then return nice/suitable
	   maximal number of colours supported by this terminal.
	   Returns 0 if it can make colours without palette (like
	   postscript).
	   2. if palette!=NULL, then allocate its own palette
	   return value is undefined
	   3. available: some negative values of max_colors for whatever
	   can be useful
	 */
	void (*previous_palette)(void *);
	/* release the palette that the above routine allocated and get
	   back the palette that was active before.
	   Some terminals, like displays, may draw parts of the figure
	   using their own palette. Those terminals that possess only
	   one palette for the whole plot don't need this routine.
	 */
	void (*set_color)(void *, unsigned int);
	/* EAM November 2004 - revised to take a pointer to struct rgb_color,
	   so that a palette gray value is not the only option for
	   specifying color.
	 */
	void (*filled_polygon)(void *, int, void *, int);
	void (*image)(void *, unsigned int, unsigned int, void *, void *, unsigned int);

	/* Enhanced text mode driver call-backs */
	void (*enhanced_open)(void *, char *fontname, double fontsize,
			double base, char widthflag, char showflag,
			int overprint);
	void (*enhanced_flush)(void *);
	void (*enhanced_writec)(void *, int c);

	/* Driver-specific synchronization or other layering commands.
	 * Introduced as an alternative to the ugly sight of
	 * driver-specific code strewn about in the core routines.
	 * As of this point (July 2005) used only by pslatex.trm
	 */
	void (*layer)(void *, unsigned int);

	/* Begin/End path control.
	 * Needed by PostScript-like devices in order to join the endpoints of
	 * a polygon cleanly.
	 */
	void (*path)(void *, int p);

	/* Pass hypertext for inclusion in the output plot */
	void (*hypertext)(void *, int type, const char *text);

	void (*boxed_text)(void *, unsigned int, unsigned int, int);

	void (*modify_plots)(void *, unsigned int operations, int plotno);

	void (*dashtype)(void *, int type, void *custom_dash_pattern);

	/* ======================================== */

	void (*set_xmax)(void *, unsigned int);
	void (*set_ymax)(void *, unsigned int);
	void (*set_h_tic)(void *, unsigned int);
	void (*set_v_tic)(void *, unsigned int);

	void (*set_color_mode)(void *, unsigned char);
	void (*set_linetype)(void *, int);
	void (*set_dashpattern)(void *, const char *);

	void (*set_h_char)(void *, unsigned int);
	void (*set_v_char)(void *, unsigned int);

	void (*set_gridline)(void *, unsigned int);
	void (*set_hasgrid)(void *, unsigned int);
	void (*set_plotno)(void *, unsigned int);

	void (*set_fill_pattern)(void *, int);
	void (*set_fill_pattern_index)(void *, unsigned int);
	void (*set_rgb)(void *, unsigned int);
	void (*set_patterncolor)(void *, unsigned int *);
	void (*set_group_filled_is_open)(void *, unsigned char);
	void (*set_in_textbox)(void *, unsigned char);
	void (*set_xsize)(void *, unsigned int);
	void (*set_ysize)(void *, unsigned int);
	void (*set_xlast)(void *, unsigned int);
	void (*set_ylast)(void *, unsigned int);
	void (*set_linecap)(void *, int);
	void (*set_group_is_open)(void *, unsigned char);
	void (*set_path_is_open)(void *, unsigned char);

	void (*set_fontscale)(void *, double);
	void (*set_dashlength)(void *, double);
	void (*set_name)(void *, const char *);
	void (*set_linewidth_factor)(void *, double);
	void (*set_background)(void *, int);

	void (*set_linecolor)(void *, const char *);
	void (*set_alpha)(void *, double);

	void (*set_term_pointsize)(void *, double);
	void (*set_stroke_width)(void *, double);

	void (*set_pen)(void *, unsigned int, unsigned int, double);

	/* ======================================== */

	/* Scale factor for converting terminal coordinates to output
	 * pixel coordinates.  Used to provide data for external mousing code.
	 */
	double tscale;
	int flags;

	GDMatPlotMethods() {
		options = gd_matplot_options;
		init = gd_matplot_init;
		reset = gd_matplot_reset;
		text = gd_matplot_text;
		scale = gd_matplot_scale;
		graphics = gd_matplot_graphics;
		move = gd_matplot_move;
		vector = gd_matplot_vector;
		linetype = gd_matplot_linetype;
		put_text = gd_matplot_put_text;
		text_angle = gd_matplot_text_angle;
		justify_text = gd_matplot_justify_text;
		point = gd_matplot_point;
		arrow = gd_matplot_arrow;
		set_font = gd_matplot_set_font;
		pointsize = gd_matplot_pointsize;
		flags = 0;
		suspend = gd_matplot_suspend;
		resume = gd_matplot_resume;
		fillbox = gd_matplot_fillbox;
		linewidth = gd_matplot_linewidth;
		make_palette = gd_matplot_make_palette;
		previous_palette = gd_matplot_previous_palette;
		set_color = gd_matplot_set_color;
		filled_polygon = gd_matplot_filled_polygon;
		image = gd_matplot_image;
		enhanced_open = gd_matplot_enhanced_open;
		enhanced_flush = gd_matplot_enhanced_flush;
		enhanced_writec = gd_matplot_enhanced_writec;
		layer = gd_matplot_layer;
		path = gd_matplot_path;
		tscale = 1;
		hypertext = gd_matplot_hypertext;
		boxed_text = gd_matplot_boxed_text;
		modify_plots = gd_matplot_modify_plots;
		dashtype = gd_matplot_dashtype;

		/* ======================================== */

		set_xmax = gd_matplot_set_xmax;
		set_ymax = gd_matplot_set_ymax;
		set_h_tic = gd_matplot_set_h_tic;
		set_v_tic = gd_matplot_set_v_tic;

		set_color_mode = gd_matplot_set_color_mode;
		set_linetype = gd_matplot_set_linetype;
		set_dashpattern = gd_matplot_set_dashpattern;

		set_h_char = gd_matplot_set_h_char;
		set_v_char = gd_matplot_set_v_char;

		set_gridline = gd_matplot_set_gridline;
		set_hasgrid = gd_matplot_set_hasgrid;
		set_plotno = gd_matplot_set_plotno;

		set_fill_pattern = gd_matplot_set_fill_pattern;
		set_fill_pattern_index = gd_matplot_set_fill_pattern_index;
		set_rgb = gd_matplot_set_rgb;
		set_patterncolor = gd_matplot_set_patterncolor;
		set_group_filled_is_open = gd_matplot_set_group_filled_is_open;
		set_in_textbox = gd_matplot_set_in_textbox;
		set_xsize = gd_matplot_set_xsize;
		set_ysize = gd_matplot_set_ysize;
		set_xlast = gd_matplot_set_xlast;
		set_ylast = gd_matplot_set_ylast;
		set_linecap = gd_matplot_set_linecap;
		set_group_is_open = gd_matplot_set_group_is_open;
		set_path_is_open = gd_matplot_set_path_is_open;

		set_fontscale = gd_matplot_set_fontscale;
		set_dashlength = gd_matplot_set_dashlength;
		set_name = gd_matplot_set_name;
		set_linewidth_factor = gd_matplot_set_linewidth_factor;
		set_background = gd_matplot_set_background;

		set_linecolor = gd_matplot_set_linecolor;
		set_alpha = gd_matplot_set_alpha;

		set_term_pointsize = gd_matplot_set_term_pointsize;
		set_stroke_width = gd_matplot_set_stroke_width;

		set_pen = gd_matplot_set_pen;

		/* ======================================== */
	}
};

struct GDMatPlotGNUPlotInterface {
	GDMatPlotMethods methods;
	GDMatPlot *self;
};
