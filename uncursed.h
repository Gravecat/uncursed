/* uncused.h -- Uncursed, a C++ front-end library to make NCurses/PDCurses less painful to use.
   RELEASE VERSION 1.0 -- 13th December 2019

MIT License

Copyright (c) 2019 Raine Simmons.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

//#define USING_GURU_MEDITATION	// Uncomment this line if you are also using my Guru Meditation library.
//#define USING_POTLUCK			// Uncomment this line if you are also using my Potluck utility library.


#if defined(_WIN32) || defined(_WIN64)
typedef struct _win WINDOW;
#else
typedef struct _win_st WINDOW;
#endif
typedef struct panel PANEL;

#include <memory>
#include <string>

namespace unc
{

// If we're NOT using the Guru Meditation system, this'll just make the stack_trace() call do nothing.
#ifndef USING_GURU_MEDITATION
#define stack_trace() (void)0
#endif

enum class Colour : unsigned int { NONE, BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

enum class Glyph : unsigned int { ULCORNER = 256, LLCORNER, URCORNER, LRCORNER, RTEE, LTEE, BTEE, TTEE, HLINE, VLINE, PLUS, S1, S9, DIAMOND, CKBOARD, DEGREE, PLMINUS, BULLET, LARROW, RARROW, DARROW, UARROW, BOARD, LANTERN, BLOCK,
	S3, S7, LEQUAL, GEQUAL, PI, NEQUAL, STERLING };

#define UNC_BOLD	1	// The specified string should be printed in bold.
#define UNC_NL		2	// There should be a new-line added to the end of the string.
#define UNC_RAW		4	// The string should not be processed or formatted, just printed as-is.
#define UNC_REVERSE	8	// The string's colours should be inverted.
#define UNC_DOUBLE	16	// Renders a char twice, side-by-side.
#define UNC_BLINK	32	// Blinking colour effect.

class Window
{
public:
					Window(unsigned int width, unsigned int height, int new_x = 0, int new_y = 0, bool new_border = false);
					~Window();
	unsigned int	get_height() const { return h; }	// Read-only access to the Window's height.
	unsigned int	get_width() const { return w; }		// Read-only access to the Window's width.
	void			redraw_border(unc::Colour col = unc::Colour::NONE);	// Re-renders the border around this Window, if any.
	void			set_visible(bool vis);				// Set this Window's panel as visible or invisible.
	void			move(int new_x, int new_y);			// Moves this Window's underlying panel to new coordinates.
	WINDOW*			win() const { return window_ptr; }	// Returns a pointer to the WINDOW struct.

private:
	std::shared_ptr<unc::Window>	border_ptr;	// If a border is present, this is the underlying border Window.
	PANEL*			panel_ptr;	// A pointer to the underlying PANEL struct.
	unsigned int	w, h;		// The width and height of this Window.
	WINDOW*			window_ptr;	// A pointer to the underlying WINDOW struct.
	int				x, y;		// The screen coordinates of this Window.
};

void			box(std::shared_ptr<unc::Window> window = nullptr, unc::Colour colour = unc::Colour::NONE, unsigned int flags = 0);	// Draws a box around the edge of a Window.
void			clear_line(std::shared_ptr<unc::Window> window = nullptr);	// Clears the current line.
void			cls(std::shared_ptr<unc::Window> window = nullptr);		// Clears the screen.
void			flip();		// Refreshes the screen.
void			flush();	// Flushes the input buffer.
unsigned int	get_cols(std::shared_ptr<unc::Window> window = nullptr);		// Gets the number of columns available on the screen right now.
unsigned int	get_cursor_x(std::shared_ptr<unc::Window> window = nullptr);	// Gets the current cursor X coordinate.
unsigned int	get_cursor_y(std::shared_ptr<unc::Window> window = nullptr);	// Gets the current cursor Y coordinate.
int				get_key(std::shared_ptr<unc::Window> window = nullptr);		// Gets a keypress as input.
unsigned int	get_midcol(std::shared_ptr<unc::Window> window = nullptr);		// Gets the central column of the specified Window.
unsigned int	get_midrow(std::shared_ptr<unc::Window> window = nullptr);		// Gets the central row of the specified Window.
unsigned int	get_rows(std::shared_ptr<unc::Window> window = nullptr);		// Gets the number of rows available on the screen right now.
std::string		get_string(std::shared_ptr<unc::Window> window = nullptr);		// C++ std::string wrapper around the PDCurses wgetnstr() function.
void			init();				// Sets up Curses.
void			init_colours();		// Sets up the Curses colour pairs.
bool			is_cancel(int key);	// Checks if a key is a cancel key (escape).
bool			is_down(int key);	// Checks if a key is the down arrow key.
bool			is_left(int key);	// Checks if a key is the left arrow key.
bool			is_right(int key);	// Checks if a key is the right arrow key.
bool			is_select(int key);	// Checks if a key is a select key (space bar or enter).
bool			is_up(int key);		// Checks if a key is the up arrow key.
void			move_cursor(int x, int y, std::shared_ptr<unc::Window> window = nullptr);	// Moves the cursor to the given coordinates; -1 for either coordinate retains its current position on that axis.
				// Prints a string on the screen, with optional word-wrap.
void			print(std::string input, unc::Colour colour = unc::Colour::NONE, unsigned int flags = 0, int x = -1, int y = -1, std::shared_ptr<unc::Window> window = nullptr);
void			print(int input = '\n', unc::Colour colour = unc::Colour::NONE, unsigned int flags = 0, int x = -1, int y = -1, std::shared_ptr<unc::Window> window = nullptr);	// As above, but for a single character.
void			print(unc::Glyph input, unc::Colour colour = unc::Colour::NONE, unsigned int flags = 0, int x = -1, int y = -1, std::shared_ptr<unc::Window> window = nullptr);	// Simple wrapper for high-ASCII glyphs.
void			print(std::shared_ptr<unc::Window> window, int newline_count = 1);	// This just makes it easier to do a newline print() on a Window.
void			render_grid(int x, int y, int w, int h, unc::Colour colour = unc::Colour::NONE, std::shared_ptr<unc::Window> window = nullptr);	// Renders a grid of the specified size.
int				resize_key();				// Access to the KEY_RESIZE definition in curses.h
void			set_cursor(bool enabled);	// Turns the cursor on or off.
void			shutdown();					// Runs Curses cleanup code.

}	// namespace unc
