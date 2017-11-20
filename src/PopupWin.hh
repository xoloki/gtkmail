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

#ifndef GTKPOPUPWIN
#define GTKPOPUPWIN

#include <gtk--/window.h>
#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/table.h>
#include <gtk--/dialog.h>
#include <gtk--/button.h>
#include <gtk--/entry.h>

#include <vector>
#include <string>

namespace gtkmail {
    
    class PopupWin : public Gtk::Dialog {
    public:
        PopupWin();
        PopupWin(std::vector<std::string> s, bool hool_ok = true, bool hook_ca = true);
        PopupWin(std::vector<std::string> s, int t);
        virtual ~PopupWin();
        
    private:
        void init(std::vector<std::string> s);
        
        void deleteSelf();
        int deleteSelf2();
        
        std::vector<Gtk::HBox*> boxVec;
        
        Gtk::VBox vbox;
        
        SigC::Connection timeCon;
        
        Gtk::Button okButton;
        Gtk::Button caButton;
        
    };

    class EntryWin : public Gtk::Dialog {
    public:
        EntryWin(std::string label);
        
        Gtk::Entry* get_entry();

        bool is_destroyed();
        void set_destroyed();
    protected:
        Gtk::Entry* m_entry;
        bool m_destroyed;
    };

    // display the exception in a Gnome::Dialog without blocking the UI
    void display_exception(std::string e, Gtk::Widget* widget);

    // display the exception in a Gnome::Dialog; the block parameter tell whether to
    // block the UI or not
    void display_exception_block(std::string e, Gtk::Widget* widget);
    void display_about();
    
}

#endif
