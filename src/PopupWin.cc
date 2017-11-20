/* -*- mode: C++ c-basic-offset: 4  -*-
 * 
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
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

#include "PopupWin.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk--/main.h>
#include <gnome--/about.h>
#include <gnome--/app.h>

namespace gtkmail {
    
    PopupWin::PopupWin() 
        : Gtk::Dialog(),
               vbox(false, 5),
               okButton("Ok"),
               caButton("Cancel")
    {
        std::vector<std::string> dummy;
        init(dummy);
        
        okButton.clicked.connect(this->destroy.slot());
        caButton.clicked.connect(this->destroy.slot());
        
        show_all();
        
    }
    
    PopupWin::PopupWin(std::vector<std::string> s, bool hook_ok, bool hook_ca)
        : Gtk::Dialog(),
               vbox(false, 5),
               okButton("Ok"),
               caButton("Cancel")
    {
        init(s);
        
        if(hook_ok) okButton.clicked.connect(this->destroy.slot());
        if(hook_ca) caButton.clicked.connect(this->destroy.slot());
        
        show_all();
    }
    
    
    PopupWin::PopupWin(std::vector<std::string> s, int t)
        : Gtk::Dialog(),
               vbox(false, 5),
               okButton("Ok"),
               caButton("Cancel")
    {
        init(s);
        
        timeCon = Gtk::Main::timeout.connect(slot(this, &PopupWin::deleteSelf2), 1000*t);
        okButton.clicked.connect(slot(this, &PopupWin::deleteSelf));
        caButton.clicked.connect(slot(this, &PopupWin::deleteSelf));
        
        show_all();
    }
    
    PopupWin::~PopupWin() {
    }
    
    void PopupWin::init(std::vector<std::string> s) {
        std::vector<std::string>::iterator i;
        for(i=s.begin(); i!=s.end(); i++) {
            Gtk::HBox* box = manage(new Gtk::HBox(false, 10));
            Gtk::Label* label = manage(new Gtk::Label(*i));
            box->pack_start(*label, false, true, 10);
            boxVec.push_back(box);
        }
        
        std::vector<Gtk::HBox*>::iterator j;
        for(j=boxVec.begin(); j!=boxVec.end(); j++) {
            vbox.pack_start(*(*j), false, true, 0);
        }
        
        get_vbox()->pack_start(vbox,true,true,10);
        set_position(GTK_WIN_POS_CENTER);
        
        get_action_area()->pack_start(okButton,TRUE, TRUE, 25);
        get_action_area()->pack_start(caButton,TRUE, TRUE, 25);
    }
    
    void PopupWin::deleteSelf() {
        hide();
        timeCon.disconnect();
        manage(this)->destroy();
    }
    
    int PopupWin::deleteSelf2() {
        hide();
        manage(this)->destroy();
        return 0;
    }

    EntryWin::EntryWin(std::string label) {
        Gtk::Button* okbutton;
        Gtk::Button* cabutton;
        Gtk::HBox* hbox;
        Gtk::Label* lab = manage(new Gtk::Label(label));
        m_destroyed = false;

        hbox = manage(new Gtk::HBox(true,0));
        Gtk::HBox* hbox2 = manage(new Gtk::HBox(true,0));
        
        okbutton = manage(new Gtk::Button("Ok"));
        cabutton = manage(new Gtk::Button("Cancel"));
        
        //this->set_title(PACKAGE);
        
        m_entry = manage(new Gtk::Entry());
        m_entry->set_visibility(false);

        hbox->pack_start(*m_entry, false, true, 20);
        hbox2->pack_start(*lab, false, true, 20);
        
        this->get_vbox()->pack_start(*hbox2, true, true, 10);
        this->get_vbox()->pack_start(*hbox, true, true, 10);
        this->get_action_area()->pack_start(*okbutton,true, true, 25);
        this->get_action_area()->pack_start(*cabutton,true, true, 25);
        this->set_position(GTK_WIN_POS_CENTER);
        
        m_entry->activate.connect(Gtk::Main::quit.slot());

        okbutton->clicked.connect(Gtk::Main::quit.slot());
        cabutton->clicked.connect(this->destroy.slot());
        cabutton->clicked.connect(Gtk::Main::quit.slot());
        this->destroy.connect(SigC::slot(this,&gtkmail::EntryWin::set_destroyed));
        //this->destroy.connect(Gtk::Main::quit.slot());

        this->show_all();
        this->get_entry()->grab_focus();

    }
        
    Gtk::Entry* EntryWin::get_entry() { 
        return m_entry; 
    }

    bool EntryWin::is_destroyed() { return m_destroyed; }
    void EntryWin::set_destroyed() { m_destroyed = true; }


    void display_exception(std::string e, Gtk::Widget* widget) {
        Gnome::App* app = dynamic_cast<Gnome::App*>(widget->get_toplevel());
        if(app != 0) {
            Gnome::Dialog* d = app->error(e);
            d->show_all();
        }
    }
    
    void display_exception_block(std::string e, Gtk::Widget* widget) {
        Gnome::App* app = dynamic_cast<Gnome::App*>(widget->get_toplevel());
        if(app != 0) {
            Gnome::Dialog* d = app->error(e);
            d->run();
        }
    }

    void display_about() {
        std::vector<std::string> vec; vec.push_back("Joe Yandle <jwy@divisionbyzero.com>");
        Gnome::About* about = new Gnome::About(PACKAGE_NAME,PACKAGE_VERSION,"Copyright (c) 2000 Joe Yandle", vec);
        about->show_all();
    }
    
}
