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

#include "Prefs.hh"
#include "Config.hh"

#include "UIPreflet.hh"
#include "MailBoxPreflet.hh"
#include "AddressBookPreflet.hh"

#include <gtkmm/stock.h>
#include <gtkmm/separator.h>
#include <gtkmm/frame.h>
#include <gtkmm/messagedialog.h>

#include <iostream>

namespace gtkmail {
    Prefs::Prefs() 
        : Gtk::Window(),
          m_current(0)
        
    {
        Gtk::ButtonBox* bbox = Gtk::manage(new Gtk::HButtonBox());
        Gtk::Frame* radio_frame = Gtk::manage(new Gtk::Frame());

        m_apply = Gtk::manage(new Gtk::Button(Gtk::Stock::APPLY));
        m_cancel = Gtk::manage(new Gtk::Button(Gtk::Stock::CANCEL));

        m_hbox = Gtk::manage(new Gtk::HBox(false, 10));
        m_vbox = Gtk::manage(new Gtk::VBox(false, 10));
        m_radio_box = Gtk::manage(new Gtk::VBox(false, 0));

        bbox->pack_start(*m_apply);
        bbox->pack_start(*m_cancel);

        radio_frame->set_shadow_type(Gtk::SHADOW_IN);
        radio_frame->add(*m_radio_box);

        m_hbox->pack_start(*radio_frame, Gtk::PACK_SHRINK);

        m_vbox->pack_start(*m_hbox);
        m_vbox->pack_end(*bbox, Gtk::PACK_SHRINK);
        m_vbox->pack_end(*Gtk::manage(new Gtk::HSeparator()), Gtk::PACK_SHRINK);

        this->add(*m_vbox);

        set_default_size(600, 400);
        set_position(Gtk::WIN_POS_CENTER);
        set_border_width(10);
        set_title("Preferences");

        m_apply->signal_clicked().connect(sigc::mem_fun(*this, &Prefs::on_apply));
        m_cancel->signal_clicked().connect(sigc::mem_fun(*this, &Prefs::on_cancel));

        append(new UIPreflet());
        append(new MailBoxPreflet());
        append(new AddressBookPreflet());
    }
    
    Prefs::~Prefs() {
        std::cout << "Prefs::~Prefs" << std::endl;
        std::list<Preflet*>::iterator i;
        for(i = m_preflets.begin(); i != m_preflets.end(); i++) {
            if(*i == m_current) {
                m_hbox->remove(**i);
            }
            delete (*i);
        }
    }

    bool Prefs::on_frame(GdkEvent* e) {
        std::cout << "Prefs::on_frame(" << e->type << ")" << std::endl;
        return true;
    }

    void Prefs::on_apply() {
        std::list<Preflet*>::iterator i;
        for(i = m_preflets.begin(); i != m_preflets.end(); i++) {
            (*i)->save();
        }

        signal_apply.emit();

        this->destroy_();
    }

    void Prefs::on_cancel() {
        signal_cancel.emit();

        this->destroy_();
    }
    
    void Prefs::append(Preflet* p) {
        Gtk::RadioButton* r = Gtk::manage(new Gtk::RadioButton());

        r->set_relief(Gtk::RELIEF_NONE);
        r->set_mode(false);
        r->add(*p->get_icon());
        r->set_group(m_group);
        r->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &Prefs::on_preflet_clicked), p));

        m_radio_box->pack_start(*r, Gtk::PACK_SHRINK);

        m_preflets.push_back(p);

        if(!m_current) {
            on_preflet_clicked(p);
        }

        p->load();

        MailBoxPreflet* mbp = dynamic_cast<MailBoxPreflet*>(p);
        if(mbp) {
            mbp->signal_new.connect(signal_new.slot());
            mbp->signal_edited.connect(signal_edited.slot());
            mbp->signal_deleted.connect(signal_deleted.slot());
        }
    }
    

    void Prefs::on_preflet_clicked(Preflet* p) {
        if(p != m_current) {
            if(m_current) {
                m_hbox->remove(*m_current);
                m_current = 0;
            }

            m_hbox->pack_start(*p);
            m_current = p;
        }
    }
    
}
