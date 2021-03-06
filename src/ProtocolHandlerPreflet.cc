/* -*- mode: C++ c-basic-offset: 4  -*-
 * AddressBookPreflet.hh - header file for class Preflet
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
 * Foundation, Inc., 59 Temple Place - Smailboxte 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "ProtocolHandlerPreflet.hh"

#include "Dialog.hh"

#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>

#include <jlib/util/util.hh>

#include "xpm/addressbook.xpm"

namespace gtkmail {

ProtocolHandlerDialog::ProtocolHandlerDialog(ProtocolHandler protocol_handler)
    : m_protocol_handler(protocol_handler)
{
    Gtk::Table* general_table = Gtk::manage(new Gtk::Table(2, 2));
    Gtk::Label* label;
    
    m_protocol = Gtk::manage(new Gtk::Entry());
    m_handler = Gtk::manage(new Gtk::Entry());

    // general table
    label = Gtk::manage(new Gtk::Label("Protocol"));
    label->set_alignment(1, 0.5);
    general_table->attach(*label,    0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL);
    general_table->attach(*m_protocol ,   1, 2, 0, 1);

    label = Gtk::manage(new Gtk::Label("Handler"));
    label->set_alignment(1, 0.5);
    general_table->attach(*label,    0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL);
    general_table->attach(*m_handler,   1, 2, 1, 2);

    general_table->set_spacings(5);
    set_border_width(10);
    get_vbox()->set_spacing(5);
    get_action_area()->set_layout(Gtk::BUTTONBOX_DEFAULT_STYLE);

    get_vbox()->pack_start(*Preflet::create_title("Protocol Handler"), Gtk::PACK_SHRINK);
    get_vbox()->pack_start(*Preflet::indent(general_table), Gtk::PACK_SHRINK);

    add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
    add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

    set_default_size(400, 300);

    load();
}
    
void ProtocolHandlerDialog::load() {
    m_protocol->set_text(m_protocol_handler.get_protocol());
    m_handler->set_text(m_protocol_handler.get_handler());
}

void ProtocolHandlerDialog::save() {
    m_protocol_handler.set_protocol(m_protocol->get_text());
    m_protocol_handler.set_handler(m_handler->get_text());
}
    
ProtocolHandlerPreflet::ProtocolHandlerPreflet()
{
    Gtk::ButtonBox* bbox = Gtk::manage(new Gtk::HButtonBox());

    bbox->set_layout(Gtk::BUTTONBOX_END);

    m_new = Gtk::manage(new Gtk::Button("New"));
    m_edit = Gtk::manage(new Gtk::Button("Edit"));
    m_delete = Gtk::manage(new Gtk::Button("Delete"));

    Gtk::ScrolledWindow* win = manage(new Gtk::ScrolledWindow());
    win->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);

    m_view = Gtk::manage(new Gtk::TreeView());
    m_model = Gtk::TreeStore::create(m_cols);
    m_view->set_model(m_model);

    m_view->append_column("Protocol", m_cols.m_protocol);
    m_view->append_column("Handler", m_cols.m_handler);
        
    m_select = m_view->get_selection();
    m_select->set_mode(Gtk::SELECTION_BROWSE);

    m_view->set_rules_hint();

    Glib::ListHandle<Gtk::TreeViewColumn*> list = m_view->get_columns();
    Glib::ListHandle<Gtk::TreeViewColumn*>::iterator i = list.begin();
    for(; i != list.end(); i++) {
	(*i)->set_alignment(0.5);
    }

    win->add(*m_view);

    bbox->pack_start(*m_new);
    bbox->pack_start(*m_edit);
    bbox->pack_start(*m_delete);

    pack_start(*win);
    pack_end(*bbox, Gtk::PACK_SHRINK);

    m_new->signal_clicked().connect(sigc::mem_fun(*this, &ProtocolHandlerPreflet::on_new));
    m_edit->signal_clicked().connect(sigc::mem_fun(*this, &ProtocolHandlerPreflet::on_edit));
    m_delete->signal_clicked().connect(sigc::mem_fun(*this, &ProtocolHandlerPreflet::on_delete));

    show_all();
}
    
void ProtocolHandlerPreflet::load() {
    for(ProtocolHandlerMap::iterator i = ProtocolHandlerMap::global.begin(); i != ProtocolHandlerMap::global.end(); i++) {
        Gtk::TreeModel::iterator j = m_model->append();
        Gtk::TreeModel::Row row = *j;
        
        row[m_cols.m_protocol] = i->second.get_protocol();
        row[m_cols.m_handler] = i->second.get_handler();
    }
}

void ProtocolHandlerPreflet::save() {
    // info is saved in real time
}
    
Gtk::Widget* ProtocolHandlerPreflet::get_icon() {
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, 0));
    Gtk::Label* label = Gtk::manage(new Gtk::Label("Protocol"));
    Gtk::Image* image = Gtk::manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(addressbook_xpm)));
    
    vbox->pack_start(*image, Gtk::PACK_SHRINK);
    vbox->pack_start(*label, Gtk::PACK_SHRINK);
    
    return vbox;
}
    
void ProtocolHandlerPreflet::on_new() {
    ProtocolHandler protocol_handler;
    ProtocolHandlerDialog dlg(protocol_handler);
    dlg.set_title("New Protocol Handler");
    dlg.show_all();
    if(dlg.run() == Gtk::RESPONSE_OK) {
        dlg.save();
        
        Gtk::TreeModel::iterator i = m_model->append();
        Gtk::TreeModel::Row row = *i;
        
        try {
            row[m_cols.m_protocol] = protocol_handler.get_protocol();
            row[m_cols.m_handler] = protocol_handler.get_handler();
            
            ProtocolHandlerMap::global.insert(protocol_handler);

        } catch(Glib::Exception& e) {
            display_exception(std::string("Unable to add new ProtocolHandler: ") + e.what(), &dlg);
        } catch(std::exception& e) {
            display_exception(std::string("Unable to add new ProtocolHandler: ") + e.what(), &dlg);
        } catch(...) {
            display_exception("Unable to add new ProtocolHandler", &dlg);
        }
    }
}

void ProtocolHandlerPreflet::on_edit() {
    Gtk::TreeModel::iterator j = m_select->get_selected();
    if(!j) return;
    Gtk::TreeModel::Row row = *j;
    
    std::string protocol = static_cast<Glib::ustring>(row[m_cols.m_protocol]);
    ProtocolHandlerMap::iterator i = ProtocolHandlerMap::global.find(protocol);
    if(i == ProtocolHandlerMap::global.end()) return;
    
    ProtocolHandlerDialog dlg(i->second);
    dlg.set_title("Edit Handleress");
    dlg.show_all();
    if(dlg.run() == Gtk::RESPONSE_OK) {
        dlg.save();
        
        try {
            row[m_cols.m_protocol] = i->second.get_protocol();
            row[m_cols.m_handler] = i->second.get_handler();
            
        } catch(Glib::Exception& e) {
            display_exception(std::string("Unable to add new ProtocolHandler: ") + e.what(), &dlg);
        } catch(std::exception& e) {
            display_exception(std::string("Unable to add new ProtocolHandler: ") + e.what(), &dlg);
        } catch(...) {
            display_exception("Unable to add new ProtocolHandler", &dlg);
        }
    }
}

void ProtocolHandlerPreflet::on_delete() {
    Gtk::TreeModel::iterator j = m_select->get_selected();
    if(!j) return;
    Gtk::TreeModel::Row row = *j;
    
    std::string protocol = static_cast<Glib::ustring>(row[m_cols.m_protocol]);
    ProtocolHandlerMap::iterator i = ProtocolHandlerMap::global.find(protocol);
    if(i == ProtocolHandlerMap::global.end()) return;
    
    ProtocolHandlerMap::global.erase(i);
    m_model->erase(j);
}

}

