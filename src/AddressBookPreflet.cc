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

#include "AddressBookPreflet.hh"

#include "Dialog.hh"

#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>

#include <jlib/util/util.hh>

#include "xpm/addressbook.xpm"

namespace gtkmail {

    AddressBookDialog::AddressBookDialog(Address addr)
        : m_address(addr)
    {
        Gtk::Table* general_table = Gtk::manage(new Gtk::Table(3, 2));
        Gtk::Label* label;

        m_key = Gtk::manage(new Gtk::Entry());
        m_name = Gtk::manage(new Gtk::Entry());
        m_addr = Gtk::manage(new Gtk::Entry());

        // general table
        label = Gtk::manage(new Gtk::Label("Key"));
        label->set_alignment(1, 0.5);
        general_table->attach(*label,    0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL);
        general_table->attach(*m_key ,   1, 2, 0, 1);

        label = Gtk::manage(new Gtk::Label("Name"));
        label->set_alignment(1, 0.5);
        general_table->attach(*label,    0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL);
        general_table->attach(*m_name,   1, 2, 1, 2);

        label = Gtk::manage(new Gtk::Label("Address"));
        label->set_alignment(1, 0.5);
        general_table->attach(*label,    0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL);
        general_table->attach(*m_addr,   1, 2, 2, 3);

        general_table->set_spacings(5);
        set_border_width(10);
        get_vbox()->set_spacing(5);
        get_action_area()->set_layout(Gtk::BUTTONBOX_DEFAULT_STYLE);

        get_vbox()->pack_start(*Preflet::create_title("Address Information"), Gtk::PACK_SHRINK);
        get_vbox()->pack_start(*Preflet::indent(general_table), Gtk::PACK_SHRINK);

        add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
        add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

        set_default_size(400, 300);

        load();
    }
    
    void AddressBookDialog::load() {
        m_key->set_text(m_address.get_key());
        m_name->set_text(m_address.get_name());
        m_addr->set_text(m_address.get_addr());
    }

    void AddressBookDialog::save() {
        m_address.set_key(m_key->get_text());
        m_address.set_name(m_name->get_text());
        m_address.set_addr(m_addr->get_text());
    }

    
    AddressBookPreflet::AddressBookPreflet()
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

        m_view->append_column("Key", m_cols.m_key);
        m_view->append_column("Name", m_cols.m_name);
        m_view->append_column("Address", m_cols.m_addr);
        
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

        m_new->signal_clicked().connect(sigc::mem_fun(*this, &AddressBookPreflet::on_new));
        m_edit->signal_clicked().connect(sigc::mem_fun(*this, &AddressBookPreflet::on_edit));
        m_delete->signal_clicked().connect(sigc::mem_fun(*this, &AddressBookPreflet::on_delete));

        show_all();
    }
    
    void AddressBookPreflet::load() {
        for(AddressBook::iterator i = AddressBook::global.begin(); i != AddressBook::global.end(); i++) {
            Gtk::TreeModel::iterator j = m_model->append();
            Gtk::TreeModel::Row row = *j;

            row[m_cols.m_key] = i->get_key();
            row[m_cols.m_name] = i->get_name();
            row[m_cols.m_addr] = i->get_addr();
        }
    }

    void AddressBookPreflet::save() {
        // info is saved in real time
    }
    
    Gtk::Widget* AddressBookPreflet::get_icon() {
        Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, 0));
        Gtk::Label* label = Gtk::manage(new Gtk::Label("Address"));
        Gtk::Image* image = Gtk::manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(addressbook_xpm)));

        vbox->pack_start(*image, Gtk::PACK_SHRINK);
        vbox->pack_start(*label, Gtk::PACK_SHRINK);

        return vbox;
    }

    void AddressBookPreflet::on_new() {
        Address addr;
        AddressBookDialog dlg(addr);
        dlg.set_title("New Address");
        dlg.show_all();
        if(dlg.run() == Gtk::RESPONSE_OK) {
            dlg.save();

            Gtk::TreeModel::iterator i = m_model->append();
            Gtk::TreeModel::Row row = *i;

            try {
                row[m_cols.m_key] = addr.get_key();
                row[m_cols.m_name] = addr.get_name();
                row[m_cols.m_addr] = addr.get_addr();

                AddressBook::global.push_back(addr);

            } catch(Glib::Exception& e) {
                display_exception(std::string("Unable to add new AddressBook: ") + e.what(), &dlg);
            } catch(std::exception& e) {
                display_exception(std::string("Unable to add new AddressBook: ") + e.what(), &dlg);
            } catch(...) {
                display_exception("Unable to add new AddressBook", &dlg);
            }
        }
    }

    void AddressBookPreflet::on_edit() {
        Gtk::TreeModel::iterator j = m_select->get_selected();
        if(!j) return;
        Gtk::TreeModel::Row row = *j;
        
        std::string key = static_cast<Glib::ustring>(row[m_cols.m_key]);
        AddressBook::iterator i = AddressBook::global.find(key);
        if(i == AddressBook::global.end()) return;

        AddressBookDialog dlg(*i);
        dlg.set_title("Edit Address");
        dlg.show_all();
        if(dlg.run() == Gtk::RESPONSE_OK) {
            dlg.save();

            try {
                row[m_cols.m_key] = i->get_key();
                row[m_cols.m_name] = i->get_name();
                row[m_cols.m_addr] = i->get_addr();

            } catch(Glib::Exception& e) {
                display_exception(std::string("Unable to add new AddressBook: ") + e.what(), &dlg);
            } catch(std::exception& e) {
                display_exception(std::string("Unable to add new AddressBook: ") + e.what(), &dlg);
            } catch(...) {
                display_exception("Unable to add new AddressBook", &dlg);
            }
        }
    }

    void AddressBookPreflet::on_delete() {
        Gtk::TreeModel::iterator j = m_select->get_selected();
        if(!j) return;
        Gtk::TreeModel::Row row = *j;
        
        std::string key = static_cast<Glib::ustring>(row[m_cols.m_key]);
        AddressBook::iterator i = AddressBook::global.find(key);
        if(i == AddressBook::global.end()) return;

        AddressBook::global.erase(i);
        m_model->erase(j);
    }

}

