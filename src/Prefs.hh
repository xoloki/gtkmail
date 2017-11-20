/* -*- mode: C++ c-basic-offset: 4  -*-
 * Prefs.hh - header file for class Prefs
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class Prefs is a widget which allows 
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

#ifndef GTKMAIL_PREFS_HH
#define GTKMAIL_PREFS_HH

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/window.h>

#include "Preflet.hh"
#include "Config.hh"

namespace gtkmail {
    
    class Prefs : public Gtk::Window {
    public:
        Prefs();
        virtual ~Prefs();

        SigC::Signal0<void> signal_apply;
        SigC::Signal0<void> signal_cancel;

        SigC::Signal1<void,Config::MailBox> signal_new;
        SigC::Signal1<void,Config::MailBox> signal_edited;
        SigC::Signal1<void,Config::MailBox> signal_deleted;

    protected:
        void on_apply();
        void on_cancel();
        bool on_frame(GdkEvent* e);

        void on_preflet_clicked(Preflet* p);

        void append(Preflet* p);

        Gtk::Button* m_apply;
        Gtk::Button* m_cancel;

        Gtk::HBox* m_hbox;
        Gtk::VBox* m_vbox;
        Gtk::VBox* m_radio_box;

        Gtk::RadioButton::Group m_group;

        Preflet* m_current;
        std::list<Preflet*> m_preflets;
    };
    
}

#endif //GTKMAIL_PREFS_HH
