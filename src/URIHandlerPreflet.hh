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
 * Foundation, Inc., 59 Temple Place - Saddressbookte 330, Boston, MA  02111-1307, USA.
 * 
 */

#ifndef GTKMAIL_URI_HANDLER_PREFLET_HH
#define GTKMAIL_URI_HANDLER_PREFLET_HH

#include "Preflet.hh"

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

namespace gtkmail {

class URIHandlerDialog : public Gtk::Dialog {
public:
    URIHandlerDialog(std::string uri, std::string handler);
    
    void load();
    void save();
    
protected:
    Gtk::Entry* m_uri;
    Gtk::Entry* m_handler;
};
    
class URIHandlerPreflet : public Preflet {
public:
    URIHandlerPreflet();
    
    virtual void load();
    virtual void save();
    
    virtual Gtk::Widget* get_icon();
    
protected:
    void on_new();
    void on_edit();
    void on_delete();
    
    class ModelCols : public Gtk::TreeModel::ColumnRecord {
    public:
	ModelCols() { add(m_uri); add(m_handler); }
	
	Gtk::TreeModelColumn<Glib::ustring> m_uri;
	Gtk::TreeModelColumn<Glib::ustring> m_handler;
    };
    
    ModelCols m_cols;
    Glib::RefPtr<Gtk::TreeStore> m_model;
    Gtk::TreeView* m_view;
    Glib::RefPtr<Gtk::TreeSelection> m_select;
    
    Gtk::Button* m_new;
    Gtk::Button* m_edit;
        Gtk::Button* m_delete;
};
    
}

#endif //GTKMAIL_URI_HANDLER_PREFLET_HH
