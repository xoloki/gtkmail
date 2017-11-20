/* -*- mode: C++ c-basic-offset: 4  -*-
 * Gtktextwin.h - header file for class Gtktextwin
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

#include "TextWin.hh"
#include "Config.hh"

#include <gtkmm/scrolledwindow.h>

namespace gtkmail {
    
    TextWin::TextWin(std::string data) {
        init(data,600,600);
    }
    
    TextWin::TextWin(std::string data, unsigned int x, unsigned int y) {
        init(data,x,y);
    }

    TextWin::~TextWin() {
        
    }

    void TextWin::set_text(std::string data) { 
        Glib::RefPtr<Gtk::TextBuffer> buf = m_text.get_buffer();
        Glib::ustring clean = data;
        Glib::ustring::iterator i;
        while(!clean.validate(i)) clean.replace(i++, i, "*");
        buf->set_text(clean);
    }

    std::string TextWin::get_text() { 
        Glib::RefPtr<Gtk::TextBuffer> buf = m_text.get_buffer();
        return buf->get_text();
    }

    void TextWin::set_size(unsigned int x, unsigned int y) {
        set_default_size(x,y);
    }
    
    void TextWin::init(std::string data, unsigned int x, unsigned int y) {
        /*
        Gtk::Text::Context ctx = m_text->get_context();
        ctx.set_font(Config::global.get_message_font());
        m_text->set_context(ctx);
        */

        set_text(data);
        set_size(x,y);

        Gtk::ScrolledWindow* win = manage(new Gtk::ScrolledWindow());
        win->add(m_text);
        
        add(*win);
        show_all();
    }
}
