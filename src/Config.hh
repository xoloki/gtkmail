/* -*- mode:C++ c-basic-offset:4  -*-
 * Config.hh - defines interfaces to config stuff
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

#ifndef GTKMAIL_CONFIG_HH
#define GTKMAIL_CONFIG_HH

#include <string>
#include <list>
#include <functional>
#include <algorithm>

#include <jlib/sys/object.hh>
#include <jlib/util/URL.hh>
#include <jlib/util/xml.hh>
#include <jlib/crypt/crypt.hh>

#include <sigc++/sigc++.h>

#include <gtkmm/widget.h>
#include <gtkmm/toolbar.h>

using namespace jlib;
using namespace jlib::util;

namespace gtkmail {

    class MailBox;

    class Config : public jlib::sys::Object {
    public:
        class no_password : public std::exception {
        public:
            virtual const char* what() const throw() { 
                return "No password";
            }
        };

        class bad_password : public std::exception {
        public:
            virtual const char* what() const throw() { 
                return "Bad password";
            }
        };

        class no_font : public std::exception {
        public:
            virtual const char* what() const throw() { 
                return "No font entry in config file";
            }
        };

        class no_style : public std::exception {
        public:
            virtual const char* what() const throw() { 
                return "No toolbar style entry in config file";
            }
        };

        class no_charset : public std::exception {
        public:
            virtual const char* what() const throw() { 
                return "No default charset entry in config file";
            }
        };

        class bad_style : public std::exception {
        public:
            bad_style(std::string style) 
                : m_style(style) {}

            virtual ~bad_style() throw() {}

            virtual const char* what() const throw() { 
                return ("bad style "+m_style).c_str();
            }

            std::string m_style;
        };

        class MailBox {
        public:
            MailBox();
            MailBox(std::string url,std::string name,std::string addr,std::string time);
            MailBox(xml::node::ptr node);

            std::string get_url() const;
            std::string get_name() const;
            std::string get_addr() const;

            std::string get_smtp() const;
            unsigned int get_smtp_port() const;
            bool get_smtp_tls() const;
            bool get_smtp_auth() const;
            bool get_smtp_auth_same() const;
            std::string get_smtp_user() const;
            std::string get_smtp_pass() const;

            bool get_idle() const;

            int get_time() const;
            int get_folder_pos() const;
            int get_message_pos() const;

            void set_url(std::string url);
            void set_name(std::string s);
            void set_addr(std::string s);

            void set_smtp(std::string s);
            void set_smtp_port(unsigned int port);
            void set_smtp_tls(bool b);
            void set_smtp_auth(bool b);
            void set_smtp_auth_same(bool b);
            void set_smtp_user(std::string s);
            void set_smtp_pass(std::string s);

            void set_idle(bool b);

            void set_time(int t);
            void set_folder_pos(int t);
            void set_message_pos(int t);

            xml::node::ptr get_node();
            const xml::node::ptr get_node() const;
            void set_node(xml::node::ptr node);

            void copy(const MailBox& box);
            MailBox copy();

        protected:
            xml::node::ptr m_node;
        };

        typedef std::list<MailBox> rep_type;
        typedef rep_type::pointer pointer;
        typedef rep_type::const_pointer const_pointer;
        typedef rep_type::reference reference;
        typedef rep_type::const_reference const_reference;
        typedef rep_type::iterator iterator;
        typedef rep_type::const_iterator const_iterator; 
        typedef rep_type::reverse_iterator reverse_iterator;
        typedef rep_type::const_reverse_iterator const_reverse_iterator;
        typedef rep_type::size_type size_type;
        typedef rep_type::difference_type difference_type;
        typedef rep_type::allocator_type allocator_type;            

        iterator begin() { return m_mailboxes.begin(); }
        const_iterator begin() const { return m_mailboxes.begin(); }
        iterator end() { return m_mailboxes.end(); }
        const_iterator end() const { return m_mailboxes.end(); }
        reverse_iterator rbegin() { return m_mailboxes.rbegin(); }
        const_reverse_iterator rbegin() const { return m_mailboxes.rbegin(); }
        reverse_iterator rend() { return m_mailboxes.rend(); }
        const_reverse_iterator rend() const { return m_mailboxes.rend(); }
        bool empty() const { return m_mailboxes.empty(); }
        size_type size() const { return m_mailboxes.size(); }

        reference back() { return m_mailboxes.back(); }
        const_reference back() const { return m_mailboxes.back(); }
        reference front() { return m_mailboxes.front(); }
        const_reference front() const { return m_mailboxes.front(); }

        iterator find(std::string name);
        const_iterator find(std::string name) const;
        
        void push_back(const_reference r);
        void erase(iterator i);
        void replace(iterator i, const_reference r);
        void clear();
        
        Config();
        Config(bool copy);
        Config(std::string file);

        bool use_toolbar_text();

        std::list<MailBox> get_mailboxes();
        
        void load_pixbufs();

        void load();
        void save();

        void load(std::string file);
        void save(std::string file);
        void create(std::string file);

        xml::node::list::iterator find_mailbox_node(std::string name);

        friend std::istream& operator>>(std::istream& i, Config& config);
        friend std::ostream& operator<<(std::ostream& o, const Config& config);
        
        static Config global;
        
        std::string get_gtkmail_dir();

        std::string get_message_font();
        void set_message_font(std::string font);

        std::string get_default_charset();
        void set_default_charset(std::string charset);

        std::vector<std::string> get_my_addrs();

        Gtk::ToolbarStyle get_toolbar_style();
        void set_toolbar_style(Gtk::ToolbarStyle style);

        std::string get_user_style();
        void set_user_style(std::string s);

        bool get_auto_load();
        void set_auto_load(bool b);

        Glib::RefPtr<Gdk::Pixbuf> mail_compose_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_view_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_reply_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_reply_all_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_delete_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_forward_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_save_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_send_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_receive_buf;

        Glib::RefPtr<Gdk::Pixbuf> mail_new_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_read_buf;
        Glib::RefPtr<Gdk::Pixbuf> mail_replied_buf;

        Glib::RefPtr<Gdk::Pixbuf> attachment_buf;
        Glib::RefPtr<Gdk::Pixbuf> empty_buf;

        Glib::RefPtr<Gdk::Pixbuf> app_buf;
        Glib::RefPtr<Gdk::Pixbuf> audio_buf;
        Glib::RefPtr<Gdk::Pixbuf> image_buf;
        Glib::RefPtr<Gdk::Pixbuf> multi_buf;
        Glib::RefPtr<Gdk::Pixbuf> text_buf;

        Glib::RefPtr<Gdk::Pixbuf> cancel_buf;
        Glib::RefPtr<Gdk::Pixbuf> save_buf;

        Glib::RefPtr<Gdk::Pixbuf> dir_open_buf;
        Glib::RefPtr<Gdk::Pixbuf> dir_close_buf;

    protected:
        xml::node::ptr find_node(std::string name);
        xml::node::ptr append(std::string name, std::string key = "", std::string val = "");

        std::string get_pref(std::string key, std::string def);
        void set_pref(std::string key, std::string val);

        void parse_document();

        void get_gpg_id();

        xml::document m_doc;
        std::string m_file;
        std::string m_gpg_id_file;
        std::string m_gpg_id;
        bool m_use_toolbar_text;

        rep_type m_mailboxes;
        //Gnome::App* m_app;
    };

}

#endif //GTKMAIL_CONFIG_HH
