/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkReadWin.C - source file for class GtkReadWin
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

#include "ReadWin.hh"
#include "WriteWin.hh"
#include "Dialog.hh"
#include "TextWin.hh"
#include "HtmlWin.hh"
#include "Config.hh"
#include "SignalChain.hh"
#include "MailView.hh"

#include <gtkmm/main.h>
#include <gtkmm/image.h>

//#include <gtk-xmhtml/gtk-xmhtml.h>

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <jlib/util/util.hh>
#include <jlib/sys/sys.hh>
#include <jlib/crypt/crypt.hh>
#include <jlib/net/net.hh>

#include <jlib/sys/tfstream.hh>
#include <jlib/media/Player.hh>
#include <jlib/media/WavFile.hh>
#include <jlib/media/audiofilestream.hh>

const int BSIZE=1024;

const int CONTENT_TYPE_COL=0;
const int ENCODING_COL=1;
const int FILENAME_COL=2;

namespace gtkmail {
    
    ReadWin::ReadWin(const jlib::net::Email& e, MailBox* box, jlib::net::folder_info_type info)

        : MessageWin(e, box, info)
    {
        init_headers();
        init_menus();
        init_toolbar();
        init_message();

        show_all();
    }

    void ReadWin::init_headers() {
        m_headers = manage(new Gtk::Table(5, 2, FALSE));
        
        Gtk::Label* date_label = manage(new Gtk::Label("Date: ",1));
        Gtk::Label* from_label = manage(new Gtk::Label("From: ",1));
        Gtk::Label* to_label = manage(new Gtk::Label("To: ",1));
        Gtk::Label* cc_label = manage(new Gtk::Label("Cc: ",1));
        Gtk::Label* subj_label = manage(new Gtk::Label("Subject: ",1));
        
        m_date->set_text(MailView::get_header(m_email, "date"));
        m_from->get_entry()->set_text(MailView::get_header(m_email, "from"));
        m_to->set_text(MailView::get_header(m_email, "to"));
        m_cc->set_text(MailView::get_header(m_email, "cc"));
        m_subj->set_text(MailView::get_header(m_email, "subject"));
        
        m_date->set_editable(false);
        m_from->get_entry()->set_editable(false);
        m_to->set_editable(false);
        m_cc->set_editable(false);
        m_subj->set_editable(false);
        
        build_tree();
        
        m_headers->attach(*date_label,   0, 1, 0, 1, Gtk::FILL);
        m_headers->attach(*from_label,   0, 1, 1, 2, Gtk::FILL);
        m_headers->attach(*to_label,     0, 1, 2, 3, Gtk::FILL);
        m_headers->attach(*cc_label,     0, 1, 3, 4, Gtk::FILL);
        m_headers->attach(*subj_label,   0, 1, 4, 5, Gtk::FILL);
        
        m_headers->attach(*m_date,   1, 2, 0, 1); 
        m_headers->attach(*m_from,   1, 2, 1, 2); 
        m_headers->attach(*m_to,     1, 2, 2, 3); 
        m_headers->attach(*m_cc,     1, 2, 3, 4); 
        m_headers->attach(*m_subj,   1, 2, 4, 5); 
        
        m_notebook->pages().push_front(Gtk::Notebook_Helpers::TabElem(*m_headers, "Headers"));
    }

    void ReadWin::init_message() {
        m_raw = m_email.raw();
        m_primary = m_email.get_primary_text();
        m_globbed = m_email.get_globbed_text();

        Gtk::ScrolledWindow* message_win = manage(new Gtk::ScrolledWindow());
        message_win->add(*m_message);
        m_vbox->pack_start(*message_win, true, true, 0);

        set_text(m_primary);
        m_message->set_editable(FALSE);
        m_message->grab_focus();
    }

