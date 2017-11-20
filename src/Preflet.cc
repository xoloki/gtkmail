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

#include "Preflet.hh"

#include <gtkmm/label.h>

namespace gtkmail {

    Gtk::HBox* Preflet::indent(Gtk::Widget* w, int x) {
        Gtk::Label* space = Gtk::manage(new Gtk::Label());
        Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, x));

        space->show();
        hbox->show();

        hbox->pack_start(*space, Gtk::PACK_SHRINK);
        hbox->pack_start(*w);

        return hbox;
    }

    Gtk::Label* Preflet::create_title(Glib::ustring s) {
        Gtk::Label* label = Gtk::manage(new Gtk::Label("<b>"+s+"</b>"));

        label->set_alignment(0, 0.5);
        label->property_use_markup() = true;

        label->show();

        return label;
    }
    
    void Preflet::append_title(Glib::ustring title) {
        this->pack_start(*create_title(title), Gtk::PACK_SHRINK);
    }

    void Preflet::append_indent(Gtk::Widget& w, int x) {
        this->pack_start(*indent(&w,x), Gtk::PACK_SHRINK);
    }
    
}

