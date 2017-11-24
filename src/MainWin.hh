/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkMainWin.h - header file for class GtkMainWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * GtkMainWin provides an implementation of the abstract class View, using
 * Gtk::Window as one of its base classes.  It uses the Gtk-- event handling
 * model (the connect_to_* functions)..
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

#ifndef GTKMAIL_MAINWIN_HH
#define GTKMAIL_MAINWIN_HH

#include <map>
#include <string>
#include <list>

#include <gtkmm/window.h>
#include <gtkmm/notebook.h>
#include <gtkmm/menubar.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/uimanager.h>

#include <jlib/net/MailFetch.hh>
#include <jlib/sys/pipe.hh>

#include <glibmm/thread.h>

#include "Config.hh"

namespace gtkmail {
    
    class MainWin : public Gtk::Window {
    public:
        MainWin();
        virtual ~MainWin();

    protected:
        bool on_delete_event(GdkEventAny* event);

    private:        
        void init_menus();
        void init_toolbar();
        void init_notebook();
        void init_popup();
        void init_fifo();

        void on_file(std::string s);
        void on_folder(std::string s);
        void on_message(std::string s);
        void on_fetch(std::string s);
        void on_settings(std::string s);
        void on_help(std::string s);

        int on_status();

        void on_preferences();
        void on_font();
        void on_reset();

        void on_prefs_apply();
        void on_prefs_cancel();

        void on_view_text();
        void on_view_html();

        void on_view_display_images();
        
        void on_window_next();
        void on_window_prev();
        
        void init_fetches();
        bool can_fetch();
        int fetch(bool del=false);

        void set_passbuf(std::string pass);

        void read_mailto_url();
        bool pipe_mailto_url(Glib::IOCondition c);

        void on_quit();

        void on_new(Config::MailBox box);
        void on_edited(Config::MailBox box);
        void on_deleted(Config::MailBox box);

        void add_box(Config::MailBox box);
        void remove_box(Config::MailBox box);

        Glib::RefPtr<Gtk::UIManager> m_ui;
        Glib::RefPtr<Gtk::ActionGroup> m_actions;

        Gtk::VBox* m_vbox;

        Gtk::Toolbar* m_tools;
        Gtk::Notebook* m_boxes;
        Gtk::Statusbar* m_status;

        std::map<Glib::StaticMutex*, jlib::net::MailFetch*> m_fetches;
        std::string m_passbuf;
        Gtk::Menu* m_save_popup;

        std::string m_mailto_file;
        Glib::Thread* m_mailto_thread;
        jlib::sys::pipe m_mailto_pipe;
    };
    
}

#endif //GTKMAIL_MAINWIN_HH
