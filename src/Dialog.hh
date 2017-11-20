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

#ifndef GTKMAIL_DIALOG_HH
#define GTKMAIL_DIALOG_HH

#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/button.h>
#include <gtkmm/entry.h>

#include <vector>
#include <string>

#include <gpgme.h>

extern "C" {
    gpgme_error_t gtkmail_passphrase_cb(void *hook, const char *uid_hint, const char *passphrase_info, int prev_was_bad, int fd);
}

namespace gtkmail {
    
    class EntryDialog : public Gtk::MessageDialog {
    public:
        EntryDialog(Glib::ustring label, Glib::ustring initial="");
        
        Glib::ustring get_text();
    protected:
        Gtk::Entry m_entry;
    };

    class PassDialog : public EntryDialog {
    public:
        PassDialog(Glib::ustring label, Glib::ustring initial="");
    };

    // display the exception in a Gtk::MessageDialog without blocking the UI
    void display_exception(Glib::ustring error, Gtk::Widget* widget = 0);

    void display_about();

    void display_verify_result(gpgme_verify_result_t stat);
}

#endif //GTKMAIL_DIALOG_HH
