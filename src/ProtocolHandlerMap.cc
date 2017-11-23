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

#include "ProtocolHandlerMap.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>

#include <jlib/util/util.hh>
#include <jlib/sys/sys.hh>

const std::string XML_HEADER = "<?xml version=\"1.0\"?>";

namespace gtkmail {

    ProtocolHandlerMap ProtocolHandlerMap::global;

    ProtocolHandler::ProtocolHandler() 
    {
        m_node = xml::node::create("protocol-handler"); 
    }

    ProtocolHandler::ProtocolHandler(std::string protocol, std::string handler)
    {
        m_node = xml::node::create("protocol-handler"); 

        set_protocol(protocol);
        set_handler(handler);
    }
    
    ProtocolHandler::ProtocolHandler(xml::node::ptr node) 
    {
        set_node(node);
    }
    
    std::string ProtocolHandler::get_protocol() const {
        return m_node->get_attribute("protocol");
    }

    std::string ProtocolHandler::get_handler() const {
        return m_node->get_attribute("handler");
    }

    void ProtocolHandler::set_protocol(std::string protocol) {
        m_node->set_attribute("protocol", protocol);
    }

    void ProtocolHandler::set_handler(std::string s) {
        m_node->set_attribute("handler", s);
    }

    xml::node::ptr ProtocolHandler::get_node() {
        return m_node;
    }

    const xml::node::ptr ProtocolHandler::get_node() const {
        return m_node;
    }

    void ProtocolHandler::set_node(xml::node::ptr node) {
        m_node = node;
    }

    std::string ProtocolHandler::get_full() const {
        return (get_protocol() + " -> " + get_handler());
    }
    
    ProtocolHandlerMap::ProtocolHandlerMap() {
        if(getenv("HOME") != 0) {
            m_file = get_gtkmail_dir()+"/protocol-handler-map.xml";

            std::ifstream ifs(m_file.c_str());
            if(ifs.fail()) {
                std::string protocol_handler_map_dir = get_gtkmail_dir();
                
                mkdir(protocol_handler_map_dir.c_str(), 0700);
                
                std::ofstream ofs(m_file.c_str());
                ofs << XML_HEADER << std::endl
                    << "<protocol-handler-map>" << std::endl
                    << "</protocol-handler-map>" << std::endl;
                ofs.close();
            }
            else {
                ifs.close();
            }
            
            load();
        }
        
    }

    ProtocolHandlerMap::rep_type& ProtocolHandlerMap::get() {
        return m_protocol_handlers;
    }
    
    void ProtocolHandlerMap::load() {
        load(m_file);
    }

    void ProtocolHandlerMap::save() {
        save(m_file);
    }
    
    void ProtocolHandlerMap::load(std::string file) {
        std::ifstream ifs(file.c_str());
        ifs >> *this;
    }

    void ProtocolHandlerMap::save(std::string file) {
        std::ofstream ofs(file.c_str());
        ofs << *this;
    }


    void ProtocolHandlerMap::parse_document() {
        xml::node::list root = m_doc.get_list();
        if(root.size() != 1 || root.front()->get_name() != "protocol-handler-map") {
            std::cerr << "gtkmail::ProtocolHandlerMap::parse_document(): error"<<std::endl;
            std::cerr << "\troot.size() = "<<root.size()<<std::endl;
            if(root.size() >= 1)
                std::cerr << "\troot.front()->get_name() = "<< root.front()->get_name() 
                          <<std::endl;
            exit(1);
        }
        
        m_protocol_handlers.clear();
        xml::node::list toplevel = root.front()->get_list();
        xml::node::list::iterator i = toplevel.begin();
        for(;i!=toplevel.end();i++) {
            xml::node::ptr node = *i;
            if(node->get_name() == "protocol-handler") {
                ProtocolHandler handler(node);
                m_protocol_handlers[handler.get_protocol()] = handler;
            }
        }
    }

    void ProtocolHandlerMap::clear() { 
        m_protocol_handlers.clear(); 
    }

    void ProtocolHandlerMap::insert(const ProtocolHandler& r) {
        m_protocol_handlers[r.get_protocol()] = r;
        xml::node::ptr node = r.get_node();
        m_doc.get_list().front()->add(node);
    }

    void ProtocolHandlerMap::erase(iterator i) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator j = find_iter(i->second.get_protocol());
        if(j != list.end()) {
            list.erase(j);
        }
    }
    
    ProtocolHandlerMap::iterator ProtocolHandlerMap::find(std::string protocol) {
        return m_protocol_handlers.find(protocol);
    }

    ProtocolHandlerMap::const_iterator ProtocolHandlerMap::find(std::string protocol) const {
        return m_protocol_handlers.find(protocol);
    }

    ProtocolHandlerMap::iterator ProtocolHandlerMap::find_handler(std::string handler) {
        iterator i = begin();

        for(;i!=end();i++) {
            if(i->second.get_handler() == handler)
                return i;
        }

        return end();
    }

    ProtocolHandlerMap::const_iterator ProtocolHandlerMap::find_handler(std::string handler) const {
        const_iterator i = begin();

        for(;i!=end();i++) {
            if(i->second.get_handler() == handler)
                return i;
        }

        return end();
    }

    xml::node::list::iterator ProtocolHandlerMap::find_iter(std::string protocol) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = list.begin();
        for(; i != list.end(); i++) {
            if((*i)->get_attribute("protocol") == protocol) {
                return i;
            }
        }
        return list.end();
    }

    std::string ProtocolHandlerMap::get_gtkmail_dir() {
        return (std::string(getenv("HOME"))+"/.gtkmail");
    }

    xml::node::ptr ProtocolHandlerMap::find_node(std::string protocol) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = find_iter(protocol);
        if(i != list.end()) {
            return (*i);
        }
        
        return xml::node::ptr();
    }

    std::istream& operator>>(std::istream& i, ProtocolHandlerMap& protocol_handler_map) {
        protocol_handler_map.m_doc.load(i);
        protocol_handler_map.parse_document();
        return i;
    }

    std::ostream& operator<<(std::ostream& o, const ProtocolHandlerMap& protocol_handler_map) {
        protocol_handler_map.m_doc.save(o);
        return o;
    }


}
