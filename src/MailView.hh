/* -*- mode: C++ c-basic-offset: 4  -*-
 * MailView.hh - header file for class MailView
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class MailView provide both a text-based and an html-based view 
 * of a jlib::net::Email data object.  It can switch between the two at
 * will, or only use one (in case you don't even want html mail running).
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

#ifndef GTKMAIL_MAILVIEW_HH
#define GTKMAIL_MAILVIEW_HH

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>
#include <gtkmm/box.h>
#include <gtkmm/textview.h>
#include <gtkmm/fontselection.h>
#include <gtkmm/fileselection.h>
#include <gtkmm/statusbar.h>

#include <jlib/net/Email.hh>

#include <webkit/webkitwebsettings.h>
#include <webkit/webkitwebview.h>
#include <webkit/webkitwebnavigationaction.h>

namespace gtkmail {

    class MailView : public Gtk::ScrolledWindow {
    public:
        class exception : public std::exception {
        public:
            exception(std::string msg = "") {
                m_msg = "gtkmail::MailView::exception" + ((msg != "") ? (": "+msg) : "");
            }
            virtual ~exception() throw() {}
            virtual const char* what() const throw() { return m_msg.c_str(); }
        protected:
            std::string m_msg;
        };

        MailView(Gtk::Statusbar* statusbar);

        void set_data(jlib::net::Email data);
        void clear();

        void refresh();

        static Glib::ustring get_header(jlib::net::Email data, std::string header);
        static Glib::ustring get_data(jlib::net::Email data);
        static Glib::ustring get(std::string data, std::string charset="");
        static std::string unget(std::string data, std::string charset="");

        static Gtk::Widget* wrap_html(jlib::net::Email data, std::string from, Gtk::Statusbar* status = nullptr);
        static Gtk::Widget* wrap_html(std::string data, std::string from, Gtk::Statusbar* status = nullptr);

        static std::string get_trim_ctype(const jlib::net::Email& e);
        static std::string get_ctype_desc(std::string ctype);

    private:
        void set_data(jlib::net::Email data, int level);
        void set_data_text(jlib::net::Email data, int level);

        void print_headers_text(jlib::net::Email data);

        void print_ctype_text(jlib::net::Email data);

        void append_widget_text(Gtk::Widget* w, bool endl = true);
        void append_separator_text();

        void on_show_clicked(Gtk::Image* image, Gtk::Widget* child);
        void on_save_clicked(jlib::net::Email data);
        void on_save(Gtk::FileSelection* file, jlib::net::Email data);

        void on_html_view_show(Gtk::Widget* w);

        static gboolean on_navigation_policy_decision_requested(WebKitWebView* web_view, WebKitWebFrame* frame, WebKitNetworkRequest* request, WebKitWebNavigationAction* navigation_action, WebKitWebPolicyDecision* policy_decision, gpointer user_data);
        
        static WebKitNavigationResponse on_navigation_requested(WebKitWebView*web_view, WebKitWebFrame *frame, WebKitNetworkRequest *request);

        static gboolean on_download_requested(WebKitWebView  *web_view, WebKitDownload *download, gpointer user_data);

        static gboolean on_mime_type_policy_decision_requested (WebKitWebView* web_view, WebKitWebFrame* frame, WebKitNetworkRequest* request, gchar* mimetype, WebKitWebPolicyDecision* policy_decision, gpointer user_data);

        static void on_hovering_over_link(WebKitWebView *web_view, gchar* title, gchar* uri, gpointer user_data);

        static WebKitWebView* create_web_view(WebKitWebView* web_view, WebKitWebFrame* frame, gpointer user_data);

        static gboolean on_new_window_policy_decision_requested(WebKitWebView* web_view, WebKitWebFrame* frame, WebKitNetworkRequest* request, WebKitWebNavigationAction* navigation_action, WebKitWebPolicyDecision* policy_decision, gpointer user_data);
        
        Gtk::TextView* create_text_view(std::string data);

        Gtk::VBox* m_box;
        jlib::net::Email m_data;
        Gtk::Statusbar* m_statusbar;
    };
    
}

#endif //GTKMAIL_MAILVIEW_HH
