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

#ifndef GTKMAIL_SIGNALQUEUE_HH
#define GTKMAIL_SIGNALQUEUE_HH

#include <string>
#include <list>
#include <map>
#include <queue>

#include <gtkmm/ctree.h>

#include "SignalChain.hh"

namespace gtkmail {

    class SignalQueue : public jlib::sys::Object {
    public:
        class exception : public std::exception {
        public:
            exception(const std::string& msg = "") {
                m_msg = "SignalQueue exception: "+msg;
            }
            virtual ~exception() throw() {}
            virtual const char* what() const throw() { return m_msg.c_str(); }
        protected:
            std::string m_msg;
        };

        class action {
        public:
            typedef u_int row_physical_type;
            typedef Gtk::CTree_Helpers::Row row_logical_type;
            typedef std::pair<row_physical_type,row_logical_type> row_type;

            action();
            action(std::string t, row_type r);
            action(std::string t, std::list<row_type> r);
            action(std::string t, std::list<std::string> src, std::list<std::string> dst, std::list<row_type> r);
            
            friend std::ostream& operator<<(std::ostream& o, const action& a);
            
            std::string get_type() const;
            void set_type(std::string t);
            
            std::list<std::string> get_src() const;
            void set_src(std::list<std::string> t);
            
            std::list<std::string> get_dst() const;
            void set_dst(std::list<std::string> t);
            
            row_physical_type get_physical_row() const;
            void set_physical_row(row_physical_type r);
            
            row_logical_type get_logical_row() const;
            void set_logical_row(row_logical_type r);
            
            std::list<row_physical_type> get_physical_rows() const;
            std::list<row_logical_type> get_logical_rows() const;
            
            row_type get_row() const;
            void set_row(row_type r);
            
            std::list<row_type> get_rows() const;
            void set_rows(std::list<row_type> r);
            
            SignalChainBase* get_chain() const;
            void set_chain(SignalChainBase* c);
            
        private:
            std::string m_type;
            std::list<std::string> m_src;
            std::list<std::string> m_dst;
            
            std::list<row_type> m_rows;
            SignalChainBase* m_chain;
        };
        
        SignalQueue();

        void init();

        void add(std::string name, u_int pri, sigc::slot1<SignalChainBase*,action> slot);
        void timeout();
        int gtk_timeout();
        bool preempt(std::string s, std::string t);

        void push(std::string type);
        void push(action a);
        bool empty(std::string type);
        u_int size(std::string type);

        void flush();
    protected:
        std::multimap< u_int, std::string > m_pri;
        std::map< std::string, std::queue<action> > m_rep;
        std::map< std::string, sigc::slot1< SignalChainBase*,action > > m_slt;
        action m_cur;
        bool m_preempt;
    };



}

#endif //GTKMAIL_SIGNALQUEUE_HH
