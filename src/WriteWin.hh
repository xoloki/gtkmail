/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkWriteWin.h - header file for class GtkWriteWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * GtkWriteWin
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

#ifndef GTKMAIL_WRITEWIN_HH
#define GTKMAIL_WRITEWIN_HH

#include "MessageWin.hh"

#include <gtkmm/fileselection.h>
#include <gtkmm/table.h>
#include <gtkmm/combo.h>

namespace gtkmail {
    
    class MailBox;
    
    class WriteWin : public MessageWin {
    public:
        WriteWin(const jlib::net::Email& email, 
                 MailBox* box,
                 jlib::net::folder_info_type info=jlib::net::folder_info_type(),
                 bool reply=false,
                 bool reply_all=false,
                 bool reply_quote=false,
                 bool forward=false, 
                 bool forward_attach=false,
                 bool forward_spam=false);
                 
        ~WriteWin();

    protected:
        void init_fields_from_dns(std::pair<std::string,std::vector<std::string> > dns);
        void init_fields_from_whois(std::string whois);
        
        void init_menus();
        void init_toolbar();

        void begin_whois_chain();
        std::string get_whois();


        void init_fields(std::string abuse_addr, std::string spam_body);
        std::string get_signature();

        void message_call(std::string s);
        void attach_call(std::string s);
        void crypt_call(std::string s);

        void attach_file_call();
        void attach_file_exec(Gtk::FileSelection* fileSel);
        void message_send_call();

        void attach_file(std::string file);
        
        void complete_address(Gtk::Entry* field);

        void on_send(jlib::net::Email data);

        jlib::net::Email m_forward;
        std::vector<bool> m_attr;
        std::string m_user_addr;
        Glib::RefPtr<Gtk::ActionGroup> m_write_actions;
    };
    
}

#endif //GTKMAIL_WRITEWIN_HH
