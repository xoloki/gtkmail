/* -*- mode: C++ c-basic-offset: 4  -*-
 * MailBoxPreflet.hh - header file for class Preflet
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class Preflet is a widget which allows 
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
 * Foundation, Inc., 59 Temple Place - Smailboxte 330, Boston, MA  02111-1307, USA.
 * 
 */

#include "MailBoxPreflet.hh"

#include "Config.hh"
#include "Dialog.hh"

#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/scrolledwindow.h>

#include <jlib/util/URL.hh>
#include <jlib/util/util.hh>

#include "xpm/mailbox.xpm"

namespace gtkmail {

    MailBoxDialog::MailBoxDialog(Config::MailBox box)
        : m_box(box)
    {
        Gtk::Table* general_table = Gtk::manage(new Gtk::Table(2, 2));
        m_account_table = Gtk::manage(new Gtk::Table(2, 2));
        Gtk::Table* server_table = Gtk::manage(new Gtk::Table(15, 2));
        Gtk::Label* label;

        m_name = Gtk::manage(new Gtk::Entry());
        m_addr = Gtk::manage(new Gtk::Entry());

        m_user = Gtk::manage(new Gtk::Entry());
        m_pass = Gtk::manage(new Gtk::Entry());

        m_pass->set_visibility(false);

        Gtk::Menu* menu = Gtk::manage(new Gtk::Menu());
        m_proto = Gtk::manage(new Gtk::OptionMenu());
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("simap", sigc::mem_fun(*this, &MailBoxDialog::on_simap)));
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("imap", sigc::mem_fun(*this, &MailBoxDialog::on_imap)));
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("mbox", sigc::mem_fun(*this, &MailBoxDialog::on_mbox)));
        m_proto->set_menu(*menu);

        m_server = Gtk::manage(new Gtk::Entry());
        m_port = Gtk::manage(new Gtk::SpinButton(*Gtk::manage(new Gtk::Adjustment(0, 0, 65535))));
        m_path = Gtk::manage(new Gtk::Entry());
        m_time = Gtk::manage(new Gtk::SpinButton(*Gtk::manage(new Gtk::Adjustment(60000, 1000, 1000000000, 10000))));
        m_smtp = Gtk::manage(new Gtk::Entry());
        m_smtp_port = Gtk::manage(new Gtk::SpinButton(*Gtk::manage(new Gtk::Adjustment(0, 0, 65535))));
        m_smtp_ssl = Gtk::manage(new Gtk::CheckButton("Use SMTP over SSL"));
        m_smtp_starttls = Gtk::manage(new Gtk::CheckButton("Use SMTP STARTTLS"));
        m_smtp_auth = Gtk::manage(new Gtk::CheckButton("Use SMTP Auth"));
        m_smtp_auth_same = Gtk::manage(new Gtk::CheckButton("Use same credentials for SMTP"));
        m_smtp_user = Gtk::manage(new Gtk::Entry());
        m_smtp_pass = Gtk::manage(new Gtk::Entry());
        m_proxy = Gtk::manage(new Gtk::Entry());

        m_idle = Gtk::manage(new Gtk::CheckButton("Use IMAP4 IDLE command"));

        m_name->show();
        m_addr->show();
        m_user->show();
        m_pass->show();

        menu->show_all();
        m_proto->show();
        m_server->show();
        m_port->show();
        m_path->show();
        m_time->show();
        m_smtp->show();
        m_smtp_port->show();
        m_smtp_ssl->show();
        m_smtp_starttls->show();
        m_smtp_auth->show();
        m_proxy->show();

        // general table
        label = Gtk::manage(new Gtk::Label("Name"));
        label->show();
        label->set_alignment(1, 0.5);
        general_table->attach(*label,    0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL);
        general_table->attach(*m_name,   1, 2, 0, 1);

        label = Gtk::manage(new Gtk::Label("Address"));
        label->show();
        label->set_alignment(1, 0.5);
        general_table->attach(*label,    0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL);
        general_table->attach(*m_addr,   1, 2, 1, 2);


        // account table
        label = Gtk::manage(new Gtk::Label("Username"));
        label->show();
        label->set_alignment(1, 0.5);
        m_account_table->attach(*label,    0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL);
        m_account_table->attach(*m_user,   1, 2, 0, 1);

        label = Gtk::manage(new Gtk::Label("Password"));
        label->show();
        label->set_alignment(1, 0.5);
        m_account_table->attach(*label,    0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL);
        m_account_table->attach(*m_pass,   1, 2, 1, 2);

        // server table
        label = Gtk::manage(new Gtk::Label("Protocol"));
        label->show();
        label->set_alignment(1, 0.5);
        server_table->attach(*label,     0, 1, 0, 1, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_proto,   1, 2, 0, 1);

        m_server_label = Gtk::manage(new Gtk::Label("Server"));
        m_server_label->show();
        m_server_label->set_alignment(1, 0.5);
        server_table->attach(*m_server_label, 0, 1, 1, 2, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_server,       1, 2, 1, 2);

        m_port_label = Gtk::manage(new Gtk::Label("Port"));
        m_port_label->show();
        m_port_label->set_alignment(1, 0.5);
        server_table->attach(*m_port_label, 0, 1, 2, 3, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_port,       1, 2, 2, 3);

        server_table->attach(*m_idle,       1, 2, 3, 4);

        label = Gtk::manage(new Gtk::Label("Path"));
        label->show();
        label->set_alignment(1, 0.5);
        server_table->attach(*label,     0, 1, 4, 5, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_path,    1, 2, 4, 5);

        label = Gtk::manage(new Gtk::Label("Poll (ms)"));
        label->show();
        label->set_alignment(1, 0.5);
        server_table->attach(*label,     0, 1, 5, 6, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_time,    1, 2, 5, 6);

        label = Gtk::manage(new Gtk::Label("SMTP Server"));
        label->show();
        label->set_alignment(1, 0.5);
        server_table->attach(*label,     0, 1, 6, 7, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_smtp,    1, 2, 6, 7);

        label = Gtk::manage(new Gtk::Label("SMTP Port"));
        label->show();
        label->set_alignment(1, 0.5);
        server_table->attach(*label,       0, 1, 7, 8, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_smtp_port, 1, 2, 7, 8);

        server_table->attach(*m_smtp_ssl,       1, 2, 8, 9);
        server_table->attach(*m_smtp_starttls,  1, 2, 9, 10);
        server_table->attach(*m_smtp_auth,      1, 2, 10, 11);
        server_table->attach(*m_smtp_auth_same, 1, 2, 11, 12);

        m_smtp_user_label = Gtk::manage(new Gtk::Label("SMTP User"));
        //m_smtp_user_label->show();
        m_smtp_user_label->set_alignment(1, 0.5);
        server_table->attach(*m_smtp_user_label, 0, 1, 12, 13, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_smtp_user,       1, 2, 12, 13);

        m_smtp_pass_label = Gtk::manage(new Gtk::Label("SMTP Pass"));
        //m_smtp_pass_label->show();
        m_smtp_pass_label->set_alignment(1, 0.5);
        server_table->attach(*m_smtp_pass_label, 0, 1, 13, 14, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_smtp_pass,       1, 2, 13, 14);

        m_proxy_label = Gtk::manage(new Gtk::Label("HTTP Proxy"));
        m_proxy_label->show();
        m_proxy_label->set_alignment(1, 0.5);
        server_table->attach(*m_proxy_label, 0, 1, 14, 15, Gtk::SHRINK|Gtk::FILL);
        server_table->attach(*m_proxy,       1, 2, 14, 15);

        general_table->set_spacings(5);
        m_account_table->set_spacings(5);
        server_table->set_spacings(5);
        set_border_width(10);
        get_vbox()->set_spacing(5);
        get_action_area()->set_layout(Gtk::BUTTONBOX_DEFAULT_STYLE);

        get_vbox()->pack_start(*Preflet::create_title("General"), Gtk::PACK_SHRINK);
        get_vbox()->pack_start(*Preflet::indent(general_table), Gtk::PACK_SHRINK);

        m_account_title = Preflet::create_title("Account");
        get_vbox()->pack_start(*m_account_title,                  Gtk::PACK_SHRINK);
        get_vbox()->pack_start(*Preflet::indent(m_account_table), Gtk::PACK_SHRINK);

        get_vbox()->pack_start(*Preflet::create_title("Server"), Gtk::PACK_SHRINK);
        get_vbox()->pack_start(*Preflet::indent(server_table), Gtk::PACK_SHRINK);

        get_vbox()->show();

        general_table->show();
        m_account_table->show();
        server_table->show();

        add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
        add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

        set_default_size(400, 400);

        m_smtp_auth->signal_toggled().connect(sigc::mem_fun(*this, &MailBoxDialog::on_smtp_auth));
        m_smtp_auth_same->signal_toggled().connect(sigc::mem_fun(*this, &MailBoxDialog::on_smtp_auth_same));

        load();
    }
    
    void MailBoxDialog::load() {
        m_name->set_text(m_box.get_name());
        m_addr->set_text(m_box.get_addr());
        m_time->set_value(m_box.get_time());

        m_smtp->set_text(m_box.get_smtp());
        m_smtp_port->set_value(m_box.get_smtp_port());
        m_smtp_ssl->set_active(m_box.get_smtp_ssl());
        m_smtp_starttls->set_active(m_box.get_smtp_starttls());
        m_smtp_auth->set_active(m_box.get_smtp_auth());
        m_smtp_auth_same->set_active(m_box.get_smtp_auth_same());
        m_smtp_user->set_text(m_box.get_smtp_user());
        m_smtp_pass->set_text(m_box.get_smtp_pass());

        m_idle->set_active(m_box.get_idle());

        if(m_box.get_url() != "") {
            try {
                jlib::util::URL url(m_box.get_url());

                m_user->set_text(url.get_user());
                m_pass->set_text(url.get_pass());

                set_proto(url.get_protocol());
                m_server->set_text(url.get_host());
                m_port->set_value(url.get_port_val());
                m_path->set_text(url.get_path());
                m_proxy->set_text(url["proxy"]);

            } catch(Glib::Exception& e) {
                display_exception(std::string("Unable to load MailBox: ") + e.what(), this);
            } catch(std::exception& e) {
                display_exception(std::string("Unable to load MailBox: ") + e.what(), this);
            } catch(...) {
                display_exception("Unable to load MailBox", this);
            }
        } else {
            set_proto("simap");
        }
    }

    void MailBoxDialog::save() {
        try {
            if(get_proto() == "mbox") {
                jlib::util::URL url(get_proto(), "", m_path->get_text());
                m_box.set_url(url);
            } else {
                std::string proxy = m_proxy->get_text();
                std::string qs = (proxy != "" ? ("proxy="+proxy) : "");
                jlib::util::URL url(get_proto(), m_user->get_text(), m_pass->get_text(), 
                                    m_server->get_text(), 
                                    std::to_string(static_cast<int>(m_port->get_value())),
                                    m_path->get_text(), qs);

                m_box.set_url(url);
            }

            m_box.set_name(m_name->get_text());
            m_box.set_addr(m_addr->get_text());
            m_box.set_time(static_cast<int>(m_time->get_value()));

            m_box.set_smtp(m_smtp->get_text());
            m_box.set_smtp_user(m_smtp_user->get_text());
            m_box.set_smtp_pass(m_smtp_pass->get_text());
            m_box.set_smtp_port(m_smtp_port->get_value());
            m_box.set_smtp_ssl(m_smtp_ssl->get_active());
            m_box.set_smtp_starttls(m_smtp_starttls->get_active());
            m_box.set_smtp_auth(m_smtp_auth->get_active());
            m_box.set_smtp_auth_same(m_smtp_auth_same->get_active());

            m_box.set_idle(m_idle->get_active());
            
        } catch(Glib::Exception& e) {
            display_exception(std::string("Unable to save MailBox: ") + e.what(), this);
        } catch(std::exception& e) {
            display_exception(std::string("Unable to save MailBox: ") + e.what(), this);
        } catch(...) {
            display_exception("Unable to save MailBox", this);
        }
    }

    
    std::string MailBoxDialog::get_proto() {
        return m_proto_buf;
    }

    void MailBoxDialog::set_proto(std::string proto) {
        Gtk::Menu* menu = m_proto->get_menu();
        Gtk::Menu_Helpers::MenuList list = menu->items();

        Gtk::Menu_Helpers::MenuList::iterator i;
        int j = 0;
        for(i = list.begin(); i != list.end(); i++, j++) {
            Gtk::Label* label = dynamic_cast<Gtk::Label*>(i->get_child());
            if(label && label->get_text() == proto) {
                menu->select_item(*i);
                m_proto->set_history(j);
                i->activate();
                break;
            }
        }

        if(proto == "simap")
            on_simap();
        else if(proto == "imap")
            on_imap();
        else if(proto == "mbox")
            on_mbox();

        m_proto_buf = proto;
    }
    
    void MailBoxDialog::on_smtp_auth_same() {
        if(m_smtp_auth_same->get_active()) {
            m_smtp_user_label->hide();
            m_smtp_user->hide();

            m_smtp_pass_label->hide();
            m_smtp_pass->hide();
        } else {
            m_smtp_user_label->show();
            m_smtp_user->show();

            m_smtp_pass_label->show();
            m_smtp_pass->show();
        }
    }

    void MailBoxDialog::on_smtp_auth() {
        if(m_smtp_auth->get_active()) {
            m_smtp_auth_same->show();
        } else {
            m_smtp_auth_same->hide();
        }
    }


    void MailBoxDialog::on_simap() {
        m_proto_buf = "simap";
        m_port->set_value(993);

        m_server->show();
        m_port->show();
        m_proxy->show();

        m_server_label->show();
        m_port_label->show();
        m_proxy_label->show();

        m_idle->show();

        m_account_table->show();
        m_account_title->show();
    }

    void MailBoxDialog::on_imap() {
        m_proto_buf = "imap";
        m_port->set_value(143);

        m_server->show();
        m_port->show();
        m_proxy->show();

        m_server_label->show();
        m_port_label->show();
        m_proxy_label->show();

        m_idle->show();

        m_account_table->show();
        m_account_title->show();
    }

    void MailBoxDialog::on_mbox() {
        m_proto_buf = "mbox";

        m_server->hide();
        m_port->hide();
        m_proxy->hide();

        m_server_label->hide();
        m_port_label->hide();
        m_proxy_label->hide();

        m_idle->hide();

        m_account_table->hide();
        m_account_title->hide();
    }


    /*
      Config::MailBox m_box;
      Gtk::OptionMenu* m_proto;
      Gtk::Entry* m_server;
      Gtk::SpinButton* m_port;
      Gtk::Entry* m_user;
      Gtk::Entry* m_pass;
    */
    
    MailBoxPreflet::MailBoxPreflet()
    {
        Gtk::ButtonBox* bbox = Gtk::manage(new Gtk::HButtonBox());

        bbox->set_layout(Gtk::BUTTONBOX_END);

        m_new = Gtk::manage(new Gtk::Button("New"));
        m_edit = Gtk::manage(new Gtk::Button("Edit"));
        m_delete = Gtk::manage(new Gtk::Button("Delete"));

        Gtk::ScrolledWindow* win = manage(new Gtk::ScrolledWindow());
        win->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);

        m_view = Gtk::manage(new Gtk::TreeView());
        m_model = Gtk::TreeStore::create(m_cols);
        m_view->set_model(m_model);
        m_view->set_rules_hint();

        m_view->append_column("Name", m_cols.m_name);
        m_view->append_column("URL", m_cols.m_url);
        
        m_select = m_view->get_selection();
        m_select->set_mode(Gtk::SELECTION_BROWSE);

        Glib::ListHandle<Gtk::TreeViewColumn*> list = m_view->get_columns();
        Glib::ListHandle<Gtk::TreeViewColumn*>::iterator i = list.begin();
        for(; i != list.end(); i++) {
            (*i)->set_alignment(0.5);
        }


        win->add(*m_view);

        bbox->pack_start(*m_new);
        bbox->pack_start(*m_edit);
        bbox->pack_start(*m_delete);

        pack_start(*win);
        pack_end(*bbox, Gtk::PACK_SHRINK);

        m_new->signal_clicked().connect(sigc::mem_fun(*this, &MailBoxPreflet::on_new));
        m_edit->signal_clicked().connect(sigc::mem_fun(*this, &MailBoxPreflet::on_edit));
        m_delete->signal_clicked().connect(sigc::mem_fun(*this, &MailBoxPreflet::on_delete));

        show_all();
    }
    
    void MailBoxPreflet::load() {
        for(Config::iterator i = Config::global.begin(); i != Config::global.end(); i++) {
            Gtk::TreeModel::iterator j = m_model->append();
            Gtk::TreeModel::Row row = *j;

            jlib::util::URL url(i->get_url());
            url.set_pass("");
            
            row[m_cols.m_name] = i->get_name();
            row[m_cols.m_original_name] = i->get_name();
            row[m_cols.m_url] = url();
            row[m_cols.m_box] = i->copy();
            row[m_cols.m_new] = false;
            row[m_cols.m_edited] = false;
        }
    }

    void MailBoxPreflet::save() {
        Gtk::TreeModel::Children kids = m_model->children();
        for(Gtk::TreeModel::Children::iterator i = kids.begin(); i != kids.end(); i++) {
            Gtk::TreeModel::Row row = *i;
            bool is_new = row[m_cols.m_new];
            bool is_edited = row[m_cols.m_edited];
            Config::MailBox box = row[m_cols.m_box];
            std::string name = row[m_cols.m_original_name];
            
            if(is_new) {
                Config::global.push_back(box);
                signal_new.emit(box);
            } 
            else if(is_edited) {
                Config::iterator j = Config::global.find(name);
                if(j != Config::global.end()) {
                    Config::global.replace(j, box);
                } else {
                    std::cerr << "MailBoxPreflet::save: box '" << name << "' cannot be found, adding" << std::endl;
                    Config::global.push_back(box);
                }
                signal_edited.emit(box);
            }
        }

        for(std::list<Config::MailBox>::iterator i = m_deleted.begin(); i != m_deleted.end(); i++) {
            Config::iterator j = Config::global.find(i->get_name());
            signal_deleted.emit(*i);

            if(j != Config::global.end()) {
                Config::global.erase(j);
            }
        }
    }
    
    Gtk::Widget* MailBoxPreflet::get_icon() {
        Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox(false, 0));
        Gtk::Label* label = Gtk::manage(new Gtk::Label("Mailbox"));
        Gtk::Image* image = Gtk::manage(new Gtk::Image(Gdk::Pixbuf::create_from_xpm_data(mailbox_xpm)));

        vbox->pack_start(*image, Gtk::PACK_SHRINK);
        vbox->pack_start(*label, Gtk::PACK_SHRINK);

        return vbox;
    }

    void MailBoxPreflet::on_new() {
        Config::MailBox box; 
        MailBoxDialog dlg(box);
        dlg.set_title("New MailBox");
        dlg.show();
        if(dlg.run() == Gtk::RESPONSE_OK) {
            dlg.save();

            Gtk::TreeModel::iterator i = m_model->append();
            Gtk::TreeModel::Row row = *i;
            jlib::util::URL url;

            try {
                url = box.get_url();
                url.set_pass("");

                row[m_cols.m_name] = box.get_name();
                row[m_cols.m_url] = url();
                row[m_cols.m_box] = box;
                row[m_cols.m_new] = true;

                //Config::global.push_back(box);

            } catch(Glib::Exception& e) {
                display_exception(std::string("Unable to add new MailBox: ") + e.what(), &dlg);
            } catch(std::exception& e) {
                display_exception(std::string("Unable to add new MailBox: ") + e.what(), &dlg);
            } catch(...) {
                display_exception("Unable to add new MailBox", &dlg);
            }
        }

        
    }

    void MailBoxPreflet::on_edit() {
        Gtk::TreeModel::iterator j = m_select->get_selected();
        if(!j) return;
        Gtk::TreeModel::Row row = *j;

        Config::MailBox box = row[m_cols.m_box];
        /*
          std::string name = static_cast<Glib::ustring>(row[m_cols.m_name]);
          Config::iterator i = Config::global.find(name);
          if(i == Config::global.end()) return;
        */

        MailBoxDialog dlg(box);
        dlg.set_title("Edit MailBox");
        dlg.show();
        if(dlg.run() == Gtk::RESPONSE_OK) {
            dlg.save();

            jlib::util::URL url;

            try {
                url = box.get_url();
                url.set_pass("");

                row[m_cols.m_name] = box.get_name();
                row[m_cols.m_url] = url();
                row[m_cols.m_edited] = true;

                save();
                
            } catch(Glib::Exception& e) {
                display_exception(std::string("Unable to add new MailBox: ") + e.what(), &dlg);
            } catch(std::exception& e) {
                display_exception(std::string("Unable to add new MailBox: ") + e.what(), &dlg);
            } catch(...) {
                display_exception("Unable to add new MailBox", &dlg);
            }
        }
    }

    void MailBoxPreflet::on_delete() {
        Gtk::TreeModel::iterator j = m_select->get_selected();
        if(!j) return;
        Gtk::TreeModel::Row row = *j;
        
        Config::MailBox box = row[m_cols.m_box];
        std::string oname = row[m_cols.m_original_name];
        if(oname != "") {
            box.set_name(oname);
        }

        m_deleted.push_back(box);
        m_model->erase(j);
    }

}

