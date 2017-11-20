/* -*- mode:C++ c-basic-offset:4  -*-
 * config.hh - defines interfaces to config stuff
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

#ifndef GTKMAIL_COMMON_HH
#define GTKMAIL_COMMON_HH

#include <map>
#include <string>
#include <fstream>

#include <jlib/util/util.hh>
#include <jlib/util/xml.hh>

namespace gtkmail {

    namespace config {
     
        std::string share();

        bool has_config_file();
        bool has_valid_config_file();

        bool use_toolbar_text();


        jlib::util::xml::document xml_config();

        std::map<std::string,std::string> get_address();
        std::map<std::string,std::string> get_smtp_server();
        
        std::vector<std::string> my_address();

        std::map< std::string, std::map<std::string,std::string> > get_fetch_info();

        std::map< std::string, std::map<std::string,std::string> > get_mailbox_info();
        std::map<std::string,std::string> get_mailbox_info(const std::string& box);
        std::string get_mailbox_info(const std::string& box, const std::string& key);
        std::vector<std::string> get_mailbox_names();

        std::string get_message_font();
    }

    jlib::net::Email decrypt(const jlib::net::Email& email, bool* verify = nullptr);
    
}

#endif //GTKMAIL_MAINWIN_HH
