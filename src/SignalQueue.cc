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

#include "SignalQueue.hh"
#include <iostream>

const std::string ACTION_TYPE_NONE = "none";

namespace gtkmail {

    SignalQueue::action::action() : m_type(ACTION_TYPE_NONE), m_chain(0) {}

    SignalQueue::action::action(std::string t, row_type r) 
        : m_type(t), 
          m_chain(0) {

        m_rows.push_back(r);
    }

    SignalQueue::action::action(std::string t, std::list<row_type> r) 
        : m_type(t), 
          m_rows(r), 
          m_chain(0) {}

    SignalQueue::action::action(std::string t, std::list<std::string> src, std::list<std::string> dst, std::list<row_type> r)
        : m_type(t), m_src(src), m_dst(dst), m_rows(r), m_chain(0) {}

    std::string SignalQueue::action::get_type() const { return m_type; }
    void SignalQueue::action::set_type(std::string t) { m_type = t; }
    
    std::list<std::string> SignalQueue::action::get_src() const { return m_src; }
    void SignalQueue::action::set_src(std::list<std::string> t) { m_src = t; }
    
    std::list<std::string> SignalQueue::action::get_dst() const { return m_dst; }
    void SignalQueue::action::set_dst(std::list<std::string> t) { m_dst = t; }

    SignalQueue::action::row_type SignalQueue::action::get_row() const { return m_rows.front(); }
    void SignalQueue::action::set_row(row_type r) { m_rows.clear(); m_rows.push_back(r); }
    
    SignalQueue::action::row_physical_type SignalQueue::action::get_physical_row() const {
        if(m_rows.size() == 0) 
            return 0;
        else 
            return m_rows.front().first;
    }

    void SignalQueue::action::set_physical_row(row_physical_type r) {
        if(m_rows.size() == 0) {
            m_rows.push_back(row_type());
        }
        else if(m_rows.size() > 1) {
            std::list<row_type>::iterator i = m_rows.begin(); i++;
            m_rows.erase(i,m_rows.end());
        }

        m_rows.front().first = r;
    }
    
    SignalQueue::action::row_logical_type SignalQueue::action::get_logical_row() const {
        if(m_rows.size() == 0) 
            return row_logical_type();
        else 
            return m_rows.front().second;
    }

    void SignalQueue::action::set_logical_row(row_logical_type r) {
        if(m_rows.size() == 0) {
            m_rows.push_back(row_type());
        }
        else if(m_rows.size() > 1) {
            std::list<row_type>::iterator i = m_rows.begin(); i++;
            m_rows.erase(i,m_rows.end());
        }

        m_rows.front().second = r;
    }
    
    std::list<SignalQueue::action::row_physical_type> SignalQueue::action::get_physical_rows() const {
        std::list<row_physical_type> ret;
        std::list<row_type>::const_iterator i = m_rows.begin();
        for(;i!=m_rows.end();i++) {
            ret.push_back(i->first);
        }
        return ret;
    }

    std::list<SignalQueue::action::row_logical_type> SignalQueue::action::get_logical_rows() const {
        std::list<row_logical_type> ret;
        std::list<row_type>::const_iterator i = m_rows.begin();
        for(;i!=m_rows.end();i++) {
            ret.push_back(i->second);
        }
        return ret;
    }

    std::list<SignalQueue::action::row_type> SignalQueue::action::get_rows() const { return m_rows; }
    void SignalQueue::action::set_rows(std::list<row_type> r) { m_rows = r; }
    
    SignalChainBase* SignalQueue::action::get_chain() const { return m_chain; }
    void SignalQueue::action::set_chain(SignalChainBase* c) { m_chain = c; }

    std::ostream& operator<<(std::ostream& o, const SignalQueue::action& a) {
        o <<a.get_type()<<std::string(":")<<a.get_row().first <<":"<<"row"<<std::string(":")<<a.get_chain();
        return o;
    }
    
    SignalQueue::SignalQueue() : m_preempt(false) {
        
    }

    void SignalQueue::init() {
        Gtk::Main::timeout.connect(SigC::slot(this,&gtkmail::SignalQueue::gtk_timeout), 10);
    }
    void SignalQueue::add(std::string name, u_int pri, SigC::Slot1<SignalChainBase*,action> slot) {
        std::queue<action> buf;
        m_pri.insert(std::make_pair(pri,name));
        m_slt.insert(std::make_pair(name,slot));
        m_rep.insert(std::make_pair(name,buf));
    }

    bool SignalQueue::preempt(std::string s, std::string t) {
        if(t == ACTION_TYPE_NONE) return true;

        u_int s_pri=0, t_pri=0;
        bool have_s=false, have_t=false;
        std::multimap<u_int,std::string>::iterator i = m_pri.begin();
        for(;i!=m_pri.end();i++) {
            if(i->second == s) {
                have_s = true;
                s_pri = i->first;
            }
            if(i->second == t) {
                have_t = true;
                t_pri = i->first;
            }
        }
        if(!have_s) {
            throw SignalQueue::exception("error finding std::string "+s+" in priority map");
        }
        if(!have_t) {
            throw SignalQueue::exception("error finding std::string "+t+" in priority map");
        }
        return (s_pri > t_pri);
    }

    void SignalQueue::push(std::string type) {
        std::list<SignalQueue::action::row_type> rows;
        m_rep[type].push(action(type,rows));
    }

    void SignalQueue::push(action a) {
        m_rep[a.get_type()].push(a);
    }

    int SignalQueue::gtk_timeout() {
        this->timeout();
        return 1;
    }

    void SignalQueue::timeout() {
        // if we were doing something, but we're done now...
        if(m_cur.get_type() != ACTION_TYPE_NONE && m_cur.get_chain()->is_complete()) {
            delete m_cur.get_chain();
            m_cur = action();
        }
        // if we're doing something, but we're not preempting...
        else if(m_cur.get_type() != ACTION_TYPE_NONE && !m_preempt) {
            return;
        }
        // either we aren't doing anything, or we're willing to preempt it if necesary
        else {
            std::multimap<u_int,std::string>::iterator i = m_pri.begin();
            for(;i!=m_pri.end();i++) {
                std::string type = i->second;
                if(!m_rep[type].empty()) {
                    //cerr << "!m_rep["<<type<<"].empty()"<<endl;
                    if( m_cur.get_type() == ACTION_TYPE_NONE || preempt(type,m_cur.get_type())) {
                        action a = m_rep[type].front();
                        m_rep[type].pop();

                        if(m_cur.get_type() != ACTION_TYPE_NONE) {
                            delete m_cur.get_chain();
                            m_rep[m_cur.get_type()].push(m_cur);
                        }
                        
                        m_cur = a;
                        //cerr << "calling slot for action type "<<m_cur.get_type()<<endl;
                        m_cur.set_chain(m_slt[type].call(m_cur));
                        return;
                    }
                    
                }
            }
        }
    }

    bool SignalQueue::empty(std::string type) {
        return m_rep[type].empty();
    }

    u_int SignalQueue::size(std::string type) {
        return m_rep[type].size();
    }

    void SignalQueue::flush() {
        std::map<std::string, std::queue<action> >::iterator i = m_rep.begin();
        for(;i!=m_rep.end();i++) {
            while(i->second.size() > 0) {
                i->second.pop();
            }
        }
    }


}

