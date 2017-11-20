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

#ifndef GTKMAIL_CONFIG_HH
#define GTKMAIL_CONFIG_HH

#include "common.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <map>
#include <string>
#include <fstream>

#include <jlib/util/util.hh>

const std::string BASE = std::string(getenv("HOME"))+std::string("/.")+std::string(PACKAGE);
const std::string CONFIG = BASE+std::string("/config");
const std::string ADDRESS = BASE+std::string("/address");
const std::string FILTER = BASE+std::string("/filter");

const std::string SHARE = std::string(DATADIR) + std::string("/gtkmail");

namespace gtkmail {
namespace config {
        
std::string share() { return SHARE; }
    
bool has_config_file() {
    std::string file = CONFIG+".xml";
    std::ifstream in(file.c_str());
    return (!in.fail());
}
    
bool has_valid_config_file() {
    std::string file = CONFIG+".xml";
    std::ifstream in(file.c_str());
    jlib::util::xml::xmldocument doc;
    if(doc.load(in)) {
        return true;
    }
    else {
        return false;
    }
    
}
    
jlib::util::xml::document get_xml(std::string file) {
    std::ifstream in(file.c_str());
    jlib::util::xml::document doc;
    if(doc.load(in,ctx)) {
        return doc;
    }
    else {
        std::cerr << "error loading xml config file '"<<file<<"'"<<std::endl;
        exit(1);
    }
}
    
    
jlib::util::xml::document xml_config() {
    return get_xml(CONFIG+".xml");
}
    
std::list<jlib::util::xml::xmlnode*> get_toplevel(std::string file,std::string top) {
    using namespace jlib::util::xml;
    xmldocument doc = get_xml(file);
    std::list<xmlnode*> root = doc.get_nodelist();
    if(root.size() < 1 || root.front()->get_nodename() != top) {
        std::cerr << "error loading xml address file '"<<file<<"' in get_address()"<<std::endl;
        std::cerr << "root.size() = "<<root.size()<<std::endl;
        if(root.size() >= 1)
            std::cerr << "root.front()->get_nodename() = "<< root.front()->get_nodename() <<std::endl;
        exit(1);
    }
    
    return root.front()->get_nodelist();
}

std::map<std::string,std::string> get_address() {
    std::map<std::string,std::string> ret;
    
    using namespace jlib::util::xml;
    std::string file = ADDRESS+".xml";
    
    std::list<xmlnode*> entries = get_toplevel(file,"addressbook");
    std::list<xmlnode*>::iterator i = entries.begin();
    for(;i!=entries.end();i++) {
        xmlnode* node = *i;
        if(node->get_nodename() == "entry") {
            xmlattributes attr = (*i)->get_attributes();
            ret[attr["id"]] = attr["name"]+" <"+attr["addr"]+">";
        }
    }
    
    return ret;
}
    
std::vector<std::string> my_address() {
    std::vector<std::string> ret;
    std::map< std::string, std::map< std::string,std::string> > boxes = get_mailbox_info();
    std::map< std::string, std::map< std::string,std::string> >::iterator i = boxes.begin();
    for(;i!=boxes.end();i++) {
        ret.push_back(i->second["addr"]);
    }
    return ret;
}

std::map< std::string, std::map<std::string,std::string> > get_box_info(std::string type) {
    std::map< std::string, std::map<std::string,std::string> > ret;
    
    using namespace jlib::util::xml;
    std::string file = CONFIG+".xml";
    
    std::list<xmlnode*> entries = get_toplevel(file,"gtkmail");
    std::list<xmlnode*>::iterator i = entries.begin();
    for(;i!=entries.end();i++) {
        xmlnode* node = *i;
        if(node->get_nodename() == type) {
            std::map<std::string,std::string> attr = node->get_attributes().get_attrmap();
            xmlnodelist list = node->get_nodelist();
            if(list.size() > 0) {
                attr["url"] = list.front()->get_cdata();
            }
            else {
                        
            }
                    
            ret[attr["name"]] = attr;
        }
    }

    return ret;
}

std::map<std::string,std::string> get_smtp_server() {
    std::map<std::string,std::string> def;
    def["type"] = "smtp";
    def["host"] = "localhost";

    using namespace jlib::util::xml;
    std::string file = CONFIG+".xml";

    std::list<xmlnode*> entries = get_toplevel(file,"gtkmail");
    std::list<xmlnode*>::iterator i = entries.begin();
    for(;i!=entries.end();i++) {
        xmlnode* node = *i;
        if(node->get_nodename() == "server") {
            xmlattributes attr = node->get_attributes();
            if(attr["type"] == "smtp")
                return attr.get_attrmap();
        }
    }

    return def;
}

std::map< std::string, std::map<std::string,std::string> > get_mailbox_info() {
    return get_box_info("mailbox");
}

std::map< std::string, std::map<std::string,std::string> > get_fetch_info() {
    return get_box_info("fetch");
}
    
std::map<std::string,std::string> get_mailbox_info(const std::string& box) {
    return get_mailbox_info()[box];
}
    
std::string get_mailbox_info(const std::string& box, const std::string& key) {
    return get_mailbox_info()[box][key];
}
    
std::vector<std::string> get_mailbox_names() {
    std::vector<std::string> ret;

    using namespace jlib::util::xml;
    std::string file = CONFIG+".xml";

    std::list<xmlnode*> entries = get_toplevel(file,"gtkmail");
    std::list<xmlnode*>::iterator i = entries.begin();
    for(;i!=entries.end();i++) {
        xmlnode* node = *i;
        if(node->get_nodename() == "mailbox") {
            xmlattributes attr = node->get_attributes();
            ret.push_back(attr["name"]);
        }
    }

    return ret;
}

bool use_toolbar_text() {
    using namespace jlib::util::xml;
    std::string file = CONFIG+".xml";

    std::list<xmlnode*> entries = get_toplevel(file,"gtkmail");
    std::list<xmlnode*>::iterator i = entries.begin();
    for(;i!=entries.end();i++) {
        xmlnode* node = *i;
        if(node->get_nodename() == "toolbar") {
            xmlattributes attr = node->get_attributes();
            if(attr["text"] == "true")
                return true;
            else if(attr["text"] == "false")
                return false;
        }
    }
    return true;
}

std::string get_message_font() {
    using namespace jlib::util::xml;
    node::list& list = m_doc.front()->get_list();
    for(node::list::iterator i=list.begin();i!=list.end();i++) {
                
    }

    std::string file = CONFIG+".xml";

    std::list<xmlnode*> entries = get_toplevel(file,"gtkmail");
    std::list<xmlnode*>::iterator i = entries.begin();
    for(;i!=entries.end();i++) {
        xmlnode* node = *i;
        if(node->get_nodename() == "font") {
            xmlattributes attr = node->get_attributes();
            return attr["name"];
        }
    }
    return "";
}

}

jlib::net::Email decrypt(const jlib::net::Email& email, gpgme_verify_result_t* verify_result = nullptr) {
    using namespace jlib::crypt;
    gpg::ctx ctx;
    gpg::data::ptr plain = gpg::data::create();
    bool show = false;
    std::string data;
    
    if(util::begins(email["Content-Type"], "multipart/encrypted")) {
        // check for two attachments: application/pgp-encrypted then octet data
        if(email.attach().size() == 2 && email[0]["Content-Type"] == "application/pgp-encrypted" && email[1]["Content-Type"] == "application/octet-stream") {
            data = email[1].data();
        } else {
            data = email.data();
        }
    } else {
        try {
            net::Email::reference r = email.grep("BEGIN PGP MESSAGE");
            data = r.data();
        } catch(net::Email::exception&) {
            data = email.data();
        }
    }
    
    gpg::data::ptr cipher = gpg::data::create(data);
    gpgme_verify_result_t result = ctx.op_decrypt_verify(cipher, plain);
    
    if(verify_result != nullptr)
        *verify_result = result;
    }
}
    
}

#endif //GTKMAIL_MAINWIN_HH
