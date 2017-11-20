/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkReadWin.h - header file for class GtkReadWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * GtkReadWin
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

#ifndef GTKMAIL_READWIN_HH
#define GTKMAIL_READWIN_HH

#include "MessageWin.hh"

#include <gtkmm/fileselection.h>
#include <gtkmm/table.h>
#include <gdkmm/pixbuf.h>

namespace gtkmail {
    
    class MailBox;
    
    class ReadWin : public MessageWin {
    public:
        ReadWin(const jlib::net::Email& e, MailBox* box, jlib::net::folder_info_type info=jlib::net::folder_info_type());

        virtual ~ReadWin() {}

    protected:
        void init_headers();
        void init_menus();
        void init_toolbar();
        void init_message();

        void message_call(std::string s);
        void attach_call(std::string s);
        void crypt_call(std::string s);
        void view_call(std::string s);
        
        void message_reply_call();
        void message_reply_all_call();
        void message_forward_call();
        void message_forward_spam_call();
        void message_raw_call();

        void attach_view_text_call();
        void attach_view_html_call();
        void attach_view_image_call();
        void attach_play_audio_call();
        void attach_play_audio_exec(std::string data);
        void attach_export_call(bool decode=true);
        void attach_export_exec(Gtk::FileSelection* fileSel, bool decode);
        void attach_view_raw_call();

        std::string m_raw, m_primary, m_globbed;
        Glib::RefPtr<Gtk::ActionGroup> m_actions;
    };
    
}

#endif //GTKMAIL_READWIN_HH
