/* -*- mode: C++ c-basic-offset: 4  -*-
 * 
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

#include "Dialog.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

gpgme_error_t gtkmail_passphrase_cb(void *hook, const char *uid_hint, const char *passphrase_info, int prev_was_bad, int fd) {
    if(uid_hint) {
        gtkmail::PassDialog pass(uid_hint);
        if(pass.run() == Gtk::RESPONSE_OK) {
            std::string data = pass.get_text() + "\n";
    
            ::write(fd, data.data(), data.size());
            
            return 0;
        } 
    }

    
    return gpgme_error(GPG_ERR_GENERAL);
}

namespace gtkmail {
    
    EntryDialog::EntryDialog(Glib::ustring message, Glib::ustring initial) 
        : Gtk::MessageDialog(message, false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK_CANCEL, false)
    {
        m_entry.set_text(initial);
        get_vbox()->pack_end(m_entry); 
        m_entry.signal_activate().connect(sigc::bind(sigc::mem_fun(*this,&Gtk::Dialog::response),
                                                     Gtk::RESPONSE_OK));

        show_all();
    }

    Glib::ustring EntryDialog::get_text() {
        return m_entry.get_text();
    }
        
    PassDialog::PassDialog(Glib::ustring message, Glib::ustring initial) 
        : EntryDialog(message, initial)
    {
        m_entry.set_visibility(false);
    }
        
    void display_exception(Glib::ustring error, Gtk::Widget* widget) {
        Gtk::MessageDialog e(error, Gtk::MESSAGE_ERROR);

        if(widget) {
            Gtk::Window* top = dynamic_cast<Gtk::Window*>(widget->get_toplevel());
            if(top) {
                e.set_transient_for(*top);
            }
        }

        e.run();
    }
    
    void display_about() {
        Glib::ustring message = PACKAGE_NAME PACKAGE_VERSION "\n"
            "Copyright (c) 2000 Joe Yandle\n"
            "Joe Yandle <jwy@divisionbyzero.com>\n";

        Gtk::MessageDialog about(message);
        about.run();
    }

    void display_verify_result(gpgme_verify_result_t result) {
        Gtk::MessageType type = Gtk::MESSAGE_ERROR;
        std::string msg;

        gpgme_signature_t sigs = result->signatures;
        while(sigs) {
            if(sigs->summary & GPGME_SIGSUM_VALID) {
                msg = "Valid signature";
                type = Gtk::MESSAGE_INFO;
            } else if(sigs->summary & GPGME_SIGSUM_GREEN) {
                msg = "Good signature";
                type = Gtk::MESSAGE_INFO;
            } else if(sigs->summary & GPGME_SIGSUM_RED) {
                msg = "Bad signature";
            } else {
                std::ostringstream o;
                o << "Unknown signature status: " << std::hex << sigs->summary;
                msg = o.str();
            }

            msg += " for key ID ";
            msg += sigs->fpr;

            sigs = sigs->next;
        }


        Gtk::MessageDialog e(msg, type);
        e.run();
    }
    
}
