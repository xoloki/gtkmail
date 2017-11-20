/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkMessageWin.C - source file for class GtkMessageWin
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

#include "MessageWin.hh"
#include "WriteWin.hh"
#include "MailBox.hh"
#include "Config.hh"
#include "Dialog.hh"
#include "MailView.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtkmm/accelgroup.h>
#include <gtkmm/style.h>
#include <gtkmm/frame.h>
#include <gtkmm/main.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/stock.h>

#include <jlib/util/util.hh>
#include <jlib/sys/sys.hh>

const int BSIZE=1024;

const int CONTENT_TYPE_COL=0;
const int ENCODING_COL=1;
const int FILENAME_COL=2;

const int CONTENT_TYPE_WIDTH=200;
const int ENCODING_WIDTH=100;
const int FILENAME_WIDTH=200;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

namespace gtkmail {
    
    /*
      MessageWin::MessageWin(Model* m) - 
    */
    MessageWin::MessageWin(const jlib::net::Email& e,MailBox* box, jlib::net::folder_info_type info)
        : Gtk::Window(),
          m_email(e),
          m_box(box),
          m_info(info)
    {
        Config::iterator i = Config::global.find(box->get_name());
        if(i != Config::global.end()) {
            m_user_addr = i->get_addr();
        }
        
        set_resizable(TRUE);
        set_position(Gtk::WIN_POS_NONE);
        m_vbox = manage(new Gtk::VBox(false,0));
        m_status = manage(new Gtk::Statusbar());

        m_vbox->pack_end(*m_status, false, false, 0);

        //m_user_addr = user_addr;
        set_default_size(WINDOW_WIDTH, WINDOW_HEIGHT);
        
        if(m_email["SUBJECT"] == "") {
            set_title("gtkmail:");
        }
        else {
            set_title(std::string("gtkmail: ")+m_email["SUBJECT"]);
        }
        
        m_from = manage(new Gtk::Combo());
        
        m_date = manage(new Gtk::Entry());
        m_subj = manage(new Gtk::Entry());
        m_to = manage(new Gtk::Entry());
        m_cc = manage(new Gtk::Entry());

        //messageWin.set_policy(GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);  
        
        std::vector<std::string> attachTitles;
        attachTitles.push_back("Content-Type");
        attachTitles.push_back("Encoding");
        attachTitles.push_back("Filename");
        
        Gtk::ScrolledWindow* attach_win = manage(new Gtk::ScrolledWindow());
        attach_win->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);

        m_attach_view = Gtk::manage(new Gtk::TreeView());
        m_attach_view->set_rules_hint();

        //m_attachlist = manage(new Gtk::CTree(attachTitles));
        m_attach_model = Gtk::TreeStore::create(m_attach_cols);
        m_attach_view->set_model(m_attach_model);

        Gtk::TreeView::Column* col = Gtk::manage( new Gtk::TreeView::Column("Content-Type") ); 
        col->pack_start(m_attach_cols.m_icon, false);
        col->pack_start(m_attach_cols.m_content);

        m_attach_view->append_column(*col);
        m_attach_view->append_column("Encoding", m_attach_cols.m_encoding);
        m_attach_view->append_column("File", m_attach_cols.m_file);

        attach_win->add(*m_attach_view);

        /*
        m_attachlist = new Gtk::CTree(attachTitles);
        m_attachlist->set_column_justification(CONTENT_TYPE_COL, GTK_JUSTIFY_LEFT);
        m_attachlist->set_column_justification(ENCODING_COL, GTK_JUSTIFY_LEFT);
        m_attachlist->set_column_justification(FILENAME_COL, GTK_JUSTIFY_LEFT);
        
        m_attachlist->set_column_width(CONTENT_TYPE_COL, CONTENT_TYPE_WIDTH);
        m_attachlist->set_column_width(ENCODING_COL, ENCODING_WIDTH);
        m_attachlist->set_column_width(FILENAME_COL, FILENAME_WIDTH);
        */

        m_menus = Gtk::manage(new Gtk::MenuBar());
        m_tools = Gtk::manage(new Gtk::Toolbar());
                                   
        m_tools->set_toolbar_style(Config::global.get_toolbar_style());
        
        m_notebook = manage(new Gtk::Notebook());
        m_notebook->pages().push_back(Gtk::Notebook_Helpers::TabElem(*attach_win, "Attachments"));

        init_menus();
        Gtk::Widget* menus = m_ui->get_widget("/MenuBar");
        
        m_vbox->pack_start(*menus, false, false, 0);
        m_vbox->pack_start(*m_tools, false, false, 0);
        m_vbox->pack_start(*m_notebook, false, false, 0);

        m_message = manage(new Gtk::TextView());

