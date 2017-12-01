/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkMainWin.C - source file for class GtkMainWin
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

#include "MainWin.hh"
#include "MailBox.hh"
#include "WriteWin.hh"
#include "Config.hh"
#include "AddressBook.hh"
#include "ProtocolHandlerMap.hh"
#include "Dialog.hh"
#include "Prefs.hh"
//#include "MailBoxDruid.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <time.h>
#include <signal.h>

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include <fstream>

#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/style.h>
#include <gtkmm/frame.h>
#include <gtkmm/main.h>
#include <gtkmm/stock.h>

#include <jlib/crypt/crypt.hh>

#include <jlib/net/ASMailBox.hh>
#include <jlib/net/ASImapBox.hh>
#include <jlib/net/ASMBox.hh>

#include <jlib/sys/sys.hh>
#include <jlib/sys/Directory.hh>

#include <jlib/util/URL.hh>
#include <jlib/util/util.hh>
#include <jlib/util/Date.hh>


const int WIN_WIDTH=1000;
const int WIN_HEIGHT=500;

namespace gtkmail {
    
    MainWin::MainWin() 
    {
        set_title(PACKAGE_NAME);
        gtkmail::Config::global.load();
        gtkmail::Config::global.load_pixbufs();

        set_default_size(WIN_WIDTH, WIN_HEIGHT);
        set_position(Gtk::WIN_POS_CENTER);
        
        m_vbox = manage(new Gtk::VBox(FALSE, 0));

        //m_menus = manage(new Gtk::MenuBar());
        //m_tools = manage(new Gtk::Toolbar());

        m_boxes = manage(new Gtk::Notebook());
        m_status = manage(new Gtk::Statusbar());

        m_save_popup = 0;

        init_menus();
        init_toolbar();
        init_notebook();
        init_fifo();

        Gtk::Widget* menubar = m_ui->get_widget("/MenuBar");
        
        m_vbox->pack_start(*menubar, Gtk::PACK_SHRINK);
        //m_vbox->pack_start(*m_tools, false, false, 0);
        m_vbox->pack_start(*m_boxes, true, true, 0);
        m_vbox->pack_end(*m_status, false, false, 0);
        add(*m_vbox);

        show_all();
    }

    MainWin::~MainWin() {
        
    }

