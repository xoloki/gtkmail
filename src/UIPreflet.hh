/* -*- mode: C++ c-basic-offset: 4  -*-
 * UIPreflet.hh - header file for class Preflet
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class Preflet is a widget which allows 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU UI Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU UI Public License for more details.
 * 
 * You should have received a copy of the GNU UI Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#ifndef GTKMAIL_UI_PREFLET_HH
#define GTKMAIL_UI_PREFLET_HH

#include "Preflet.hh"

#include <gtkmm/optionmenu.h>
#include <gtkmm/entry.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/toolbar.h>

namespace gtkmail {
    
    class UIPreflet : public Preflet {
    public:
        UIPreflet();

        virtual void load();
        virtual void save();
        
        virtual Gtk::Widget* get_icon();

    protected:
        void on_font_clicked();
        Gtk::ToolbarStyle get_toolbar_style();
        void set_toolbar_style(Gtk::ToolbarStyle style);
        void on_choose();

        Gtk::OptionMenu* m_toolbar_style;
        Gtk::Entry* m_font;
        Gtk::Entry* m_charset;
        Gtk::Entry* m_user_style;
        Gtk::CheckButton* m_auto_load;
    };
    
}

#endif //GTKMAIL_UI_PREFLET_HH
