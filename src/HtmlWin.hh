/* -*- mode: C++ c-basic-offset: 4  -*-
 * HtmlWin.hh - header file for class HtmlWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * Gtkhtmlwin
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

#ifndef GTKMAIL_HTMLWIN_HH
#define GTKMAIL_HTMLWIN_HH

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/window.h>

#include <string>

namespace gtkmail {

    class HtmlWin : public Gtk::Window {
    public:
        HtmlWin(std::string data, std::string from);
        virtual ~HtmlWin();
        
        void set_source(std::string data, std::string from);
        std::string get_source();
    protected:
        Gtk::Widget* m_rep;
        Gtk::ScrolledWindow* m_win;

        std::string m_source;
    };

}

#endif //GTKMAIL_HTMLWIN_HH