    void MainWin::init_menus() {
        static const Glib::ustring ui =
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='File'>"
            "      <menuitem action='Reset'/>"
            "      <separator/>"
            "      <menuitem action='Quit'/>"
            "    </menu>"
            "    <menu action='Folder'>"
            "      <menuitem action='NewFolder'/>"
            "      <menuitem action='DeleteFolder'/>"
            "      <menuitem action='Rename'/>"
            "      <menuitem action='Expunge'/>"
            "    </menu>"
            "    <menu action='Message'>"
            "      <menuitem action='Compose'/>"
            "      <menuitem action='ViewMessage'/>"
            "      <menuitem action='Reply'/>"
            "      <menuitem action='ReplyAll'/>"
            "      <menuitem action='Forward'/>"
            "      <menuitem action='ForwardInline'/>"
            "      <menuitem action='DeleteMessage'/>"
            "      <menuitem action='UndeleteMessage'/>"
            "    </menu>"
            "    <menu action='Settings'>"
            "      <menuitem action='Prefs'/>"
            "    </menu>"
            "    <menu action='View'>"
            "      <menuitem action='DisplayImages'/>"
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
        m_actions->add(Gtk::Action::create("Reset", Gtk::Stock::REFRESH, "_Reset Mailbox"), Gtk::AccelKey("<control>Z"), sigc::mem_fun(this, &gtkmail::MainWin::on_reset));
        m_actions->add(Gtk::Action::create("Quit", Gtk::Stock::QUIT), sigc::mem_fun(this, &gtkmail::MainWin::on_quit));

        m_actions->add(Gtk::Action::create("Folder", "F_older"));
        m_actions->add(Gtk::Action::create("NewFolder", Gtk::Stock::NEW, "_New"), Gtk::AccelKey(""), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_folder), "New"));
        m_actions->add(Gtk::Action::create("DeleteFolder", Gtk::Stock::DELETE, "_Delete"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_folder), "Delete"));
        m_actions->add(Gtk::Action::create("Rename", Gtk::Stock::REFRESH, "_Rename"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_folder), "Rename"));
        m_actions->add(Gtk::Action::create("Expunge", Gtk::Stock::REFRESH, "E_xpunge"), Gtk::AccelKey("<control>X"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_folder), "Expunge"));

        m_actions->add(Gtk::Action::create("Message", "_Message"));
        m_actions->add(Gtk::Action::create("Compose", Gtk::Stock::NEW), Gtk::AccelKey("<control>N"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Compose"));
        m_actions->add(Gtk::Action::create("ViewMessage", Gtk::Stock::OPEN, "_View Message"), Gtk::AccelKey("<control>V"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "View"));
        m_actions->add(Gtk::Action::create("Reply", Gtk::Stock::GO_BACK, "_Reply"), Gtk::AccelKey("<control>R"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Reply"));
        m_actions->add(Gtk::Action::create("ReplyAll", Gtk::Stock::MEDIA_REWIND, "R_eply to All"), Gtk::AccelKey("<control>E"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Reply"));
        m_actions->add(Gtk::Action::create("Forward", Gtk::Stock::GO_BACK, "_Forward"), Gtk::AccelKey("<control>F"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Forward"));
        m_actions->add(Gtk::Action::create("ForwardInline", Gtk::Stock::MEDIA_REWIND, "Forward _Inline"), Gtk::AccelKey("<control>I"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Forward Inline"));
        m_actions->add(Gtk::Action::create("DeleteMessage", Gtk::Stock::DELETE), Gtk::AccelKey("Delete"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Delete"));
        m_actions->add(Gtk::Action::create("UndeleteMessage", Gtk::Stock::UNDELETE), Gtk::AccelKey("<shift>Delete"), 
                       sigc::bind(sigc::mem_fun(this, &gtkmail::MainWin::on_message), "Undelete"));

        m_actions->add(Gtk::Action::create("Settings", "_Settings"));
        m_actions->add(Gtk::Action::create("Prefs", Gtk::Stock::PREFERENCES), Gtk::AccelKey("<control>P"), sigc::mem_fun(this, &gtkmail::MainWin::on_preferences));

        m_actions->add(Gtk::Action::create("View", "_View"));
        m_actions->add(Gtk::Action::create("DisplayImages", Gtk::Stock::PREFERENCES, "_Display Images"), Gtk::AccelKey("<control>D"), sigc::mem_fun(this, &gtkmail::MainWin::on_view_display_images));
        
        m_actions->add(Gtk::Action::create("Tab", "_Tab"));
        m_actions->add(Gtk::Action::create("Next", Gtk::Stock::GO_FORWARD, "_Next"), Gtk::AccelKey("<control>Page_Down"), 
                       sigc::mem_fun(this, &gtkmail::MainWin::on_window_next));
        m_actions->add(Gtk::Action::create("Prev", Gtk::Stock::GO_BACK, "_Prev"), Gtk::AccelKey("<control>Page_Up"), 
                       sigc::mem_fun(this, &gtkmail::MainWin::on_window_prev));

        m_actions->add(Gtk::Action::create("Help", "_Help"));
        m_actions->add(Gtk::Action::create("About", Gtk::Stock::ABOUT), Gtk::AccelKey("<control>A"), sigc::ptr_fun(&gtkmail::display_about));

        m_ui->insert_action_group(m_actions);
        m_ui->add_ui_from_string(ui);

        add_accel_group(m_ui->get_accel_group());
    }

    void MainWin::init_toolbar() {
#if 0
        using namespace Gtk::Toolbar_Helpers;

        m_tools->tools().push_back(ButtonElem("Compose",
                                              *manage(new Gtk::Image(Config::global.mail_compose_buf->copy())),
                                              bind(slot(*this,&MainWin::on_message),"Compose"),
                                              "Compose new mail",
                                              "Compose new mail"));

        m_tools->tools().push_back(ButtonElem("View",
                                              *manage(new Gtk::Image(Config::global.mail_view_buf->copy())),
                                              bind(slot(*this,&MainWin::on_message),"View"),
                                              "View mail",
                                              "View mail"));

        m_tools->tools().push_back(ButtonElem("Reply",
                                              *manage(new Gtk::Image(Config::global.mail_reply_buf->copy())),
                                              bind(slot(*this,&MainWin::on_message),"Reply"),
                                              "Reply to mail",
                                              "Reply to mail"));

        m_tools->tools().push_back(ButtonElem("Reply All",
                                              *manage(new Gtk::Image(Config::global.mail_reply_all_buf->copy())),
                                              bind(slot(*this,&MainWin::on_message),"Reply All"),
                                              "Reply to All",
                                              "Reply to All"));

        m_tools->tools().push_back(Space());

        m_tools->tools().push_back(ButtonElem("Save",
                                              *manage(new Gtk::Image(Config::global.mail_save_buf->copy())),
                                              bind(slot(*this,&MainWin::on_message),"Save"),
                                              "Save mail",
                                              "Save mail"));

        m_tools->tools().push_back(ButtonElem("Delete",
                                              *manage(new Gtk::Image(Config::global.mail_delete_buf->copy())),
                                              bind(slot(*this,&MainWin::on_message),"Delete"),
                                              "Delete mail",
                                              "Delete mail"));

        m_tools->set_toolbar_style(Config::global.get_toolbar_style());
#endif
    }

    void MainWin::init_notebook() {
        m_boxes->pages().clear();
        Config::iterator i = Config::global.begin();
        for(;i!=Config::global.end();i++) {
            add_box(*i);
        }

    }

    void MainWin::init_popup() {
        m_save_popup = manage(new Gtk::Menu());
        Gtk::Notebook_Helpers::PageList pages = m_boxes->pages();
        Gtk::Notebook_Helpers::PageList::iterator i = pages.begin();
        std::list<Gtk::Menu*> save_menus;
        for(;i!=pages.end();i++) {
            gtkmail::MailBox* box = dynamic_cast<gtkmail::MailBox*>(i->get_child());
            if(box != 0 && box->get_save_menu() != 0) {
                m_save_popup->items().push_back(Gtk::Menu_Helpers::MenuElem(box->get_name()));
                m_save_popup->items().back().set_submenu(*box->get_save_menu());
            }
            else {
                delete m_save_popup;
                m_save_popup = 0;
                return;
            }
        }
        
    }

    void MainWin::init_fifo() {
        m_mailto_file = std::string(getenv("HOME"))+"/.gtkmail/mailto";

        mkfifo(m_mailto_file.c_str(),0600);

        Glib::signal_io().connect(sigc::mem_fun(*this, &gtkmail::MainWin::pipe_mailto_url), m_mailto_pipe.get_reader(), Glib::IO_IN);

        m_mailto_thread = Glib::Thread::create(sigc::mem_fun(*this, &gtkmail::MainWin::read_mailto_url), false);
    }

    void MainWin::set_passbuf(std::string pass) {
        m_passbuf = pass;
    }

    void MainWin::on_file(std::string s) {
        if(s == "Quit") {
            Gtk::Main::quit();
        }
    }
    
    void MainWin::on_folder(std::string s) {
        if(s == "New") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->folder_new_call();
        }
        else if(s == "Delete") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->folder_delete_call();
        }
        else if(s == "Rename") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->folder_rename_call();
        }
        else if(s == "Expunge") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->folder_expunge_call();
        }
    }
    
    void MainWin::on_message(std::string s) {
        if(s == "Compose") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_compose_call();
        }
        else if(s == "Delete") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_delete_call();
        }
        else if(s == "Undelete") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_undelete_call();
        }
        else if(s == "View") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_view_call();
        }        
        else if(s == "Reply") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_reply_call();
        }
        else if(s == "Forward") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_forward_call();
        }
        else if(s == "Forward Inline") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_forward_call(true);
        }
        else if(s == "Reply All") {
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->message_reply_all_call();
        }
        else if(s == "Save") {
            /*
            if(m_save_popup == 0) {
                init_popup();
            }
            if(m_save_popup != 0) {
                m_save_popup->popup(0,0);
            }
            */
            //static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->popup();
            static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->get_save_menu()->popup(0,0);
        }
    }

    void MainWin::on_fetch(std::string s) {
        if(s == "Fetch") {
            fetch();
        }
        else if(s == "Delete") {
            fetch(true);
        }
    }
    
    void MainWin::on_help(std::string s) {
        if(s == "About") {
            gtkmail::display_about();
        }
    }
    
    bool MainWin::can_fetch() {
        return false;
    }
    
    void MainWin::init_fetches() {
        /*
        std::map< std::string, std::map<std::string,std::string> > config = gtkmail::config::get_fetch_info();
        std::map< std::string, std::map<std::string,std::string> >::iterator i = config.begin();
        for(;i!=config.end();i++) {
            std::map<std::string,std::string>& fetch = i->second;

            std::string key = fetch["name"];
            std::string user = fetch["user"];
            std::string pass = fetch["pass"];
            std::string host = fetch["host"];
            std::string type = fetch["type"];
            std::string port = fetch["port"];

            jlib::util::URL url(type,user,pass,host,port,"inbox","");

            bool rem = true;
            if(jlib::util::imaps(fetch,"delete","false")) {
                rem = false;
            }
            
            if(jlib::util::iequals(type,"imap")) {
                jlib::util::URL url("imap", user,pass, host,"","","");
                m_fetches[new jlib::sys::Mutex()] = new jlib::net::Imap4Fetch(url,false);
            }
            else {
            
            }
            
            i++;
        }
        */

//         if(can_fetch() && !jlib::util::imaps(config,"FETCH_TIMEOUT", "-1")) {
//             if(jlib::util::imaps(config,"FETCH_START", "true")) {
//                 fetch();
//             }
//             Gtk::Main::timeout.connect(bind(slot(*this, &gtkmail::MainWin::fetch), false), 1000*jlib::util::intValue(config["FETCH_TIMEOUT"]));
//         }
    }
    
    int MainWin::fetch(bool del) {
        /*
        std::map<jlib::sys::Mutex*,jlib::net::MailFetch*>::iterator i = m_fetches.begin();
        while(i != m_fetches.end()) {
            if(!i->first->locked()) {
                jlib::net::MailFetch* fetch = i->second;
                jlib::sys::thread(SigC::slot(fetch, static_cast<void (jlib::net::MailFetch::*)(void)>(&jlib::net::MailFetch::retrieve)));
            }
            else {
                std::cout << i->first << " is locked, no reason to try to fetch"<<std::endl;
            }

            i++;
        }
        
        */

        return 0;
    }

    int MainWin::on_status() {
        std::map<Glib::StaticMutex*, jlib::net::MailFetch*>::iterator i = m_fetches.begin();
        std::string buf;
        unsigned int count = 0;

        while(i != m_fetches.end()) {
            if(i->first->trylock()) {
                i->first->unlock();
                count++;
            }
            i++;
        }
        if(buf != "") {
            m_status->pop();
            m_status->push(" Fetching from "+std::to_string(count)+" boxes");
        }
        else {
            m_status->pop();
            m_status->push(" No fetch in progress");
        }

        return 1;
    }

    void MainWin::on_view_display_images() {
        static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->display_images();
    }

    void MainWin::on_preferences() {
        Prefs* prefs = new Prefs();

        prefs->signal_apply.connect(sigc::mem_fun(*this, &MainWin::on_prefs_apply));
        prefs->signal_cancel.connect(sigc::mem_fun(*this, &MainWin::on_prefs_cancel));

        prefs->signal_new.connect(sigc::mem_fun(*this, &MainWin::on_new));
        prefs->signal_edited.connect(sigc::mem_fun(*this, &MainWin::on_edited));
        prefs->signal_deleted.connect(sigc::mem_fun(*this, &MainWin::on_deleted));

        prefs->show_all();
    }

    void MainWin::on_font() {
        try {
            std::string name = Config::global.get_message_font();
            Gtk::FontSelectionDialog font;
            font.set_font_name(name);
            if(font.run() == Gtk::RESPONSE_OK) {
                Config::global.set_message_font(font.get_font_name());
            }
        } catch(std::exception& e) {
            Gtk::MessageDialog d(e.what(), Gtk::MESSAGE_ERROR);
            d.run();
        }
    }


    void MainWin::on_reset() {
        static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child())->reset();
    }

    void MainWin::on_settings(std::string s) {
        /*
        if(getenv("GTKMAIL_MAINWIN_DEBUG"))
            std::cout << "gtkmail::MainWin::on_settings(\""<<s<<"\")"<<std::endl;
        if(s == "New MailBox") {
            MailBoxDruid druid;
            druid.run();
            if(druid.is_finished()) {
                if(!druid.is_filled()) {
                    Gnome::Dialog* d = error("You must fill in all fields");
                    d->run();
                }
                else if(Config::global.find(druid.get_name()) != Config::global.end()) {
                    Gnome::Dialog* d = error("A mailbox with that name alredy exists");
                    d->run();
                }
                else {
                    Config::global.push_back(Config::MailBox(druid.get_url(),
                                                                    druid.get_name(),
                                                                    druid.get_address(),
                                                                    druid.get_time()));
                    Config::global.save();
                    Config::global.load();
                    init_notebook();
                }
            }
        }
        else if(s == "Edit MailBox") {
            gtkmail::MailBox* box = static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child());
            std::string oldname = box->get_name();
            MailBoxDruid druid(box->get_name(),
                               box->get_address(),
                               box->get_url(),
                               std::to_string(box->get_time()));
            druid.select_info_page();
            druid.run();
            if(druid.is_finished()) {
                if(!druid.is_filled()) {
                    Gnome::Dialog* d = error("You must fill in all fields");
                    d->run();
                }
                else if(oldname != druid.get_name() && 
                        Config::global.find(druid.get_name()) != Config::global.end()) {

                    //Gnome::Dialog* d = error("Error: you can't change mailbox '"+oldname+"' to '"+druid.get_name()+"', a mailbox with that name alredy exists");
                    Gnome::Dialog* d = error("A mailbox with that name already exists");
                    d->run();
                }
                else {
                    Config::iterator i = Config::global.find(box->get_name());
                    Config::global.replace(i,Config::MailBox(druid.get_url(),
                                                                    druid.get_name(),
                                                                    druid.get_address(),
                                                                    druid.get_time()));
                    
                    Config::global.save();
                    Config::global.load();
                    init_notebook();
                }
            }
        }
        else if(s == "Delete MailBox") {
            gtkmail::MailBox* box = static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child());
            Config::iterator i = Config::global.find(box->get_name());
            Config::global.erase(i);

            Config::global.save();
            Config::global.load();
            init_notebook();
        }
        */
    }

    void MainWin::read_mailto_url() {
        if(getenv("GTKMAIL_MAINWIN_DEBUG")) {
            std::cout << "gtkmail::MainWin::read_mailto_url()" << std::endl;
        }
        const int N = 1024;
        char buf[N];
        
        int fd = open(m_mailto_file.c_str(), O_RDONLY);
        //std::ifstream fifo;
        while(true) {
            int n = read(fd, buf, N);
            //fifo.read(buf, N);
            //int n = fifo.gcount();

            if(n > 0) {
                std::string url(buf, n);
                if(getenv("GTKMAIL_MAINWIN_DEBUG")) {
                    std::cout << "\twriting '"<<url<<"' to m_mailto_pipe[1]" << std::endl;
                }
                m_mailto_pipe.write(url);
            }
        }

        close(fd);
    }

    bool MainWin::pipe_mailto_url(Glib::IOCondition c) {
        int fd = m_mailto_pipe.get_reader();
        
        if(getenv("GTKMAIL_MAINWIN_DEBUG")) {
            std::cout << "gtkmail::MainWin::pipe_mailto_url()" << std::endl;
        }

        const int N = 1024;
        char buf[N];
        int n = read(fd,buf,N);

        if(n > 0) {
            std::string url(buf,n);
            if(getenv("GTKMAIL_MAINWIN_DEBUG")) {
                std::cout << "\tread '"<<url<<"'" << std::endl;
            }
            
            u_int i = url.find(":");
            if(i != std::string::npos) {
                std::string to = url.substr(i+1);
                std::string qs;
                u_int j = to.find("?");
                u_int k = to.find("&");
                
                if(j != std::string::npos) {
                    qs = to.substr(j+1);
                    to = to.substr(0,j);
                }
                else if(k != std::string::npos) {
                    qs = to.substr(k+1);
                    to = to.substr(0,k);
                }
                
                std::map<std::string,std::string> qsmap = 
                    jlib::util::URL::parse_qs(qs);
                std::string subject;
                std::map<std::string,std::string>::iterator p;
                for(p = qsmap.begin();p!=qsmap.end();p++) {
                    if(jlib::util::lower(p->first) == "subject") {
                        subject = jlib::crypt::uri::decode(p->second);
                    }
                }
                
                WriteWin* write;
                write = new WriteWin(jlib::net::Email(),
                                     static_cast<gtkmail::MailBox*>(m_boxes->get_current()->get_child()));
                write->set_to(to);
                write->set_subject(subject);
            }
        }

        return true;
    }

    void MainWin::on_prefs_apply() {
        Config::global.save();
        AddressBook::global.save();
        ProtocolHandlerMap::global.save();

        //m_tools->set_toolbar_style(Config::global.get_toolbar_style());

        Gtk::Notebook::PageList::iterator i;
        for(i = m_boxes->pages().begin(); i != m_boxes->pages().end(); i++) {
            gtkmail::MailBox* box = dynamic_cast<MailBox*>(i->get_child());
            if(box) {
                box->refresh_viewer();
            }
        }
    }

    void MainWin::on_prefs_cancel() {
        // call load so that any mailbox changes are cleared

        // unnecessary now...
        //Config::global.clear();
        //Config::global.load();

        AddressBook::global.clear();
        AddressBook::global.load();
    }

    void MainWin::on_window_next() {
        if(m_boxes->get_n_pages() > 1) {
            int n = m_boxes->get_current_page();
            m_boxes->next_page();
            if(n == m_boxes->get_current_page()) {
                m_boxes->set_current_page(0);
            }
        }
    }

    void MainWin::on_window_prev() {
        if(m_boxes->get_n_pages() > 1) {
            int n = m_boxes->get_current_page();
            m_boxes->prev_page();
            if(n == m_boxes->get_current_page()) {
                m_boxes->set_current_page( (m_boxes->get_n_pages()-1) );
            }
        }
    }
        

    bool MainWin::on_delete_event(GdkEventAny* event) {
        on_quit();
        return true;
    }

    void MainWin::on_quit() {
        Gtk::Notebook_Helpers::PageList pages = m_boxes->pages();
        Gtk::Notebook_Helpers::PageList::iterator i = pages.begin();
        for(; i != pages.end(); i++) {
            gtkmail::MailBox* box = dynamic_cast<gtkmail::MailBox*>(i->get_child());
            if(box) {
                box->set_pane_pos();
                box->set_col_pos();
            }
        }

        Config::global.save();
        Gtk::Main::quit();
    }

    void MainWin::on_new(Config::MailBox box) {
        add_box(box);
    }

    void MainWin::on_edited(Config::MailBox box) {

    }

    void MainWin::on_deleted(Config::MailBox box) {
        remove_box(box);
    }

    void MainWin::add_box(Config::MailBox box) {
        jlib::util::URL url(box.get_url());
        std::string name = box.get_name();
        std::string addr = box.get_addr();
        int time = box.get_time();
        
        if(time == -1) {
            time = 60000;
        }
        
        std::string type = url.get_protocol();
        
        jlib::net::ASMailBox* asbox = 0;
        if(type.find("imap") != type.npos) {
            if(url.get_pass() == "") {
                std::string label = "Enter password for\n"+url.get_user()+"@"+url.get_host();
                PassDialog pass(label);
                if(pass.run() == Gtk::RESPONSE_OK) {
                    url.set_pass(pass.get_text());
                }
            }
            
            asbox = new jlib::net::ASImapBox(url, box.get_idle());
        }
        else if(type == "mbox") {
            asbox = new jlib::net::ASMBox(url);
        }
        
        if(asbox) {
            gtkmail::MailBox* mbox = manage(new gtkmail::MailBox(asbox,name,time,addr,url()));
            m_boxes->pages().push_back(Gtk::Notebook_Helpers::TabElem(*mbox, name));
        }
        
    }

    void MainWin::remove_box(Config::MailBox box) {
        Gtk::Notebook_Helpers::PageList pages = m_boxes->pages();
        Gtk::Notebook_Helpers::PageList::iterator i = pages.begin();
        for(; i != pages.end(); i++) {
            gtkmail::MailBox* pbox = dynamic_cast<gtkmail::MailBox*>(i->get_child());
            if(pbox->get_name() == box.get_name()) {
                m_boxes->remove_page(*pbox);
            }
        }
    }
}
