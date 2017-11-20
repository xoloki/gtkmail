/* -*- mode: C++ c-basic-offset: 4  -*-
 * HtmlWin.cc
 * Copyright (c) 2000 Joe Yandle <jwy@divisionbyzero.com>
 *
 * Gtktextwin
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

#include "HtmlWin.hh"
#include "Dialog.hh"
#include "MailView.hh"

#include <jlib/net/net.hh>

namespace gtkmail {
    
    HtmlWin::HtmlWin(std::string data, std::string from)
        : m_rep(0)
    {
        m_win = Gtk::manage(new Gtk::ScrolledWindow());
        m_win->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
        m_win->show();

        add(*m_win);

        //set_default_size(400, 400);
        set_source(data, from);
    }
    
    HtmlWin::~HtmlWin() {
        
    }

    void HtmlWin::set_source(std::string data, std::string from) {
        m_source = data;
        try {
            if(m_rep) {
                m_win->remove();
                m_rep = 0;
            }
            m_rep = Gtk::manage(MailView::wrap_html(data, from));
            m_rep->show();
            m_win->add(*m_rep);
            
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }

    }
    
    std::string HtmlWin::get_source() { 
        return m_source; 
    }
    
}
