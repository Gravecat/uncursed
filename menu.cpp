/* menu.cpp -- Menu class definition, for a fairly generic scrollable menu of items.
   RELEASE VERSION 1.4 -- 18th December 2019

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

#include "menu.h"

#ifdef USE_UNCURSED_MENU
#ifdef USING_GURU_MEDITATION
#include "guru/guru.h"
#endif


namespace unc
{

#define MENU_SIDEBOX_WIDTH	20


// Adds an item to this Menu.
void Menu::add_item(std::string txt, Colour col, std::string sidebox)
{
	stack_trace();
	items.push_back(txt);
	item_x.push_back(0);
	colour.push_back(col);
	item_sidebox.push_back(sidebox);
	if (sidebox.size())
	{
		const unsigned int height = unc::vector_split(sidebox, MENU_SIDEBOX_WIDTH).size();
		if (height > sidebox_height) sidebox_height = height;
	}
}

// Repositions the menu.
void Menu::reposition()
{
	stack_trace();
	if (!items.size()) return;
	const int midrow = 12, midcol = 40;
	unsigned int widest = 0;
	for (auto item : items)
	{
		const unsigned int len = item.size();
		if (len > widest) widest = len;
	}
	if (tag_bl.size() + tag_br.size() > widest) widest = tag_bl.size() + tag_br.size();
	if (title.size() > widest) widest = title.size();
	x_size = widest + 4;
	y_size = items.size() + 2;
	if (y_size > 24) y_size = 24;
	x_pos = midcol - (x_size / 2);
	y_pos = midrow - (y_size / 2);
	if (offset_text) x_pos += ((MENU_SIDEBOX_WIDTH + 2) / 2);

	// Assign a new Window to this Menu here. Since it's a shared_ptr, it doesn't matter if it was nullptr or another Window before -- the latter will just be deleted and replaced.
	window = std::make_shared<Window>(x_size, y_size, x_pos, y_pos);
	if (offset_text) window_offset = std::make_shared<Window>(MENU_SIDEBOX_WIDTH + 4, sidebox_height + 2, x_pos - MENU_SIDEBOX_WIDTH - 4, y_pos);
	const int window_midcol = unc::get_midcol(window);

	for (unsigned int i = 0; i < item_x.size(); i++)
		item_x.at(i) = window_midcol - (items.at(i).size() / 2);

	title_x = window_midcol - (title.size() / 2);
	bl_x = x_pos + 1;
	br_x = x_pos + x_size - tag_br.size() - 1;
}

// Renders the menu, returns the chosen menu item (or -1 if none chosen)
int Menu::render()
{
	stack_trace();
	if (!items.size()) return -1;

	if (colour.at(0) == Colour::BLACK)
	{
		for (unsigned int i = 0; i < items.size(); i++)
		{
			if (colour.at(i) != Colour::BLACK)
			{
				selected = i;
				break;
			}
		}
	}

	reposition();
	while(true)
	{
		unc::cls(window);
		unc::box(window);
		if (title.size())
		{
			unc::move_cursor(title_x - 1, 0, window);
			unc::print(Glyph::RTEE, Colour::NONE, 0, -1, -1, window);
			unc::print(title, Colour::CYAN, UNC_BOLD, -1, -1, window);
			unc::print(Glyph::LTEE, Colour::NONE, 0, -1, -1, window);
		}
		if (tag_bl.size()) unc::print(tag_bl, Colour::WHITE, UNC_BOLD, bl_x, window->get_height() - 1, window);
		if (tag_br.size()) unc::print(tag_br, Colour::WHITE, UNC_BOLD, br_x, window->get_height() - 1, window);
		const unsigned int start = offset;
		unsigned int end = items.size();
		if (end - offset > 22) end = 22 + offset;
		for (unsigned int i = start; i < end; i++)
			unc::print(items.at(i), colour.at(i), UNC_BOLD | (selected == i ? UNC_REVERSE : 0), item_x.at(i), 1 + i - offset, window);
		if (offset > 0) unc::print(Glyph::UARROW, Colour::GREEN, UNC_BOLD, window->get_width() - 1, 1, window);
		if (end < items.size()) unc::print(Glyph::DARROW, Colour::GREEN, UNC_BOLD, window->get_width() - 1, window->get_height() - 2, window);
		if (offset_text)
		{
			unc::cls(window_offset);
			unc::box(window_offset);
			if (item_sidebox.at(selected).size())
			{
				std::vector<std::string> lines = unc::vector_split(item_sidebox.at(selected), MENU_SIDEBOX_WIDTH);
				for (unsigned int i = 0; i < lines.size(); i++)
					unc::print(lines.at(i), Colour::NONE, 0, 2, 1 + i, window_offset);
			}
		}
		unc::flip();

		int key = unc::get_key();
		if (key == unc::resize_key())
		{
			reposition();
			continue;
		}
		else if (unc::is_up(key) && selected > 0)
		{
			const unsigned int old_selected = selected;
			selected--;
			while (selected > 0 && (!items.at(selected).size() || colour.at(selected) == Colour::BLACK)) selected--;
			if (colour.at(selected) == Colour::BLACK) selected = old_selected;
		}
		else if (unc::is_down(key) && selected < items.size() - 1)
		{
			const unsigned int old_selected = selected;
			selected++;
			while (selected < items.size() - 1 && (!items.at(selected).size() || colour.at(selected) == Colour::BLACK)) selected++;
			if (colour.at(selected) == Colour::BLACK) selected = old_selected;
		}
		else if (unc::is_left(key) && allow_left) return -2;
		else if (unc::is_right(key) && allow_right) return -3;
		else if (unc::is_select(key)) return selected;
		else if (unc::is_cancel(key)) return -1;

		if (selected > offset + 21) offset++;
		else if (selected < offset) offset--;
	}
	return 0;
}

// Sets a title for this menu.
void Menu::set_title(std::string new_title)
{
	title = new_title;
}

// Sets one or both of the bottom tags.
void Menu::set_tags(std::string bl, std::string br)
{
	if (bl.size()) tag_bl = bl;
	if (br.size()) tag_br = br;
}

// Allow left and/or right keys as input.
void Menu::allow_left_right(unsigned int flags)
{
	if ((flags & UNC_FLAG_LEFT) == UNC_FLAG_LEFT) allow_left = true;
	if ((flags & UNC_FLAG_RIGHT) == UNC_FLAG_RIGHT) allow_right = true;
}

// Sets the currently-selected item.
void Menu::set_selected(unsigned int pos)
{
	selected = pos;
	while (selected > offset + 21) offset++;
	while (selected < offset) offset--;
}

}	// namespace unc
#endif
