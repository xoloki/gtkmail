/* -*- mode: C++ c-basic-offset: 4  -*-
 * TextWin.hh - header file for class TextWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * Gtktextwin
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#ifndef GTKMAIL_TEXTWIN_HH
#define GTKMAIL_TEXTWIN_HH

#include <string>

#include <gtkmm/window.h>
#include <gtkmm/textview.h>

namespace gtkmail {

    class TextWin : public Gtk::Window {
    public:
        TextWin(std::string data);
        TextWin(std::string data,unsigned int x, unsigned int y);
        virtual ~TextWin();
        
        void set_text(std::string data);
        std::string get_text();

        void set_size(unsigned int x, unsigned int y);
    protected:
        void init(std::string data, unsigned int x, unsigned int y);

        Gtk::TextView m_text;
    };

}

#endif //GTKMAIL_TEXTWIN_HH
