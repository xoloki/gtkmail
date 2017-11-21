/* -*- mode:C++ c-basic-offset:4  -*-
 * AddressBook.hh - defines interfaces to addressbook stuff
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

#include "URIHandlerMap.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>

#include <jlib/util/util.hh>
#include <jlib/sys/sys.hh>

const std::string XML_HEADER = "<?xml version=\"1.0\"?>";

namespace gtkmail {

    URIHandlerMap URIHandlerMap::global;

    URIHandler::URIHandler() 
    {
        m_node = xml::node::create("uri-handler"); 
    }

    URIHandler::URIHandler(std::string uri, std::string handler)
    {
        m_node = xml::node::create("uri-handler"); 

        set_uri(uri);
        set_handler(handler);
    }
    
    URIHandler::URIHandler(xml::node::ptr node) 
    {
        set_node(node);
    }
    
    std::string URIHandler::get_uri() const {
        return m_node->get_attribute("uri");
    }

    std::string URIHandler::get_handler() const {
        return m_node->get_attribute("handler");
    }

    void URIHandler::set_uri(std::string uri) {
        m_node->set_attribute("uri", uri);
    }

    void URIHandler::set_handler(std::string s) {
        m_node->set_attribute("handler", s);
    }

    xml::node::ptr URIHandler::get_node() {
        return m_node;
    }

    const xml::node::ptr URIHandler::get_node() const {
        return m_node;
    }

    void URIHandler::set_node(xml::node::ptr node) {
        m_node = node;
    }

    std::string URIHandler::get_full() const {
        return (get_uri() + " -> " + get_handler());
    }
    
    URIHandlerMap::URIHandlerMap() {
        if(getenv("HOME") != 0) {
            m_file = get_gtkmail_dir()+"/uri-handler-map.xml";

            std::ifstream ifs(m_file.c_str());
            if(ifs.fail()) {
                std::string uri_handler_map_dir = get_gtkmail_dir();
                
                mkdir(uri_handler_map_dir.c_str(), 0700);
                
                std::ofstream ofs(m_file.c_str());
                ofs << XML_HEADER << std::endl
                    << "<uri-handler-map>" << std::endl
                    << "</uri-handler-map>" << std::endl;
                ofs.close();
            }
            else {
                ifs.close();
            }
            
            load();
        }
        
    }

    URIHandlerMap::rep_type& URIHandlerMap::get() {
        return m_uri_handlers;
    }
    
    void URIHandlerMap::load() {
        load(m_file);
    }

    void URIHandlerMap::save() {
        save(m_file);
    }
    
    void URIHandlerMap::load(std::string file) {
        std::ifstream ifs(file.c_str());
        ifs >> *this;
    }

    void URIHandlerMap::save(std::string file) {
        std::ofstream ofs(file.c_str());
        ofs << *this;
    }


    void URIHandlerMap::parse_document() {
        xml::node::list root = m_doc.get_list();
        if(root.size() != 1 || root.front()->get_name() != "uri-handler-map") {
            std::cerr << "gtkmail::URIHandlerMap::parse_document(): error"<<std::endl;
            std::cerr << "\troot.size() = "<<root.size()<<std::endl;
            if(root.size() >= 1)
                std::cerr << "\troot.front()->get_name() = "<< root.front()->get_name() 
                          <<std::endl;
            exit(1);
        }
        
        m_uri_handlers.clear();
        xml::node::list toplevel = root.front()->get_list();
        xml::node::list::iterator i = toplevel.begin();
        for(;i!=toplevel.end();i++) {
            xml::node::ptr node = *i;
            if(node->get_name() == "uri-handler") {
                URIHandler handler(node);
                m_uri_handlers[handler.get_uri()] = handler;
            }
        }
    }

    void URIHandlerMap::clear() { 
        m_uri_handlers.clear(); 
    }

    void URIHandlerMap::insert(const URIHandler& r) {
        m_uri_handlers[r.get_uri()] = r;
        xml::node::ptr node = r.get_node();
        m_doc.get_list().front()->add(node);
    }

    void URIHandlerMap::erase(iterator i) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator j = find_iter(i->second.get_uri());
        if(j != list.end()) {
            list.erase(j);
        }
    }
    
    URIHandlerMap::iterator URIHandlerMap::find(std::string uri) {
        return m_uri_handlers.find(uri);
    }

    URIHandlerMap::const_iterator URIHandlerMap::find(std::string uri) const {
        return m_uri_handlers.find(uri);
    }

    URIHandlerMap::iterator URIHandlerMap::find_handler(std::string handler) {
        iterator i = begin();

        for(;i!=end();i++) {
            if(i->second.get_handler() == handler)
                return i;
        }

        return end();
    }

    URIHandlerMap::const_iterator URIHandlerMap::find_handler(std::string handler) const {
        const_iterator i = begin();

        for(;i!=end();i++) {
            if(i->second.get_handler() == handler)
                return i;
        }

        return end();
    }

    xml::node::list::iterator URIHandlerMap::find_iter(std::string uri) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = list.begin();
        for(; i != list.end(); i++) {
            if((*i)->get_attribute("uri") == uri) {
                return i;
            }
        }
        return list.end();
    }

    std::string URIHandlerMap::get_gtkmail_dir() {
        return (std::string(getenv("HOME"))+"/.gtkmail");
    }

    xml::node::ptr URIHandlerMap::find_node(std::string uri) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = find_iter(uri);
        if(i != list.end()) {
            return (*i);
        }
        
        return xml::node::ptr();
    }

    xml::node::ptr URIHandlerMap::append(std::string uri, std::string name, std::string handler) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::ptr node = xml::node::create(name); 

        node->set_attribute("uri", uri);
        node->set_attribute("name", name);
        node->set_attribute("handler", handler);

        list.insert(list.begin(), node);

        return node;
    }

    
    std::istream& operator>>(std::istream& i, URIHandlerMap& uri_handler_map) {
        uri_handler_map.m_doc.load(i);
        uri_handler_map.parse_document();
        return i;
    }

    std::ostream& operator<<(std::ostream& o, const URIHandlerMap& uri_handler_map) {
        uri_handler_map.m_doc.save(o);
        return o;
    }


}
