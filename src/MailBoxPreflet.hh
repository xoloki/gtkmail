/* -*- mode: C++ c-basic-offset: 4  -*-
 * MailBoxPreflet.hh - header file for class Preflet
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

#ifndef GTKMAIL_MAILBOX_PREFLET_HH
#define GTKMAIL_MAILBOX_PREFLET_HH

#include "Preflet.hh"
#include "Config.hh"

#include <gtkmm/dialog.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/optionmenu.h>
#include <gtkmm/entry.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/table.h>

namespace gtkmail {

    class MailBoxDialog : public Gtk::Dialog {
    public:
        MailBoxDialog(Config::MailBox box);

        void load();
        void save();

    protected:
        std::string get_proto();
        void set_proto(std::string proto);

        void on_simap();
        void on_imap();
        void on_mbox();

        void on_smtp_auth();
        void on_smtp_auth_same();

        Config::MailBox m_box;

        Gtk::Entry* m_name;
        Gtk::OptionMenu* m_proto;
        Gtk::Entry* m_server;
        Gtk::SpinButton* m_port;
        Gtk::Entry* m_user;
        Gtk::Entry* m_pass;
        Gtk::Entry* m_addr;
        Gtk::Entry* m_path;
        Gtk::Entry* m_smtp;
        Gtk::SpinButton* m_smtp_port;
        Gtk::CheckButton* m_smtp_tls;
        Gtk::CheckButton* m_smtp_auth;
        Gtk::CheckButton* m_smtp_auth_same;
        Gtk::Entry* m_smtp_user;
        Gtk::Entry* m_smtp_pass;
        Gtk::Entry* m_proxy;
        Gtk::SpinButton* m_time;

        Gtk::Table* m_account_table;
        Gtk::Widget* m_account_title;

        Gtk::Label* m_server_label;
        Gtk::Label* m_port_label;
        Gtk::Label* m_smtp_user_label;
        Gtk::Label* m_smtp_pass_label;
        Gtk::Label* m_proxy_label;

        Gtk::CheckButton* m_idle;

        std::string m_proto_buf;
    };
    
    class MailBoxPreflet : public Preflet {
    public:
        MailBoxPreflet();

        virtual void load();
        virtual void save();
        
        virtual Gtk::Widget* get_icon();

        SigC::Signal1<void,Config::MailBox> signal_new;
        SigC::Signal1<void,Config::MailBox> signal_edited;
        SigC::Signal1<void,Config::MailBox> signal_deleted;

    protected:
        void on_new();
        void on_edit();
        void on_delete();

        class ModelCols : public Gtk::TreeModel::ColumnRecord {
        public:
            ModelCols() { add(m_name); add(m_original_name); add(m_url); add(m_box); add(m_new); add(m_edited); }

            Gtk::TreeModelColumn<Glib::ustring> m_name;
            Gtk::TreeModelColumn<std::string> m_original_name;
            Gtk::TreeModelColumn<Glib::ustring> m_url;
            Gtk::TreeModelColumn<Config::MailBox> m_box;
            Gtk::TreeModelColumn<bool> m_new;
            Gtk::TreeModelColumn<bool> m_edited;
        };

        ModelCols m_cols;
        Glib::RefPtr<Gtk::TreeStore> m_model;
        Gtk::TreeView* m_view;
        Glib::RefPtr<Gtk::TreeSelection> m_select;
        std::list<Config::MailBox> m_deleted;

        Gtk::Button* m_new;
        Gtk::Button* m_edit;
        Gtk::Button* m_delete;
    };
    
}

#endif //GTKMAIL_MAILBOX_PREFLET_HH
