/* -*- mode: C++ c-basic-offset: 4  -*-
 * MessageWin.h - header file for class MessageWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * MessageWin
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

#ifndef GTKMAIL_MESSAGEWIN_HH
#define GTKMAIL_MESSAGEWIN_HH

#include <gtkmm/window.h>
#include <gtkmm/entry.h>
#include <gtkmm/notebook.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/table.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/combo.h>
#include <gtkmm/textview.h>
#include <gtkmm/treeview.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/fontselection.h>
#include <gtkmm/uimanager.h>

#include <jlib/net/Email.hh>
#include <jlib/net/ASMailBox.hh>

namespace gtkmail {

    class MailBox;
    
    class MessageWin : public Gtk::Window {
    public:
        MessageWin(const jlib::net::Email& e, MailBox* box, jlib::net::folder_info_type info);
        ~MessageWin();

        void set_to(std::string s);
        void set_cc(std::string s);
        void set_subject(std::string s);

    protected:
        static gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer data);

        void build_tree();
        void build_tree(Gtk::TreeModel::Row& row, jlib::net::Email& e);
        //void delete_tree(Gtk::CTree_Helpers::RowList& rows);
        
        void message_call(std::string s);
        void help_call(std::string s);

        void on_open_url();

        void on_tab_next();
        void on_tab_prev();
        
        void init_menus();

        void append_text(std::string s);
        void set_text(std::string s);
        std::string get_text();

        class AttachModelCols : public Gtk::TreeModel::ColumnRecord {
        public:
            AttachModelCols() { add(m_icon); add(m_content); add(m_encoding); add(m_file); add(m_data); }

            Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_icon;
            Gtk::TreeModelColumn<Glib::ustring> m_content;
            Gtk::TreeModelColumn<Glib::ustring> m_encoding;
            Gtk::TreeModelColumn<Glib::ustring> m_file;
            Gtk::TreeModelColumn<jlib::net::Email> m_data;
        };

        AttachModelCols m_attach_cols;
        Glib::RefPtr<Gtk::TreeStore> m_attach_model;
        Gtk::TreeView* m_attach_view;

        Gtk::MenuBar* m_menus;
        Gtk::Toolbar* m_tools;

        Gtk::Menu m_file_menu;
        Gtk::Menu m_message_menu;
        Gtk::Menu m_view_menu;
        Gtk::Menu m_attach_menu;
        Gtk::Menu m_crypt_menu;
        Gtk::Menu m_help_menu;

        Gtk::TextView* m_message;
        Gtk::Combo* m_from;
        Gtk::Entry *m_date, *m_subj, *m_to, *m_cc;
        Gtk::Notebook* m_notebook;
        Gtk::Table* m_headers;
        Gtk::VBox* m_vbox;
        Gtk::Statusbar *m_status;

        Glib::RefPtr<Gtk::TextBuffer::Tag> m_text_tag;

        jlib::net::Email m_email;
        std::string m_user_addr;
        gtkmail::MailBox* m_box;

        jlib::net::folder_info_type m_info;

        Glib::RefPtr<Gtk::UIManager> m_ui;
        Glib::RefPtr<Gtk::ActionGroup> m_actions;
    };
    
}

#endif
