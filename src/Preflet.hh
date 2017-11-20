/* -*- mode: C++ c-basic-offset: 4  -*-
 * Preflet.hh - header file for class Preflet
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class Preflet is a widget which allows 
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

#ifndef GTKMAIL_PREFLET_HH
#define GTKMAIL_PREFLET_HH

#include <gtkmm/box.h>

namespace gtkmail {
    
    class Preflet : public Gtk::VBox {
    public:
        virtual void load() = 0;
        virtual void save() = 0;

        virtual Gtk::Widget* get_icon() = 0;

        static Gtk::HBox* indent(Gtk::Widget* w, int x = 16);
        static Gtk::Label* create_title(Glib::ustring s);

    protected:
        void append_title(Glib::ustring title);
        void append_indent(Gtk::Widget& w, int x);
    };
    
}

#endif //GTKMAIL_PREFLET_HH
