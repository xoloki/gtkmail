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

#ifndef GTKMAIL_ADDRESS_BOOK_HH
#define GTKMAIL_ADDRESS_BOOK_HH

#include <string>
#include <list>
#include <functional>
#include <algorithm>

#include <jlib/util/xml.hh>

#include <sigc++/sigc++.h>

using namespace jlib::util;

namespace gtkmail {

    class Address {
    public:
        Address();
        Address(std::string key, std::string name, std::string addr);
        Address(xml::node::ptr node);
        
        std::string get_key() const;
        std::string get_name() const;
        std::string get_addr() const;

        void set_key(std::string s);
        void set_name(std::string s);
        void set_addr(std::string s);
        
        xml::node::ptr get_node();
        const xml::node::ptr get_node() const;
        void set_node(xml::node::ptr node);

        std::string get_full() const;
        
    protected:
        xml::node::ptr m_node;
    };


    class AddressBook : public jlib::sys::Object {
    public:
        class no_charset : public std::exception {
        public:
            virtual const char* what() const throw() { 
                return "No default charset entry in addressbook file";
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

        typedef std::list<Address> rep_type;
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

        iterator begin() { return m_addresses.begin(); }
        const_iterator begin() const { return m_addresses.begin(); }
        iterator end() { return m_addresses.end(); }
        const_iterator end() const { return m_addresses.end(); }
        reverse_iterator rbegin() { return m_addresses.rbegin(); }
        const_reverse_iterator rbegin() const { return m_addresses.rbegin(); }
        reverse_iterator rend() { return m_addresses.rend(); }
        const_reverse_iterator rend() const { return m_addresses.rend(); }
        bool empty() const { return m_addresses.empty(); }
        size_type size() const { return m_addresses.size(); }

        reference back() { return m_addresses.back(); }
        const_reference back() const { return m_addresses.back(); }
        reference front() { return m_addresses.front(); }
        const_reference front() const { return m_addresses.front(); }

        iterator find(std::string key);
        const_iterator find(std::string key) const;
        
        iterator find_addr(std::string addr);
        const_iterator find_addr(std::string addr) const;
        
        void push_back(const_reference r);
        void erase(iterator i);
        void replace(iterator i, const_reference r);
        void clear();
        
        AddressBook();

        std::list<Address> get_addresses();
        
        void load();
        void save();

        void load(std::string file);
        void save(std::string file);

        friend std::istream& operator>>(std::istream& i, AddressBook& addressbook);
        friend std::ostream& operator<<(std::ostream& o, const AddressBook& addressbook);
        
        static AddressBook global;
        
        std::string get_gtkmail_dir();

    protected:
        xml::node::list::iterator find_iter(std::string key);
        xml::node::ptr find_node(std::string key);
        xml::node::ptr append(std::string key, std::string name, std::string addr);

        void parse_document();

        xml::document m_doc;
        std::string m_file;

        rep_type m_addresses;
    };

}

#endif //GTKMAIL_ADDRESS_BOOK_HH
