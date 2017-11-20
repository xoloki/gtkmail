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

#include "AddressBook.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>

#include <jlib/util/util.hh>
#include <jlib/sys/sys.hh>

const std::string XML_HEADER = "<?xml version=\"1.0\"?>";

namespace gtkmail {

    AddressBook AddressBook::global;

    Address::Address() 
    {
        m_node = xml::node::create("address"); 
    }

    Address::Address(std::string key, std::string name, std::string addr)
    {
        m_node = xml::node::create("address"); 

        set_key(key);
        set_name(name);
        set_addr(addr);
    }
    
    Address::Address(xml::node::ptr node) 
    {
        set_node(node);
    }
    
    std::string Address::get_key() const {
        return m_node->get_attribute("key");
    }

    std::string Address::get_name() const {
        return m_node->get_attribute("name");
    }

    std::string Address::get_addr() const {
        return m_node->get_attribute("addr");
    }

    void Address::set_key(std::string key) {
        m_node->set_attribute("key", key);
    }

    void Address::set_name(std::string s) {
        m_node->set_attribute("name", s);
    }

    void Address::set_addr(std::string s) {
        m_node->set_attribute("addr", s);
    }

    xml::node::ptr Address::get_node() {
        return m_node;
    }

    const xml::node::ptr Address::get_node() const {
        return m_node;
    }

    void Address::set_node(xml::node::ptr node) {
        m_node = node;
    }

    std::string Address::get_full() const {
        return (get_name() + " <" + get_addr() + ">");
    }
    
    AddressBook::AddressBook() {
        if(getenv("HOME") != 0) {
            m_file = get_gtkmail_dir()+"/addressbook.xml";

            std::ifstream ifs(m_file.c_str());
            if(ifs.fail()) {
                std::string addressbook_dir = get_gtkmail_dir();
                
                mkdir(addressbook_dir.c_str(), 0700);
                
                std::ofstream ofs(m_file.c_str());
                ofs << XML_HEADER << std::endl
                    << "<addressbook>" << std::endl
                    << "</addressbook>" << std::endl;
                ofs.close();
            }
            else {
                ifs.close();
            }
            
            load();
        }
        
    }

    std::list<Address> AddressBook::get_addresses() {
        return m_addresses;
    }
    
    void AddressBook::load() {
        load(m_file);
    }

    void AddressBook::save() {
        save(m_file);
    }
    
    void AddressBook::load(std::string file) {
        std::ifstream ifs(file.c_str());
        ifs >> *this;
    }

    void AddressBook::save(std::string file) {
        std::ofstream ofs(file.c_str());
        ofs << *this;
    }


    void AddressBook::parse_document() {
        xml::node::list root = m_doc.get_list();
        if(root.size() != 1 || root.front()->get_name() != "addressbook") {
            std::cerr << "gtkmail::AddressBook::parse_document(): error"<<std::endl;
            std::cerr << "\troot.size() = "<<root.size()<<std::endl;
            if(root.size() >= 1)
                std::cerr << "\troot.front()->get_name() = "<< root.front()->get_name() 
                          <<std::endl;
            exit(1);
        }
        
        m_addresses.clear();
        xml::node::list toplevel = root.front()->get_list();
        xml::node::list::iterator i = toplevel.begin();
        for(;i!=toplevel.end();i++) {
            xml::node::ptr node = *i;
            if(node->get_name() == "address") {
                m_addresses.push_back(Address(node));
            }
        }
    }

    void AddressBook::clear() { 
        m_addresses.clear(); 
    }

    void AddressBook::push_back(const_reference r) {
        m_addresses.push_back(r);
        xml::node::ptr node = r.get_node();
        m_doc.get_list().front()->add(node);
    }

    void AddressBook::erase(iterator i) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator j = find_iter(i->get_key());
        if(j != list.end()) {
            list.erase(j);
        }
    }
    
    // XXX this doesn't actually work, but it isn't being used
    void AddressBook::replace(iterator i, const_reference r) {
        if(i != end()) {
            i->set_node(r.get_node());
        }
    }

    AddressBook::iterator AddressBook::find(std::string key) {
        iterator i = begin();

        for(;i!=end();i++) {
            if(i->get_key() == key)
                return i;
        }

        return end();
    }

    AddressBook::const_iterator AddressBook::find(std::string key) const {
        const_iterator i = begin();

        for(;i!=end();i++) {
            if(i->get_key() == key)
                return i;
        }

        return end();
    }

    AddressBook::iterator AddressBook::find_addr(std::string addr) {
        iterator i = begin();

        for(;i!=end();i++) {
            if(i->get_addr() == addr)
                return i;
        }

        return end();
    }

    AddressBook::const_iterator AddressBook::find_addr(std::string addr) const {
        const_iterator i = begin();

        for(;i!=end();i++) {
            if(i->get_addr() == addr)
                return i;
        }

        return end();
    }

    xml::node::list::iterator AddressBook::find_iter(std::string key) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = list.begin();
        for(; i != list.end(); i++) {
            if((*i)->get_attribute("key") == key) {
                return i;
            }
        }
        return list.end();
    }

    std::string AddressBook::get_gtkmail_dir() {
        return (std::string(getenv("HOME"))+"/.gtkmail");
    }

    xml::node::ptr AddressBook::find_node(std::string key) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = find_iter(key);
        if(i != list.end()) {
            return (*i);
        }
        
        return xml::node::ptr();
    }

    xml::node::ptr AddressBook::append(std::string key, std::string name, std::string addr) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::ptr node = xml::node::create(name); 

        node->set_attribute("key", key);
        node->set_attribute("name", name);
        node->set_attribute("addr", addr);

        list.insert(list.begin(), node);

        return node;
    }

    
    std::istream& operator>>(std::istream& i, AddressBook& addressbook) {
        addressbook.m_doc.load(i);
        addressbook.parse_document();
        return i;
    }

    std::ostream& operator<<(std::ostream& o, const AddressBook& addressbook) {
        addressbook.m_doc.save(o);
        return o;
    }


}
