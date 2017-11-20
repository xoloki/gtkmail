/* -*- mode: C++ c-basic-offset: 4  -*-
 * UIPreflet.hh - header file for class Preflet
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class Preflet is a widget which allows 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU UI Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU UI Public License for more details.
 * 
 * You should have received a copy of the GNU UI Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "UIPreflet.hh"
#include "Config.hh"

#include <gtkmm/fontselection.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/menu_elems.h>
#include <gtkmm/stock.h>

#include "xpm/ui.xpm"
#include "xpm/general.xpm"

namespace gtkmail {
    
    UIPreflet::UIPreflet() 
        
    {
        //Gtk::Frame* msg_frame = Gtk::manage(new Gtk::Frame("Message Viewer"));

        Gtk::VBox* app_box = Gtk::manage(new Gtk::VBox(false, 2));
        Gtk::VBox* msg_box = Gtk::manage(new Gtk::VBox(false, 2));

        Gtk::HBox* tool_box = Gtk::manage(new Gtk::HBox(false, 10));
        Gtk::HBox* font_box = Gtk::manage(new Gtk::HBox(false, 10));
        Gtk::HBox* char_box = Gtk::manage(new Gtk::HBox(false, 10));
        Gtk::HBox* style_box = Gtk::manage(new Gtk::HBox(false, 10));

        Gtk::Label* tool_label = Gtk::manage(new Gtk::Label("Toolbar style:"));
        Gtk::Label* font_label = Gtk::manage(new Gtk::Label("Font:"));
        Gtk::Label* char_label = Gtk::manage(new Gtk::Label("Default charset:"));
        Gtk::Label* style_label = Gtk::manage(new Gtk::Label("User Style:"));

        Gtk::Menu* tool_menu = Gtk::manage(new Gtk::Menu());

        Gtk::Button* font_button = Gtk::manage(new Gtk::Button(Gtk::Stock::SELECT_FONT));

        m_toolbar_style = Gtk::manage(new Gtk::OptionMenu());
        m_font = Gtk::manage(new Gtk::Entry());
        m_charset = Gtk::manage(new Gtk::Entry());

        tool_menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Text below icons"));
        tool_menu->items().back().property_user_data() = GINT_TO_POINTER(Gtk::TOOLBAR_BOTH);

        tool_menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Text beside icons"));
        tool_menu->items().back().property_user_data() = GINT_TO_POINTER(Gtk::TOOLBAR_BOTH_HORIZ);

        tool_menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Text only"));
        tool_menu->items().back().property_user_data() = GINT_TO_POINTER(Gtk::TOOLBAR_TEXT);

        tool_menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Icons only"));
        tool_menu->items().back().property_user_data() = GINT_TO_POINTER(Gtk::TOOLBAR_ICONS);

        m_toolbar_style->set_menu(*tool_menu);

        m_user_style = Gtk::manage(new Gtk::Entry());

        m_auto_load = Gtk::manage(new Gtk::CheckButton("Auto load images from people in my address book"));

        tool_box->pack_start(*tool_label, Gtk::PACK_SHRINK);
        tool_box->pack_start(*m_toolbar_style, Gtk::PACK_SHRINK);

        font_box->pack_start(*font_label, Gtk::PACK_SHRINK);
        font_box->pack_start(*m_font, Gtk::PACK_SHRINK);
        font_box->pack_start(*font_button, Gtk::PACK_SHRINK);

        char_box->pack_start(*char_label, Gtk::PACK_SHRINK);
        char_box->pack_start(*m_charset, Gtk::PACK_SHRINK);

        style_box->pack_start(*style_label, Gtk::PACK_SHRINK);
        style_box->pack_start(*m_user_style, Gtk::PACK_EXPAND_WIDGET);

        app_box->pack_start(*tool_box, Gtk::PACK_SHRINK);
        msg_box->pack_start(*font_box, Gtk::PACK_SHRINK);
        msg_box->pack_start(*char_box, Gtk::PACK_SHRINK);
        msg_box->pack_start(*style_box, Gtk::PACK_SHRINK);
        msg_box->pack_start(*m_auto_load, Gtk::PACK_SHRINK);

        //app_frame->add(*app_box);
        //msg_frame->add(*msg_box);

        //this->pack_start(*app_frame, Gtk::PACK_SHRINK);
        //this->pack_start(*msg_frame, Gtk::PACK_SHRINK);

        app_box->set_border_width(5);
        msg_box->set_border_width(5);

        append_title("Main Window");
        append_indent(*app_box, 16);

        append_title("Viewer");
        append_indent(*msg_box, 16);

        font_button->signal_clicked().connect(sigc::mem_fun(*this, &UIPreflet::on_font_clicked));
        
    }

    void UIPreflet::load() {
        set_toolbar_style(Config::global.get_toolbar_style());
        m_font->set_text(Config::global.get_message_font());
        m_charset->set_text(Config::global.get_default_charset());

        m_user_style->set_text(Config::global.get_user_style());
        m_auto_load->set_active(Config::global.get_auto_load());
    }

    void UIPreflet::save() {
        Config::global.set_message_font(m_font->get_text());
        Config::global.set_default_charset(m_charset->get_text());
        Config::global.set_toolbar_style(get_toolbar_style());

        Config::global.set_user_style(m_user_style->get_text());
        Config::global.set_auto_load(m_auto_load->get_active());
    }

    Gtk::Widget* UIPreflet::get_icon() {
        Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, 0));
        Gtk::Label* label = Gtk::manage(new Gtk::Label("Interface"));
        Gtk::Image* image = Gtk::manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(general_xpm)));

        vbox->pack_start(*image, Gtk::PACK_SHRINK);
        vbox->pack_start(*label, Gtk::PACK_SHRINK);

        return vbox;
    }
        
    void UIPreflet::on_font_clicked() {
        try {
            std::string name = m_font->get_text();
            Gtk::FontSelectionDialog font;
            font.set_font_name(name);
            if(font.run() == Gtk::RESPONSE_OK) {
                m_font->set_text(font.get_font_name());
            }
        } catch(std::exception& e) {
            Gtk::MessageDialog d(e.what(), Gtk::MESSAGE_ERROR);
            d.run();
        }
    }

    Gtk::ToolbarStyle UIPreflet::get_toolbar_style() {
        void* p = m_toolbar_style->get_menu()->get_active()->property_user_data();
        int style = GPOINTER_TO_INT(p);
        return static_cast<Gtk::ToolbarStyle>(style);
    }

    void UIPreflet::set_toolbar_style(Gtk::ToolbarStyle style) {
        Gtk::Menu* menu = m_toolbar_style->get_menu();
        Gtk::Menu_Helpers::MenuList list = menu->items();

        Gtk::Menu_Helpers::MenuList::iterator i;
        int j = 0;
        for(i = list.begin(); i != list.end(); i++, j++) {
            void* p = i->property_user_data();
            Gtk::ToolbarStyle s = static_cast<Gtk::ToolbarStyle>(GPOINTER_TO_INT(p));

            if(s == style) {
                menu->select_item(*i);
                m_toolbar_style->set_history(j);
                return;
            }
        }
        
        
    }

}