        Glib::RefPtr<Gtk::TextBuffer::TagTable> tagTable = m_message->get_buffer()->get_tag_table();
        m_text_tag = Gtk::TextBuffer::Tag::create("font-tag"); 
        m_text_tag->property_wrap_mode() = Gtk::WRAP_WORD;
        tagTable->add(m_text_tag);
        Glib::RefPtr<Gtk::RcStyle> rcstyle = m_message->get_modifier_style();
        rcstyle->set_font(Pango::FontDescription(Config::global.get_message_font()));
        m_message->modify_style(rcstyle);

        add(*m_vbox);
    }



    MessageWin::~MessageWin() {
        
    }

    /*
      void addHelpMenu();
    */
    void MessageWin::init_menus() {
        static const Glib::ustring ui =
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='File'>"
            "      <menuitem action='Open'/>"
            "      <menuitem action='Close'/>"
            "    </menu>"
            "    <menu action='Message'>"
            "    </menu>"
            "    <menu action='View'>"
            "    </menu>"
            "    <menu action='Attach'>"
            "    </menu>"
            "    <menu action='Crypt'>"
            "    </menu>"
            "    <menu action='Tab'>"
            "      <menuitem action='Next'/>"
            "      <menuitem action='Prev'/>"
            "    </menu>"
            "    <menu action='Help'>"
            "      <menuitem action='About'/>"
            "    </menu>"
            "  </menubar>"
            "  <toolbar name='ToolBar'>"
            "    <placeholder name='ToolBarItemsPH'>"
            "    </placeholder>"
            "  </toolbar>"
            "</ui>";

        m_ui = Gtk::UIManager::create();
        m_actions = Gtk::ActionGroup::create();

        m_actions->add(Gtk::Action::create("File", "_File"));
        m_actions->add(Gtk::Action::create("Open", Gtk::Stock::JUMP_TO, "_Open URL"), Gtk::AccelKey("<control>U"),
                       sigc::mem_fun(this, &gtkmail::MessageWin::on_open_url));
        m_actions->add(Gtk::Action::create("Close", Gtk::Stock::CLOSE), sigc::mem_fun(*this, &gtkmail::MessageWin::destroy_));

        m_actions->add(Gtk::Action::create("Message", "_Message"));

        m_actions->add(Gtk::Action::create("View", "_View"));

        m_actions->add(Gtk::Action::create("Attach", "_Attach"));

        m_actions->add(Gtk::Action::create("Crypt", "_Crypt"));

        m_actions->add(Gtk::Action::create("Tab", "_Tab"));
        m_actions->add(Gtk::Action::create("Next", Gtk::Stock::GO_FORWARD, "_Next"), Gtk::AccelKey("<control>Page_Down"), 
                       sigc::mem_fun(this, &gtkmail::MessageWin::on_tab_next));
        m_actions->add(Gtk::Action::create("Prev", Gtk::Stock::GO_BACK, "_Prev"), Gtk::AccelKey("<control>Page_Up"), 
                       sigc::mem_fun(this, &gtkmail::MessageWin::on_tab_prev));

        m_actions->add(Gtk::Action::create("Help", Gtk::Stock::HELP));
        m_actions->add(Gtk::Action::create("About", "_About"), sigc::ptr_fun(&gtkmail::display_about));

        m_ui->insert_action_group(m_actions);
        m_ui->add_ui_from_string(ui);

        //add_accel_group(m_ui->get_accel_group());
    }
    
    void MessageWin::append_text(std::string s) {
        Glib::RefPtr<Gtk::TextBuffer> buf = m_message->get_buffer();
        Glib::ustring clean = s;
        Glib::ustring::iterator i;
        while(!clean.validate(i)) clean.replace(i++, i, "*");
        buf->insert(buf->end(), clean);
    }

    void MessageWin::set_text(std::string s) {
        try {
            m_text_tag->property_font() = Config::global.get_message_font();
            Glib::RefPtr<Gtk::TextBuffer> buf = m_message->get_buffer();

            Glib::ustring clean = MailView::get(s);

            buf->erase(buf->begin(), buf->end());
            buf->insert_with_tag(buf->begin(), clean, m_text_tag);

        } catch(Glib::Exception& e) {
            Gtk::MessageDialog d(e.what(), Gtk::MESSAGE_ERROR);
            d.run();
        } catch(std::exception& e) {
            Gtk::MessageDialog d(e.what(), Gtk::MESSAGE_ERROR);
            d.run();
        }
    }

    std::string MessageWin::get_text() {
        Glib::RefPtr<Gtk::TextBuffer> buf = m_message->get_buffer();
        return buf->get_text();
    }

    /*
    void MessageWin::delete_tree(Gtk::CTree_Helpers::RowList& rows) {
        //cout << "entering delete_tree()"<<std::endl;
        
        Gtk::CTree_Helpers::RowIterator i = rows.begin();
        int j=0;
        while(i != rows.end()) {
            std::cout << "On row "<<j<<", get_data() returns "<<(int)i->get_data()<<std::endl;
            if(i->get_data() != 0) {
                //std::cout << "deleting... "<<flush;
                delete reinterpret_cast<jlib::net::Email*>(i->get_data());
                //std::cout << "done"<<std::endl;
            }
            else {
                //std::cout << "not deleting"<<std::endl;
            }
            
            if(! i->is_leaf()) {
                Gtk::CTree_Helpers::RowList subrows = i->subtree();
                delete_tree(subrows);
            }
            
            i++;
            j++;
        }
        //std::cout << "leaving delete_tree()"<<std::endl;
    }
    */
    
    void MessageWin::build_tree(Gtk::TreeModel::Row& row,jlib::net::Email& v) {
        //std::cout << "entering build_tree(); v.size="<<v.size() << std::endl;
        
        //while(i != v.end()) {
        std::string ctype = v.headers()["CONTENT-TYPE"];
        if(ctype == "")
            ctype = "text/plain";
        //std::cout << "ctype='"<<ctype<<"'"<<std::endl;
        std::vector<std::string> ctypeV = jlib::util::tokenize(ctype, ";");
        std::string filename;
        int k=0;
        while((unsigned int)k < ctypeV.size() && !jlib::util::icontains(ctypeV[k], "name=")) k++;
        if((unsigned int)k < ctypeV.size()) {
            filename = ctypeV[k].substr(ctypeV[k].find("=")+1);
            filename = jlib::util::slice(filename, "\"", "\"");
        }
        
        std::string pixfile;
        
        if(ctypeV[0].find("text") != ctypeV[0].npos) {
            row[m_attach_cols.m_icon] = Config::global.text_buf->copy();
        }
        else if(ctypeV[0].find("image") != ctypeV[0].npos) {
            row[m_attach_cols.m_icon] = Config::global.image_buf->copy();
        }
        else if(ctypeV[0].find("audio") != ctypeV[0].npos) {
            row[m_attach_cols.m_icon] = Config::global.audio_buf->copy();
        }
        else if(ctypeV[0].find("multipart") != ctypeV[0].npos) {
            row[m_attach_cols.m_icon] = Config::global.multi_buf->copy();
        }
        else if(ctypeV[0].find("application") != ctypeV[0].npos) {
            row[m_attach_cols.m_icon] = Config::global.app_buf->copy();
        }
        else {
            row[m_attach_cols.m_icon] = Config::global.app_buf->copy();
        }

        row[m_attach_cols.m_content] = ctypeV[0];
        row[m_attach_cols.m_encoding] = v.headers()["CONTENT-TRANSFER-ENCODING"];
        row[m_attach_cols.m_file] = filename;
        row[m_attach_cols.m_data] = v;

        if(v.attach().size() > 0) {
            for(unsigned int i=0;i<v.attach().size();i++) {
                Gtk::TreeModel::Row sub = *(m_attach_model->append(row.children()));
                build_tree(sub,v.attach()[i]);
            }
        }
        
        //std::cout << "leaving build_tree()" << std::endl;
    }
    
    void MessageWin::on_open_url() {
        Glib::RefPtr<Gtk::Clipboard> clip = Gtk::Clipboard::get();
        if(clip->wait_is_text_available()) {
            std::string url = clip->wait_for_text();
            jlib::sys::shell("firefox "+url);
        }
    }
    
    void MessageWin::message_call(std::string s) {
        if(s == "Compose") {
            WriteWin* write; 
            write = new WriteWin(jlib::net::Email(),m_box,m_info,false,false,false,false,false,false);
        }
    }
    
    void MessageWin::help_call(std::string s) {
        if(s == "About") {
            gtkmail::display_about();
        }
    }
    
    void MessageWin::build_tree() {
        m_attach_model->clear();
        Gtk::TreeModel::Row row = *(m_attach_model->append());
        build_tree(row,m_email);
        m_attach_view->expand_row(Gtk::TreePath("0"), true);
    }

    void MessageWin::set_to(std::string s) {
        m_to->set_text(s);
    }

    void MessageWin::set_cc(std::string s) {
        m_cc->set_text(s);
    }

    void MessageWin::set_subject(std::string s) {
        m_subj->set_text(s);
    }

    void MessageWin::on_tab_next() {
        if(m_notebook->get_n_pages() > 1) {
            int n = m_notebook->get_current_page();
            m_notebook->next_page();
            if(n == m_notebook->get_current_page()) {
                m_notebook->set_current_page(0);
            }
        }
    }

    void MessageWin::on_tab_prev() {
        if(m_notebook->get_n_pages() > 1) {
            int n = m_notebook->get_current_page();
            m_notebook->prev_page();
            if(n == m_notebook->get_current_page()) {
                m_notebook->set_current_page( (m_notebook->get_n_pages()-1) );
            }
        }
    }

    
}