    void ReadWin::init_menus() {
        static const Glib::ustring ui =
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='Message'>"
            "      <menuitem action='Reply'/>"
            "      <menuitem action='ReplyAll'/>"
            "      <menuitem action='Forward'/>"
            "      <menuitem action='ForwardSpam'/>"
            "    </menu>"
            "    <menu action='View'>"
            "      <menuitem action='Normal'/>"
            "      <menuitem action='Globbed'/>"
            "      <menuitem action='Raw'/>"
            "    </menu>"
            "    <menu action='Attach'>"
            "      <menuitem action='ViewRaw'/>"
            "      <menuitem action='ViewText'/>"
            "      <menuitem action='ViewHtml'/>"
            "      <menuitem action='ViewImage'/>"
            "      <menuitem action='Export'/>"
            "      <menuitem action='ExportRaw'/>"
            "      <menuitem action='PlayAudio'/>"
            "    </menu>"
            "    <menu action='Crypt'>"
            "      <menuitem action='Verify'/>"
            "      <menuitem action='Decrypt'/>"
            "      <menuitem action='VerifyDecrypt'/>"
            "    </menu>"
            "  </menubar>"
            "</ui>";

        Glib::RefPtr<Gtk::ActionGroup> m_read_actions = Gtk::ActionGroup::create();
        m_read_actions->add(Gtk::Action::create("Reply", "_Reply"), Gtk::AccelKey("<control>R"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::message_call), std::string("Reply")));
        m_read_actions->add(Gtk::Action::create("ReplyAll", "Reply _All"), Gtk::AccelKey("<control>E"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::message_call), std::string("ReplyAll")));
        m_read_actions->add(Gtk::Action::create("Forward", "_Forward"), Gtk::AccelKey("<control>F"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::message_call), std::string("Forward")));
        m_read_actions->add(Gtk::Action::create("ForwardSpam", "Forward _Spam"), Gtk::AccelKey("<control>S"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::message_call), std::string("Forward")));

        m_read_actions->add(Gtk::Action::create("Normal", "_Normal"), Gtk::AccelKey("<control>N"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::view_call), std::string("Normal")));
        m_read_actions->add(Gtk::Action::create("Globbed", "_Globbed"), Gtk::AccelKey("<control>G"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::view_call), std::string("Globbed")));
        m_read_actions->add(Gtk::Action::create("Raw", "_Raw"), Gtk::AccelKey("<control>V"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::view_call), std::string("Raw")));

        m_read_actions->add(Gtk::Action::create("Export", "_Export"), Gtk::AccelKey("<control>E"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("Export")));
        m_read_actions->add(Gtk::Action::create("ExportRaw", "E_xport Raw"), Gtk::AccelKey("<control>U"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("ExportRaw")));
        m_read_actions->add(Gtk::Action::create("ViewImage", "View Image"), Gtk::AccelKey("<control>M"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("ViewImage")));
        m_read_actions->add(Gtk::Action::create("PlayAudio", "_Play Audio"), Gtk::AccelKey("<control>O"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("PlayAudio")));
        m_read_actions->add(Gtk::Action::create("ViewText", "View _Text"), Gtk::AccelKey("<control>T"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("ViewText")));
        m_read_actions->add(Gtk::Action::create("ViewHtml", "View _Html"), Gtk::AccelKey("<control>L"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("ViewHtml")));
        m_read_actions->add(Gtk::Action::create("ViewRaw", "View _Raw"), Gtk::AccelKey("<control>A"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_call), std::string("ViewRaw")));

        m_read_actions->add(Gtk::Action::create("Verify", "_Verify"), Gtk::AccelKey("<control>Y"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::crypt_call), std::string("Verify")));
        m_read_actions->add(Gtk::Action::create("Decrypt", "_Decrypt"), Gtk::AccelKey("<control>D"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::crypt_call), std::string("Decrypt")));
        m_read_actions->add(Gtk::Action::create("VerifyDecrypt", "Verify/Decrypt"), Gtk::AccelKey("<control>P"), 
                       sigc::bind(sigc::mem_fun(*this, &ReadWin::crypt_call), std::string("VerifyDecrypt")));

        m_ui->insert_action_group(m_read_actions);
        m_ui->add_ui_from_string(ui);

        add_accel_group(m_ui->get_accel_group());
    }

    void ReadWin::init_toolbar() {
#if 0
        using namespace Gtk::Toolbar_Helpers;

        m_tools->tools().push_back(ButtonElem("Compose",
                                              *manage(new Gtk::Image(Config::global.mail_compose_buf->copy())),
                                              bind(sigc::mem_fun(*this,&ReadWin::message_call),
                                                   std::string("Compose")),
                                              "Compose",
                                              "Compose new mail"));

        m_tools->tools().push_back(Space());

        m_tools->tools().push_back(ButtonElem("Reply",
                                              *manage(new Gtk::Image(Config::global.mail_reply_buf->copy())),
                                              bind(sigc::mem_fun(*this,&ReadWin::message_call),
                                                   std::string("Reply")),
                                              "Reply",
                                              "Reply to mail"));

        m_tools->tools().push_back(ButtonElem("Reply All",
                                              *manage(new Gtk::Image(Config::global.mail_reply_all_buf->copy())),
                                              bind(sigc::mem_fun(*this,&ReadWin::message_call),
                                                   std::string("Reply All")),
                                              "Reply to All",
                                              "Reply to All"));

        m_tools->tools().push_back(Space());

        m_tools->tools().push_back(ButtonElem("Forward",
                                              *manage(new Gtk::Image(Config::global.mail_forward_buf->copy())),
                                              bind(sigc::mem_fun(*this,&ReadWin::message_call),
                                                   std::string("Forward")),
                                              "Forward",
                                              "Forward to mail"));

#endif
    }

    void ReadWin::message_call(std::string s) {
        if(s == "Reply") {
            message_reply_call();
        }
        else if(s == "ReplyAll") {
            message_reply_all_call();
        }
        else if(s == "Forward") {
            message_forward_call();
        }
        else if(s == "Raw") {
            message_raw_call();
        }
        else if(s == "ForwardSpam") {
            message_forward_spam_call();
        }
        else {
            MessageWin::message_call(s);
        }
    }
    
    void ReadWin::attach_call(std::string s) {
        if(s == "ViewText") {
            attach_view_text_call();
        }
        else if(s == "ViewHtml") {
            attach_view_html_call();
        }
        else if(s == "ViewImage") {
            attach_view_image_call();
        }
        else if(s == "ViewRaw") {
            attach_view_raw_call();
        }
        else if(s == "Export") {
            attach_export_call();
        }
        else if(s == "ExportRaw") {
            attach_export_call(false);
        }
        else if(s == "PlayAudio") {
            attach_play_audio_call();
        }
    }

    void ReadWin::crypt_call(std::string s) {
        using namespace jlib::crypt;
        try {
            gpgme_verify_result_t result;
            gpg::ctx ctx;
            static bool init = false;
            
            if(!init) {
                gpg::init(GPGME_PROTOCOL_OpenPGP);
                init = true;
            }

            ctx.set_armor();
            ctx.set_passphrase_cb(gtkmail_passphrase_cb);
            ctx.set_keylist_mode(1);

            if(s == "Verify") {
                if(m_email.is("multipart/signed")) {
                    std::string data;
                    std::string sign;

                    jlib::net::Email::iterator i = m_email.begin();
                    for(;i!=m_email.end();i++) {
                        if(i->is("application/pgp-signature")) {
                            sign = i->data();
                        }
                        else {
                            data = i->raw();
                        }
                    }

                    gpg::data::ptr sig = gpg::data::create(sign);
                    gpg::data::ptr plain = gpg::data::create(data);

                    result = ctx.op_verify(sig, plain);
                } 
                else {
                    gpg::data::ptr sig = gpg::data::create(this->get_text());
                    gpg::data::ptr plain = gpg::data::create();

                    result = ctx.op_verify(sig, plain);
                    //set_text(plain->read());
                }
                display_verify_result(result);
            } 
            else if(s == "Decrypt" || s == "VerifyDecrypt") {
                gpg::data::ptr plain = gpg::data::create();
                bool show = false;
		std::string data;

		if(util::begins(m_email["Content-Type"], "multipart/encrypted")) {
		    // check for two attachments: application/pgp-encrypted then octet data
		    if(m_email.attach().size() == 2 && m_email[0]["Content-Type"] == "application/pgp-encrypted" && m_email[1]["Content-Type"] == "application/octet-stream") {
		        data = m_email[1].data();
		    } else {
		        data = m_email.data();
		    }
		} else {
		    try {
			net::Email::reference r = m_email.grep("BEGIN PGP MESSAGE");
		        data = r.data();
		    } catch(net::Email::exception&) {
		        data = m_email.data();
		    }
		}
                gpg::data::ptr cipher = gpg::data::create(data);
		
                if(s == "Decrypt") {
                    ctx.op_decrypt_verify(cipher, plain);
                    set_text(plain->read());
                } else {
                    result = ctx.op_decrypt_verify(cipher, plain);
                    //set_text(plain->read());

                    display_verify_result(result);
                }
            }


        }
        catch(std::exception& e) {
            display_exception(e.what(), this);
        }
    }
    
    void ReadWin::view_call(std::string s) {
        if(s == "Normal") {
            set_text(m_primary);
        }
        else if(s == "Raw") {
            set_text(m_raw);
        }
        else if(s == "Globbed") {
            set_text(m_globbed);
        }
    }
    
    
    /*
      void messageReplay_call();
    */
    void ReadWin::message_reply_call() {
        WriteWin* temp;
        temp = new WriteWin(m_email, m_box, m_info, true, false, true);//,false,false,false);
    }
    
    void ReadWin::message_reply_all_call() {
        WriteWin* temp;
        temp = new WriteWin(m_email, m_box, m_info, true, true, true);//, false, false, false);
    }
    
    void ReadWin::message_forward_call() {
        WriteWin* temp;
        temp = new WriteWin(m_email, m_box, m_info, false, false, false, true, true, false);
    }
    
    void ReadWin::message_forward_spam_call() {
        WriteWin* temp;
        temp = new WriteWin(m_email, m_box, m_info, false, false, false, true, false, true);
    }

    void ReadWin::message_raw_call() {
        
    }
    
    void ReadWin::attach_view_raw_call() {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                jlib::net::Email e = (*i)[m_attach_cols.m_data];
                TextWin* text_win; text_win = new TextWin(e.raw());
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
    }

    void ReadWin::attach_view_text_call() {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                jlib::net::Email e = (*i)[m_attach_cols.m_data];
                TextWin* text_win; text_win = new TextWin(e.data());
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
    }
    
    void ReadWin::attach_view_html_call() {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                jlib::net::Email e = (*i)[m_attach_cols.m_data];
                HtmlWin* html_win; html_win = new HtmlWin(e.data(), m_email["from"]);
                html_win->show();
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
    }
    
    void ReadWin::attach_view_image_call() {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                jlib::net::Email e = (*i)[m_attach_cols.m_data];
                Glib::ustring file = (*i)[m_attach_cols.m_file];
                jlib::sys::tfstream fs;
                fs << e.data();
                fs.close();

                Gtk::Image* image = Gtk::manage(new Gtk::Image(fs.get_path()));
                Glib::RefPtr<Gdk::Pixbuf> pixbuf = image->get_pixbuf();
                int width = pixbuf->get_width() > 1024 ? 1024 : pixbuf->get_width();
                int height = pixbuf->get_height() > 1024 ? 1024 : pixbuf->get_height();

                Gtk::ScrolledWindow* swin = Gtk::manage(new Gtk::ScrolledWindow());
                swin->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
                swin->add(*image);

                Gtk::Window* win = new Gtk::Window();
                win->add(*swin);
                win->set_title("gtkmail: " + file);
                win->set_default_size(width, height);
                win->show_all();
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
    }
    

    
    void ReadWin::attach_play_audio_call() {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                jlib::net::Email e = (*i)[m_attach_cols.m_data];
                std::string data = e.data();
                
                SignalChain0<void>* chain = 
                    new SignalChain0<void>(bind(sigc::mem_fun(*this,
                                                     &gtkmail::ReadWin::attach_play_audio_exec),
                                                data));
                chain->error.connect(sigc::bind(sigc::ptr_fun(&display_exception),this));
                chain->start();
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
    }
    
    void ReadWin::attach_play_audio_exec(std::string data) {
        sys::tfstream fs;
        fs << data;
        fs.close();
        
        media::WavFile wav(fs.get_path());
        media::audiofilestream stream(&wav);
        media::Player player(&stream);
        player.play();
        while(!player.is_playing());
        while(player.is_playing());
    }
    
    void ReadWin::attach_export_call(bool decode) {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                Glib::ustring file = (*i)[m_attach_cols.m_file];
                
                Gtk::FileSelection* fileSel = manage(new Gtk::FileSelection("Export Attachment"));
                fileSel->set_position(Gtk::WIN_POS_CENTER);
                fileSel->set_filename(file);
                fileSel->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &ReadWin::attach_export_exec), fileSel, decode));
                fileSel->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&operator delete), fileSel));
                fileSel->get_cancel_button()->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&operator delete), fileSel));
                fileSel->show_all();    
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
    }
    
    
    /*
      void attachExport_exec();
    */
    void ReadWin::attach_export_exec(Gtk::FileSelection* fileSel, bool decode) {
        Glib::RefPtr<Gtk::TreeSelection> selection = m_attach_view->get_selection();
        Gtk::TreeModel::iterator i = selection->get_selected();
        try {
            if(i) {
                jlib::net::Email e = (*i)[m_attach_cols.m_data];
                std::string path = fileSel->get_filename();
                std::ofstream fs(path.c_str(), std::ios_base::out | std::ios_base::trunc);
                if(decode)
                    fs << e.data();
                else
                    fs << e.raw();
                fs.close();
            }
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
        
    }

    
}
