/* uncused.cpp -- Uncursed, a C++ front-end library to make NCurses/PDCurses less painful to use.
   RELEASE VERSION 1.3 -- 16th December 2019

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

#include <curses.h>
#include <panel.h>
#include <vector>

#include "uncursed.h"

#ifdef USING_GURU_MEDITATION
#include "guru/guru.h"
#endif
#ifdef USING_POTLUCK
#include "potluck/potluck.h"
#else
#include <algorithm>
#endif

namespace unc
{

unsigned int	cursor_state = 1;	// The current state of the cursor.


Window::Window(unsigned int width, unsigned int height, int new_x, int new_y, bool new_border) : border_ptr(nullptr)
{
	stack_trace();
	if (new_border)
	{
		border_ptr = std::make_shared<unc::Window>(width, height, new_x, new_y, 0);
		width -= 4;
		height -= 2;
		new_x += 2;
		new_y += 1;
	}
	w = width;
	h = height;
	x = new_x;
	y = new_y;
	window_ptr = newwin(height, width, new_y, new_x);
	panel_ptr = new_panel(window_ptr);
}

Window::~Window()
{
	stack_trace();
	del_panel(panel_ptr);
	delwin(window_ptr);
}

// Moves this Window's underlying panel to new coordinates.
void Window::move(int new_x, int new_y)
{
	stack_trace();
	x = new_x;
	y = new_y;
	move_panel(panel_ptr, y, x);
}

// Re-renders the border around this Window, if any.
void Window::redraw_border(Colour col)
{
	if (border_ptr) unc::box(border_ptr, col);
#ifdef USING_GURU_MEDITATION
	else guru::nonfatal("Attempt to re-render window border, with no border defined.", GURU_WARN);
#endif
}

// Set this Window's panel as visible or invisible.
void Window::set_visible(bool vis)
{
	stack_trace();
	if (vis) show_panel(panel_ptr);
	else hide_panel(panel_ptr);
}


// Draws a box around the edge of a Window.
void box(std::shared_ptr<unc::Window> window, unc::Colour colour, unsigned int flags)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	const bool bold = ((flags & UNC_BOLD) == UNC_BOLD);
	const bool reverse = ((flags & UNC_REVERSE) == UNC_REVERSE);
	const bool blink = ((flags & UNC_BLINK) == UNC_BLINK);

	unsigned int colour_flags = 0;
	if (bold) colour_flags |= A_BOLD;
	if (reverse) colour_flags |= A_REVERSE;
	if (blink) colour_flags |= A_BLINK;

	if (colour != unc::Colour::NONE) wattron(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
	::box(win, 0, 0);
	if (colour != unc::Colour::NONE) wattroff(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
}

// Clears the current line.
void clear_line(std::shared_ptr<Window> window)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	wclrtoeol(win);
}

// Clears the screen.
void cls(std::shared_ptr<unc::Window> window)
{
	stack_trace();
#ifdef PDCURSES
	// Workaround to deal with PDCurses' lack of an inbuilt SIGWINCH handler.
	if (is_termresized())
	{
		resize_term(0, 0);
		if (unc::get_cols() < 80 || unc::get_rows() < 24) resize_term(24, 80);
		curs_set(cursor_state);
	}
#endif
	if (!window) clear();
	else wclear(window->win());
}

// Refreshes the screen.
void flip()
{
	stack_trace();
	if (unc::get_cols() < 80 || unc::get_rows() < 24) resize_term(24, 80);
	update_panels();
	refresh();
}

// Flushes the input buffer.
void flush()
{
	stack_trace();
	flushinp();
}

// Gets the number of columns available on the screen right now.
unsigned int get_cols(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	if (window) return window->get_width();
	else return getmaxx(stdscr);
}

// Gets the current cursor X coordinate.
unsigned int get_cursor_x(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	return getcurx(win);
}

// Gets the current cursor Y coordinate.
unsigned int get_cursor_y(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	return getcury(win);
}

// Gets a keypress as input.
int get_key(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	int key = wgetch(win);
	if (key == KEY_RESIZE)
	{
		resize_term(0, 0);
		if (unc::get_cols() < 80 || unc::get_rows() < 24) resize_term(24, 80);
		curs_set(cursor_state);
	}
	return key;
}

// Gets the central column of the specified unc::Window.
unsigned int get_midcol(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	return unc::get_cols(window) / 2;
}

// Gets the central row of the specified unc::Window.
unsigned int get_midrow(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	return unc::get_rows(window) / 2;
}

// Gets the number of rows available on the screen right now.
unsigned int get_rows(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	if (window) return window->get_height();
	else return getmaxy(stdscr);
}

// C++ std::string wrapper around the PDCurses wgetnstr() function.
std::string get_string(std::shared_ptr<unc::Window> window)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	char buffer[256];
	wgetnstr(win, buffer, 255);
	return buffer;
}

// Sets up Curses.
void init(std::string syslog_filename)
{
	stack_trace();
#ifdef USING_GURU_MEDITATION
#if GURU_MEDITATION_VERSION >= 111
	guru::open_syslog(syslog_filename);
#else
	guru::open_sysog();
#endif
#endif
	initscr();
	cbreak();
	unc::set_cursor(true);
	keypad(stdscr, true);
	unc::init_colours();

#ifdef USING_GURU_MEDITATION
	guru::console_ready(true);
#endif
}

// Sets up the Curses colour pairs.
void init_colours()
{
	stack_trace();
	if (!has_colors()) return;
	start_color();
	init_pair(static_cast<unsigned int>(unc::Colour::BLACK), COLOR_BLACK, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::RED), COLOR_RED, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::GREEN), COLOR_GREEN, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::YELLOW), COLOR_YELLOW, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::BLUE), COLOR_BLUE, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::MAGENTA), COLOR_MAGENTA, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::CYAN), COLOR_CYAN, COLOR_BLACK);
	init_pair(static_cast<unsigned int>(unc::Colour::WHITE), COLOR_WHITE, COLOR_BLACK);
}

// Checks if a key is a cancel key (escape).
bool is_cancel(int key)
{
	return (key == 27);
}

// Checks if a key is the down arrow key.
bool is_down(int key)
{
	return (key == KEY_DOWN || key == 's' || key == 'S');
}

// Checks if a key is the left arrow key.
bool is_left(int key)
{
	return (key == KEY_LEFT || key == 'a' || key == 'A');
}

// Checks if a key is the right arrow key.
bool is_right(int key)
{
	return (key == KEY_RIGHT || key == 'd' || key == 'D');
}

// Checks if a key is a select key (space bar or enter).
bool is_select(int key)
{
	return (key == ' ' || key == '\n' || key == '\r');
}

// Checks if a key is the up arrow key.
bool is_up(int key)
{
	return (key == KEY_UP || key == 'w' || key == 'W');
}

// Moves the cursor to the given coordinates; -1 for either coordinate retains its current position on that axis.
void move_cursor(int x, int y, std::shared_ptr<unc::Window> window)
{
	stack_trace();
	if (x == -1 && y == -1) return;
	WINDOW *win = (window ? window->win() : stdscr);
	const int old_x = unc::get_cursor_x(window);
	const int old_y = unc::get_cursor_y(window);
	if (x == -1) x = old_x;
	if (y == -1) y = old_y;
	wmove(win, y, x);
}

// Parses a string into a Colour, or Colour::NONE if it could not be parsed.
Colour parse_colour(std::string input)
{
	stack_trace();
	if (!input.size()) return Colour::NONE;
#ifdef USING_POTLUCK
	input = potluck::str_toupper(input);
#else
	std::transform(input.begin(), input.end(), input.begin(), ::toupper);
#endif

	if (input == "BLACK") return Colour::BLACK;
	else if (input == "RED") return Colour::RED;
	else if (input == "GREEN") return Colour::GREEN;
	else if (input == "YELLOW") return Colour::YELLOW;
	else if (input == "BLUE") return Colour::BLUE;
	else if (input == "MAGENTA") return Colour::MAGENTA;
	else if (input == "CYAN") return Colour::CYAN;
	else if (input == "WHITE") return Colour::WHITE;
	else return Colour::NONE;
}

// Parses a string into flags (such as UNC_BOLD | UNC_REVERSE), or 0 if nothing could be parsed from the string.
unsigned int parse_flags(std::string input)
{
	stack_trace();
	if (!input.size()) return 0;
#ifdef USING_POTLUCK
	input = potluck::str_toupper(input);
#else
	std::transform(input.begin(), input.end(), input.begin(), ::toupper);
#endif

	unsigned int flags = 0;
	if (input.find("BOLD") != std::string::npos) flags |= UNC_BOLD;
	if (input.find("NL") != std::string::npos) flags |= UNC_NL;
	if (input.find("RAW") != std::string::npos) flags |= UNC_RAW;
	if (input.find("REVERSE") != std::string::npos) flags |= UNC_REVERSE;
	if (input.find("DOUBLE") != std::string::npos) flags |= UNC_DOUBLE;
	if (input.find("BLINK") != std::string::npos) flags |= UNC_BLINK;
	return flags;
}

// Prints a string on the screen, with optional word-wrap.
void print(std::string input, unc::Colour colour, unsigned int flags, int x, int y, std::shared_ptr<unc::Window> window)
{
	stack_trace();
	if (!input.size()) return;

	WINDOW *win = (window ? window->win() : stdscr);
	const bool bold = ((flags & UNC_BOLD) == UNC_BOLD);
	const bool newline = ((flags & UNC_NL) == UNC_NL);
	const bool raw = ((flags & UNC_RAW) == UNC_RAW);
	const bool reverse = ((flags & UNC_REVERSE) == UNC_REVERSE);
	const bool no_colour = (colour == unc::Colour::NONE);
	const bool blink = ((flags & UNC_BLINK) == UNC_BLINK);
	unc::move_cursor(x, y, window);

	unsigned int colour_flags = 0;
	if (bold) colour_flags |= A_BOLD;
	if (reverse) colour_flags |= A_REVERSE;
	if (blink) colour_flags |= A_BLINK;

	if (raw)
	{
		if (!no_colour) wattron(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
		waddstr(win, input.c_str());
		if (newline) waddch(win, '\n');
		if (!no_colour) wattroff(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
		return;
	}

	unsigned int spaces_at_start = 0;
	while (input.size() && input[0] == ' ')
	{
		input = input.substr(1);
		spaces_at_start++;
	}
	const unsigned int width = unc::get_cols(window);
	unsigned int current_pos = unc::get_cursor_x(window);
	std::vector<std::string> words = unc::string_explode(input, " ");
	std::string line;
	if (words.size() && spaces_at_start) words.at(0) = std::string(spaces_at_start, ' ') + words.at(0);
	if (!no_colour) wattron(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
	while (words.size())
	{
		std::string word = words.at(0);
		words.erase(words.begin());
		if (line.size() + word.size() + current_pos >= width)
		{
			waddstr(win, line.c_str());
			line = word;
			current_pos = 0;
			if (unc::get_cursor_x(window) != 0) waddch(win, '\n');
		}
		else
		{
			if (line.size()) line += " " + word;
			else line = word;
		}
	}
	if (line.size()) waddstr(win, (line).c_str());
	if (newline && unc::get_cursor_x(window) != 0) waddch(win, '\n');
	if (!no_colour) wattroff(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
}

// As above, but for a single character.
void print(int input, unc::Colour colour, unsigned int flags, int x, int y, std::shared_ptr<unc::Window> window)
{
	stack_trace();
	WINDOW *win = (window ? window->win() : stdscr);
	const bool bold = ((flags & UNC_BOLD) == UNC_BOLD);
	const bool reverse = ((flags & UNC_REVERSE) == UNC_REVERSE);
	const bool no_colour = (colour == unc::Colour::NONE);
	const bool render_double = ((flags & UNC_DOUBLE) == UNC_DOUBLE);
	const bool blink = ((flags & UNC_BLINK) == UNC_BLINK);
	unc::move_cursor(x, y, window);

	unsigned int colour_flags = 0;
	if (bold) colour_flags |= A_BOLD;
	if (reverse) colour_flags |= A_REVERSE;
	if (blink) colour_flags |= A_BLINK;

	if (input > 255)
	{
		switch(static_cast<unc::Glyph>(input))
		{
			case unc::Glyph::ULCORNER: input = ACS_ULCORNER; break;
			case unc::Glyph::LLCORNER: input = ACS_LLCORNER; break;
			case unc::Glyph::URCORNER: input = ACS_URCORNER; break;
			case unc::Glyph::LRCORNER: input = ACS_LRCORNER; break;
			case unc::Glyph::RTEE: input = ACS_RTEE; break;
			case unc::Glyph::LTEE: input = ACS_LTEE; break;
			case unc::Glyph::BTEE: input = ACS_BTEE; break;
			case unc::Glyph::TTEE: input = ACS_TTEE; break;
			case unc::Glyph::HLINE: input = ACS_HLINE; break;
			case unc::Glyph::VLINE: input = ACS_VLINE; break;
			case unc::Glyph::PLUS: input = ACS_PLUS; break;
			case unc::Glyph::S1: input = ACS_S1; break;
			case unc::Glyph::S9: input = ACS_S9; break;
			case unc::Glyph::DIAMOND: input = ACS_DIAMOND; break;
			case unc::Glyph::CKBOARD: input = ACS_CKBOARD; break;
			case unc::Glyph::DEGREE: input = ACS_DEGREE; break;
			case unc::Glyph::PLMINUS: input = ACS_PLMINUS; break;
			case unc::Glyph::BULLET: input = ACS_BULLET; break;
			case unc::Glyph::LARROW: input = ACS_LARROW; break;
			case unc::Glyph::RARROW: input = ACS_RARROW; break;
			case unc::Glyph::DARROW: input = ACS_DARROW; break;
			case unc::Glyph::UARROW: input = ACS_UARROW; break;
			case unc::Glyph::BOARD: input = ACS_BOARD; break;
			case unc::Glyph::LANTERN: input = ACS_LANTERN; break;
			case unc::Glyph::BLOCK: input = ACS_BLOCK; break;
			case unc::Glyph::S3: input = ACS_S3; break;
			case unc::Glyph::S7: input = ACS_S7; break;
			case unc::Glyph::LEQUAL: input = ACS_LEQUAL; break;
			case unc::Glyph::GEQUAL: input = ACS_GEQUAL; break;
			case unc::Glyph::PI: input = ACS_PI; break;
			case unc::Glyph::NEQUAL: input = ACS_NEQUAL; break;
			case unc::Glyph::STERLING: input = ACS_STERLING; break;
		}
	}

	if (!no_colour) wattron(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
	waddch(win, input);
	if (render_double) waddch(win, input);
	if (!no_colour) wattroff(win, COLOR_PAIR(static_cast<unsigned int>(colour)) | colour_flags);
}

// Simple wrapper for unc::Glyph glyphs.
void print(unc::Glyph input, unc::Colour colour, unsigned int flags, int x, int y, std::shared_ptr<unc::Window> window)
{
	stack_trace();
	unc::print(static_cast<int>(input), colour, flags, x, y, window);
}

// This just makes it easier to do a newline print() on a unc::Window.
void print(std::shared_ptr<unc::Window> window, int newline_count)
{
	for (int i = 0; i < newline_count; i++)
		unc::print('\n', unc::Colour::NONE, 0, -1, -1, window);
}

// Renders a grid of the specified size.
void render_grid(int x, int y, int w, int h, unc::Colour colour, std::shared_ptr<unc::Window> window)
{
	stack_trace();
	for (int gx = 0; gx < w; gx++)
	{
		for (int gy = 0; gy < h; gy++)
		{
			int screen_x = x + (gx * 4), screen_y = y + (gy * 2);
			unc::Glyph glyph_l = unc::Glyph::PLUS, glyph_m = unc::Glyph::HLINE, glyph_r = unc::Glyph::PLUS;
			if (gy == 0)
			{
				glyph_l = unc::Glyph::TTEE;
				glyph_r = unc::Glyph::TTEE;
				if (gx == 0) glyph_l = unc::Glyph::ULCORNER;
				else if (gx == w - 1) glyph_r = unc::Glyph::URCORNER;
			}
			else if (gx == 0) glyph_l = unc::Glyph::LTEE;
			else if (gx == w - 1) glyph_r = unc::Glyph::RTEE;
			unc::print(glyph_l, colour, 0, screen_x, screen_y, window);
			for (int i = 1; i <= 3; i++)
				unc::print(glyph_m, colour, 0, screen_x + i, screen_y, window);
			unc::print(unc::Glyph::VLINE, colour, 0, screen_x, screen_y + 1, window);
			if (gx == w - 1)
			{
				unc::print(glyph_r, colour, 0, screen_x + 4, screen_y, window);
				unc::print(unc::Glyph::VLINE, colour, 0, screen_x + 4, screen_y + 1, window);
			}
			if (gy == h - 1)
			{
				unc::Glyph glyph_bl = unc::Glyph::BTEE;
				if (gx == 0) glyph_bl = unc::Glyph::LLCORNER;
				unc::print(glyph_bl, colour, 0, screen_x, screen_y + 2, window);
				for (int i = 1; i <= 3; i++)
					unc::print(unc::Glyph::HLINE, colour, 0, screen_x + i, screen_y + 2, window);
				if (gx == w - 1) unc::print(unc::Glyph::LRCORNER, colour, 0, screen_x + 4, screen_y + 2, window);
			}
		}
	}
}

// Access to the KEY_RESIZE definition in curses.h
int resize_key()
{
	return KEY_RESIZE;
}

// Turns the cursor on or off.
void set_cursor(bool enabled)
{
	stack_trace();
	if (enabled)
	{
		cursor_state = 2;
		curs_set(2);
		echo();
	}
	else
	{
		cursor_state = 0;
		curs_set(0);
		noecho();
	}
}

// Sets the console window title. Only works on PDCurses; does nothing on NCurses.
#ifdef PDCURSES
void set_window_title(std::string title)
{
	stack_trace();
	PDC_set_title(title.c_str());
}
#else
void set_window_title(std::string) { }
#endif

// Runs Curses cleanup code.
void shutdown()
{
	stack_trace();
	cursor_state = 1;
	curs_set(1);
	echo();
	endwin();
#ifdef USING_GURU_MEDITATION
	guru::close_syslog();
#endif
}

#ifndef USING_POTLUCK
// Below this point are replacement libraries from the Potluck library, used when USING_POTLUCK is not defined.

// String split/explode function.
std::vector<std::string> string_explode(std::string str, std::string separator)
{
	stack_trace();
	std::vector<std::string> results;

	std::string::size_type pos = str.find(separator, 0);
	const int pit = separator.length();

	while(pos != std::string::npos)
	{
		if (pos == 0) results.push_back("");
		else results.push_back(str.substr(0, pos));
        str.erase(0, pos + pit);
        pos = str.find(separator, 0);
    }
    results.push_back(str);

    return results;
}

// Splits a string into a vector of strings, to a given line length.
std::vector<std::string> vector_split(std::string source, unsigned int line_len)
{
	stack_trace();
	std::vector<std::string> result;
	if (source.size() <= line_len)
	{
		result.push_back(source);
		return result;
	}
	std::vector<std::string> words = unc::string_explode(source, " ");
	std::string current_line;
	while (words.size())
	{
		std::string word = words.at(0);
		words.erase(words.begin());
		if (word.size() > line_len)
		{
			// If the word itself is too long for the line, break it into two, then deal with them individually.
			// We'll then deal with that in the normal way afterwards. If the second half is still too long, it'll get broken up again when we get to it.
			// This could result in more iteration for edge cases, but overall keeps the code nice and simple.
			std::string first_half = word.substr(0, line_len);
			std::string second_half = word.substr(line_len + 1);
			words.insert(words.begin(), second_half);
			words.insert(words.begin(), first_half);
			continue;
		}
		if (current_line.size() + word.size() + 1 > line_len)
		{
			result.push_back(current_line);
			current_line = word;
			continue;
		}
		if (current_line.size()) current_line += " " + word;
		else current_line = word;
	}
	if (current_line.size()) result.push_back(current_line);
	return result;
}
#else
std::vector<std::string> string_explode(std::string str, std::string separator) { return potluck::string_explode(str, separator); }
std::vector<std::string> vector_split(std::string source, unsigned int line_len) { return potluck::vector_split(source, line_len); }
#endif

}	// namespace unc
