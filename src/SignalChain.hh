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

#ifndef GTKMAIL_SIGNALCHAIN_HH
#define GTKMAIL_SIGNALCHAIN_HH


#include <sigc++/sigc++.h>

#include <glibmm/thread.h>
#include <glibmm/ustring.h>

#include <gtkmm/main.h>

#include <jlib/sys/object.hh>
#include <jlib/sys/pipe.hh>

#include <string>
#include <iostream>

#include <unistd.h>

namespace gtkmail {
    
    
    class SignalChainBase : public jlib::sys::Object {
    public:

        class exception : public std::exception {
        public:
            exception(Glib::ustring msg = "") {
                m_msg = "gtkmail::SignalChain exception"+(msg != "" ? (": "+msg):"");
            }
            virtual ~exception() throw() {}
            virtual const char* what() const throw() { return m_msg.c_str(); }
        protected:
            Glib::ustring m_msg;
        };

        SignalChainBase();
        virtual ~SignalChainBase();

        void start();
        bool input(Glib::IOCondition c);

        bool is_complete() const;
        void kill();

        virtual void exec() = 0;
        virtual void succeed_emit() = 0;

        sigc::signal0<void>                fail;
        sigc::signal1<void,Glib::ustring>    error;
        sigc::signal0<void>                finish;

    protected:        
        void exec_write();

        SigC::Connection m_con;
        Glib::Thread* m_thread;
        bool m_complete;
        bool m_threw;
        std::string m_what;
        jlib::sys::pipe m_pipe;
    };

    template<class R0>
    class SignalChain0 : public SignalChainBase {
    public:
        SignalChain0(sigc::slot0<R0> s0) 
            : SignalChainBase(),
              m_slot0(s0) 
        {}
        
        void exec() {
            m_retval = m_slot0();
        }

        virtual void succeed_emit() {
            succeed.emit(m_retval);
        }
        
        sigc::signal1<void,R0> succeed;
        
    protected:
        sigc::slot0<R0> m_slot0;
        R0 m_retval;
    };

    template<>
    class SignalChain0<void> : public SignalChainBase {
    public:
        SignalChain0(sigc::slot0<void> s0)
            : SignalChainBase(),
              m_slot0(s0)
        {}
        
        void exec() {
            m_slot0();
        }
        
        virtual void succeed_emit() {
            succeed.emit();
        }

        sigc::signal0<void> succeed;

    protected:
        sigc::slot0<void> m_slot0;
    };
    
    template<class R0, class R1>
    class SignalChain1 : public SignalChainBase {
    public:
        SignalChain1(sigc::slot0<R0> s0, sigc::slot1<R1,R0> s1) 
            : SignalChainBase(),
              m_slot0(s0),
              m_slot1(s1)
        {}
        
        void exec() {
            m_retval = m_slot1( m_slot0() );
        }
        
        virtual void succeed_emit() {
            succeed.emit(m_retval);
        }

        sigc::signal1<void,R1> succeed;
        
    protected:
        sigc::slot0<R0> m_slot0;
        sigc::slot1<R1,R0> m_slot1;

        R1 m_retval;
    };

    inline
    SignalChainBase::SignalChainBase() {
        reference();
        m_complete = false;
        m_threw = false;
    }
    
    inline
    SignalChainBase::~SignalChainBase() {
        m_con.disconnect();
        //m_thread.term();
    }
    
    inline
    void SignalChainBase::start() {
        m_con = Glib::signal_io().connect(sigc::mem_fun(*this, 
                                                        &gtkmail::SignalChainBase::input),
                                          m_pipe.get_reader(),
                                          Glib::IO_IN);

        m_thread = Glib::Thread::create(sigc::mem_fun(*this,&gtkmail::SignalChainBase::exec_write), false);
    }
    
    inline
    bool SignalChainBase::input(Glib::IOCondition c) {
        const int N = 16;
        char buf[N];
        int fd = m_pipe.get_reader();

        int n = read(fd,buf,N);
        if(m_complete) {
            if(m_threw) {
                fail.emit();
                error.emit(m_what);
            }
            else {
                succeed_emit();
            }
            finish.emit();
            unreference();
        }
        else {
            throw exception("input(): !m_thread.is_complete()");
        }            
    }
    
    inline
    bool SignalChainBase::is_complete() const { 
        return m_complete; 
    }

    inline
    void SignalChainBase::exec_write() { 
        try {
            exec();
        } catch(std::exception& e) {
            m_threw = true;
            m_what = e.what();
        }

        m_complete = true;
        m_pipe.write(0);
    }
    
}

#endif //GTKMAIL_SIGNAL_SIGNALCHAIN_HH
