/* menu.h -- Menu class definition, for a fairly generic scrollable menu of items.
   RELEASE VERSION 1.22 -- 15th December 2019

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

#include "uncursed.h"


#ifdef USE_UNCURSED_MENU
#include <memory>
#include <string>
#include <vector>

namespace unc
{

class Window;						// defined in uncursed.h
enum class Colour : unsigned int;	// defined in uncursed.h

#define UNC_FLAG_LEFT	1
#define UNC_FLAG_RIGHT	2


class Menu
{
public:
			Menu() : allow_left(false), allow_right(false), centered_text(true), offset(0), offset_text(false), redraw_on_exit(true), selected(0), sidebox_height(0), window(nullptr), window_offset(nullptr), x_pos(0), y_pos(0),
				x_size(0), y_size(0), title_x(0), bl_x(0), br_x(0) { }
	void	add_item(std::string txt, Colour col = static_cast<Colour>(0), std::string sidebox = "");	// Adds an item to this Menu.
	int		render();													// Renders the menu, returns the chosen menu item (or -1 if none chosen)
	void	set_title(std::string new_title);							// Sets a title for this menu.
	void	set_tags(std::string bl = "", std::string br = "");			// Sets one or both of the bottom tags.
	void	allow_left_right(unsigned int flags = 0);					// Allow left and/or right keys as input.
	void	set_selected(unsigned int pos);								// Sets the currently-selected item.
	void	no_redraw_on_exit() { redraw_on_exit = false; }				// Disables redraw on exit.
	void	set_centered_text(bool choice) { centered_text = choice; }	// Do we want the text center-aligned?
	void	set_sidebox(bool choice) { offset_text = choice; }			// Do we want an offset sidebox?

private:
	bool						allow_left, allow_right;	// Whether to allow left-right keys as input.
	bool						centered_text;				// Do we want the text center-aligned?
	std::vector<Colour>			colour;						// The colours of menu items.
	std::vector<std::string>	items;						// The menu item text.
	std::vector<std::string>	item_sidebox;				// Optionally, sidebox text for the selected menu item.
	std::vector<int>			item_x;						// The menu item positions.
	unsigned int				offset;						// The menu scroll.
	bool						offset_text;				// Do we want an offset sidebox?
	bool						redraw_on_exit;				// Redraws the background when exiting the menu.
	unsigned int				selected;					// The selected menu item.
	unsigned short				sidebox_height;				// The height of the optional sidebox.
	std::string					tag_bl, tag_br;				// Tag text at the bottom left and bottom right.
	std::string					title;						// The menu's title, if any.
	std::shared_ptr<Window>		window, window_offset;		// The Window classes assigned to this Menu.
	int							x_pos, y_pos, x_size, y_size, title_x, bl_x, br_x;	// Screen coordinates.

	void	reposition();	// Repositions the menu.
};

}	// namespace unc
#endif
