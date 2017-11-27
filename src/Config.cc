/* -*- mode:C++ c-basic-offset:4  -*-
 * Config.hh - defines interfaces to config stuff
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

#include "Config.hh"
#include "Dialog.hh"
#include "MailBox.hh"

#include "xpm/mail-compose.xpm"
#include "xpm/mail-delete.xpm"
#include "xpm/mail-forward.xpm"
#include "xpm/mail-receive.xpm"
#include "xpm/mail-reply.xpm"
#include "xpm/mail-reply-all.xpm"
#include "xpm/mail-save.xpm"
#include "xpm/mail-send.xpm"
#include "xpm/mail-view.xpm"

#include "xpm/mail-new.xpm"
#include "xpm/mail-read.xpm"
#include "xpm/mail-replied.xpm"

#include "xpm/attachment.xpm"
#include "xpm/empty.xpm"

#include "xpm/application.xpm"
#include "xpm/audio.xpm"
#include "xpm/image.xpm"
#include "xpm/mail.xpm"
#include "xpm/multipart.xpm"
#include "xpm/text.xpm"

#include "xpm/save.xpm"
#include "xpm/cancel.xpm"

#include "xpm/dir-open.xpm"
#include "xpm/dir-close.xpm"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fstream>

#include <jlib/util/util.hh>
#include <jlib/sys/sys.hh>

#include <gtkmm/main.h>

const std::string XML_HEADER = "<?xml version=\"1.0\"?>";

const std::string GTKMAIL_CONFIG_ROOT_NODENAME = PACKAGE_NAME;
const std::string GTKMAIL_CONFIG_MAILBOX_NODENAME = "mailbox";
const std::string GTKMAIL_CONFIG_MAILBOX_NAME_ATTR = "name";
const std::string GTKMAIL_CONFIG_MAILBOX_TIME_ATTR = "time";
const std::string GTKMAIL_CONFIG_MAILBOX_ADDR_ATTR = "addr";

using namespace jlib::util;

namespace gtkmail {

    Config Config::global;

    Config::MailBox::MailBox() 
    {
        static int n = 0;
        std::ostringstream os; os << "mailbox" << n++;

        m_node = xml::node::create("mailbox"); 
        m_node->set_attribute("name", os.str());

        set_smtp_port(25);
        set_smtp_tls(false);
        set_smtp_auth(false);
        set_smtp_auth_same(true);

        set_idle(false);
    }

    Config::MailBox::MailBox(std::string url,std::string name,std::string addr,std::string time) 
    {
        m_node = xml::node::create("mailbox");

        set_url(url);
        set_name(name);
        set_addr(addr);
        set_time(int_value(time));

        set_smtp_port(25);
        set_smtp_tls(false);
        set_smtp_auth(false);
        set_smtp_auth_same(true);

        set_idle(false);
    }
    
    Config::MailBox::MailBox(xml::node::ptr node)
    {
        set_node(node);
    }

    std::string Config::MailBox::get_url() const {
        if(m_node->get_list().empty()) {
            xml::node::ptr cdata = xml::cdata::create();
            m_node->add(cdata);
        } 

        xml::node::ptr cdatanode = m_node->get_list().front();
        return cdatanode->get_cdata();
    }

    std::string Config::MailBox::get_name() const {
        return m_node->get_attribute("name");
    }

    std::string Config::MailBox::get_addr() const {
        return m_node->get_attribute("addr");
    }

    std::string Config::MailBox::get_smtp() const {
        return m_node->get_attribute("smtp");
    }

    unsigned int Config::MailBox::get_smtp_port() const {
        return jlib::util::int_value(m_node->get_attribute("smtp_port"));
    }

    bool Config::MailBox::get_smtp_tls() const {
        return m_node->get_attribute("smtp_tls") == "true";
    }

    bool Config::MailBox::get_smtp_auth() const {
        return m_node->get_attribute("smtp_auth") == "true";
    }

    bool Config::MailBox::get_smtp_auth_same() const {
        return m_node->get_attribute("smtp_auth_same") == "true";
    }

    std::string Config::MailBox::get_smtp_user() const {
        return m_node->get_attribute("smtp_user");
    }

    std::string Config::MailBox::get_smtp_pass() const {
        return m_node->get_attribute("smtp_pass");
    }

    int Config::MailBox::get_time() const {
        return int_value(m_node->get_attribute("time"));
    }
    
    int Config::MailBox::get_folder_pos() const {
        if(m_node->get_attribute("folder_pos") == "") {
            m_node->set_attribute("folder_pos", "200");
        } 
        return int_value(m_node->get_attribute("folder_pos"));
    }

    int Config::MailBox::get_message_pos() const {
        if(m_node->get_attribute("message_pos") == "") {
            m_node->set_attribute("message_pos", "200");
        } 
        return int_value(m_node->get_attribute("message_pos"));
    }

    std::vector<Config::MailBox::Column> Config::MailBox::get_message_cols() const {
        std::vector<Column> cols;
        
        if(m_node->get_attribute("message_cols") == "") {
            m_node->set_attribute("message_cols", "READ:-1,Subject:-1,From:-1,Size:-1,ATTACH:-1,Date:-1");
        } 

        std::string message_cols = m_node->get_attribute("message_cols");
        auto tcols = jlib::util::tokenize(message_cols, ",");
        for(auto tc : tcols) {
            auto tcs = jlib::util::tokenize(tc, ":");
            std::string title = tcs[0];
            int width = jlib::util::int_value(tcs[1]);

            cols.push_back(Column(title, width));
        }

        return cols;
    }
    
    bool Config::MailBox::get_idle() const {
        return m_node->get_attribute("idle") == "true";
    }

    void Config::MailBox::set_idle(bool b) {
        m_node->set_attribute("idle", b ? "true" : "false");
    }

    void Config::MailBox::set_url(std::string url) {
        if(m_node->get_list().empty()) {
            xml::node::ptr cdata = xml::cdata::create(url);
            m_node->add(cdata);
        } else {
            xml::node::ptr cdatanode = m_node->get_list().front();
            cdatanode->set_cdata(url);
        }
    }

    void Config::MailBox::set_name(std::string s) {
        m_node->set_attribute("name", s);
    }

    void Config::MailBox::set_addr(std::string s) {
        m_node->set_attribute("addr", s);
    }

    void Config::MailBox::set_smtp(std::string s) {
        m_node->set_attribute("smtp", s);
    }

    void Config::MailBox::set_smtp_user(std::string s) {
        m_node->set_attribute("smtp_user", s);
    }

    void Config::MailBox::set_smtp_pass(std::string s) {
        m_node->set_attribute("smtp_pass", s);
    }

    void Config::MailBox::set_smtp_port(unsigned int port) {
        m_node->set_attribute("smtp_port", jlib::util::string_value(port));
    }

    void Config::MailBox::set_smtp_tls(bool b) {
        m_node->set_attribute("smtp_tls", b ? "true" : "false");
    }

    void Config::MailBox::set_smtp_auth(bool b) {
        m_node->set_attribute("smtp_auth", b ? "true" : "false");
    }

    void Config::MailBox::set_smtp_auth_same(bool b) {
        m_node->set_attribute("smtp_auth_same", b ? "true" : "false");
    }

    void Config::MailBox::set_time(int t) {
        m_node->set_attribute("time", string_value(t));
    }

    void Config::MailBox::set_folder_pos(int t) {
        m_node->set_attribute("folder_pos", string_value(t));
    }

    void Config::MailBox::set_message_pos(int t) {
        m_node->set_attribute("message_pos", string_value(t));
    }

    void Config::MailBox::set_message_cols(const std::vector<Config::MailBox::Column>& cols) {
        std::ostringstream o;
        for(int i = 0; i < cols.size(); i++) {
            o << cols[i].title << ":" << cols[i].width;
            if((i+1) < cols.size())
                o << ",";
        }
        m_node->set_attribute("message_cols", o.str());
    }
    
    xml::node::ptr Config::MailBox::get_node() {
        return m_node;
    }

    const xml::node::ptr Config::MailBox::get_node() const {
        return m_node;
    }

    void Config::MailBox::set_node(xml::node::ptr node) {
        m_node = node;
    }

    void Config::MailBox::copy(const Config::MailBox& box) {
        set_url(box.get_url());
        set_name(box.get_name());
        set_addr(box.get_addr());
        set_smtp(box.get_smtp());
        set_smtp_port(box.get_smtp_port());
        set_smtp_user(box.get_smtp_user());
        set_smtp_pass(box.get_smtp_pass());
        set_smtp_tls(box.get_smtp_tls());
        set_smtp_auth(box.get_smtp_auth());
        set_smtp_auth_same(box.get_smtp_auth_same());
        set_time(box.get_time());
        set_folder_pos(box.get_folder_pos());
        set_message_pos(box.get_message_pos());
        set_idle(box.get_idle());
    }

    Config::MailBox Config::MailBox::copy() {
        Config::MailBox box;
        box.copy(*this);
        return box;
    }

    
    Config::Config() {
        using namespace jlib::crypt;

        if(!getenv("HOME") != 0) {
            std::cerr << "Unable to get home directory, exiting" << std::endl;
            exit(1);
        }

        std::string dir = get_gtkmail_dir();
        int e = mkdir(dir.c_str(), 0700);
        if(e == -1) {
            switch(errno) {
            case EEXIST:
                break;
            default:
                std::cerr << "Unable to create home directory: " << strerror(errno) << std::endl;
                exit(1);
            }
        }

        m_file = dir + "/config.xml";
        m_gpg_id_file = dir + "/id";
    }

    void Config::load_pixbufs() {
        cancel_buf =  Gdk::Pixbuf::create_from_xpm_data(cancel_xpm);
        save_buf =  Gdk::Pixbuf::create_from_xpm_data(save_xpm);
        
        mail_view_buf = Gdk::Pixbuf::create_from_xpm_data(mail_view_xpm);
        mail_delete_buf = Gdk::Pixbuf::create_from_xpm_data(mail_delete_xpm);

        mail_compose_buf = Gdk::Pixbuf::create_from_xpm_data(mail_compose_xpm);
        mail_reply_buf = Gdk::Pixbuf::create_from_xpm_data(mail_reply_xpm);
        mail_reply_all_buf = Gdk::Pixbuf::create_from_xpm_data(mail_reply_all_xpm);
        mail_forward_buf = Gdk::Pixbuf::create_from_xpm_data(mail_forward_xpm);
        mail_save_buf = Gdk::Pixbuf::create_from_xpm_data(mail_save_xpm);
        mail_send_buf = Gdk::Pixbuf::create_from_xpm_data(mail_send_xpm);
        mail_receive_buf = Gdk::Pixbuf::create_from_xpm_data(mail_receive_xpm);

        mail_new_buf = Gdk::Pixbuf::create_from_xpm_data(mail_new_xpm);
        mail_read_buf = Gdk::Pixbuf::create_from_xpm_data(mail_read_xpm);
        mail_replied_buf = Gdk::Pixbuf::create_from_xpm_data(mail_replied_xpm);

        attachment_buf = Gdk::Pixbuf::create_from_xpm_data(attachment_xpm);
        empty_buf = Gdk::Pixbuf::create_from_xpm_data(empty_xpm);

        app_buf =   Gdk::Pixbuf::create_from_xpm_data(application_xpm);
        audio_buf = Gdk::Pixbuf::create_from_xpm_data(audio_xpm);
        dir_open_buf = Gdk::Pixbuf::create_from_xpm_data(dir_open_xpm);
        dir_close_buf = Gdk::Pixbuf::create_from_xpm_data(dir_close_xpm);
        audio_buf = Gdk::Pixbuf::create_from_xpm_data(audio_xpm);
        image_buf = Gdk::Pixbuf::create_from_xpm_data(image_xpm);
        multi_buf = Gdk::Pixbuf::create_from_xpm_data(multipart_xpm);
        text_buf =  Gdk::Pixbuf::create_from_xpm_data(text_xpm);
    }
    
    bool Config::use_toolbar_text() {
        return true;
    }
    
    std::list<Config::MailBox> Config::get_mailboxes() {
        return m_mailboxes;
    }
    
    void Config::load() {
        load(m_file);
    }

    void Config::save() {
        save(m_file);
    }
    
    void Config::create(std::string file) {
        if(getenv("GTKMAIL_CONFIG_DEBUG"))
            std::cerr << "Config::create(" << file << ")" << std::endl;


        util::xml::node::ptr node = util::xml::node::create("gtkmail");

        m_doc.get_list().push_back(node);

        save(file);
        load(file);
    }


    void Config::load(std::string file) {
        using namespace jlib::crypt;
        static bool init = false;

        if(getenv("GTKMAIL_CONFIG_DEBUG"))
            std::cerr << "Config::load(" << file << ")" << std::endl;

        // do this before anything
        if(!init) {
            gpg::init(GPGME_PROTOCOL_OpenPGP);
            init = true;
        }

        // find out the gpg id used for the config file, then use it to open/decrypt
        get_gpg_id();

        std::ifstream ifs(file.c_str());
        if(!ifs) {
            if(getenv("GTKMAIL_CONFIG_DEBUG"))
                std::cerr << "Config::load: no ifstream for file, create" << std::endl;

            create(file);
            return;
        }

        std::string data;
        sys::read(ifs, data);

        if(data.size() == 0) {
            if(getenv("GTKMAIL_CONFIG_DEBUG"))
                std::cerr << "Config::load: file was empty, create" << std::endl;

            create(file);
            return;
        }

        gpg::ctx ctx;
        gpg::data::ptr cipher = gpg::data::create(data);
        gpg::data::ptr plain = gpg::data::create();

        ctx.set_passphrase_cb(gtkmail_passphrase_cb);
        ctx.set_keylist_mode(1);
        ctx.op_decrypt(cipher, plain);
        
        std::istringstream iss(plain->read());
        iss >> *this;
    }

    void Config::save(std::string file) {
        if(getenv("GTKMAIL_CONFIG_DEBUG"))
            std::cerr << "Config::save(" << file << ")" << std::endl;

        using namespace jlib::crypt;
        static bool init = false;

        // do this before anything
        if(!init) {
            gpg::init(GPGME_PROTOCOL_OpenPGP);
            init = true;
        }

        // find out the gpg id used for the config file, then use it to open/decrypt
        get_gpg_id();

        std::ostringstream oss;
        oss << *this;

        gpg::ctx ctx;
        gpg::data::ptr cipher = gpg::data::create();
        gpg::data::ptr plain = gpg::data::create(oss.str());
        gpg::key::list keys = gpg::list_keys(m_gpg_id);

        if(getenv("GTKMAIL_CONFIG_DEBUG"))
            std::cerr << "Config::save: found " << keys.size() << " keys for id " << m_gpg_id << std::endl;

        gpg::key::list rcpts;
        gpg::key::ptr key = keys.front();

        rcpts.push_back(key);

        //ctx.set_passphrase_cb(gtkmail_passphrase_cb);
        ctx.set_keylist_mode(1);
        ctx.op_encrypt(rcpts, plain, cipher);

        gpgme_encrypt_result_t res = ctx.op_encrypt_result();

        if(res && res->invalid_recipients)
            std::cerr << "Config::save: result->invalid_recipients: " << res->invalid_recipients << std::endl;
        
        std::ofstream ofs(file.c_str());
        ofs << cipher->read();
    }

    void Config::get_gpg_id() {
        using namespace jlib::crypt;

        // read the id from a file; prompt the user if needed and store initial value
        if(m_gpg_id == "") {
            std::ifstream ids(m_gpg_id_file.c_str());
            if(!ids) {
                ids.close();
                std::ofstream ods(m_gpg_id_file.c_str());
                EntryDialog dialog("Please enter a gpg id to use for crypto operations on the config file:");
                dialog.run();
                
                ods << dialog.get_text();
                ods.close();
                
                m_gpg_id = dialog.get_text();
            } else {
                sys::read(ids, m_gpg_id);
            }
            
            m_gpg_id = trim(m_gpg_id);
        }
        
        // check to make sure there is a key with this id
        /*
        std::list<gpg::Key> keys = gpg::list_keys(m_gpg_id);
        while(keys.size() == 0) {
            std::ofstream ods(m_gpg_id_file.c_str());
            std::string title = "ERROR: no keys for id " + m_gpg_id + "\n"
                "Please enter a gpg id to use for crypto operations on the config file:";
            EntryDialog dialog(title);
            dialog.run();

            ods << dialog.get_text();
            ods.close();

            std::ifstream ids(m_gpg_id_file.c_str());
            sys::read(ids, m_gpg_id);
            //m_gpg_id = trim(m_gpg_id);

            keys = gpg::list_keys(m_gpg_id);
        }
        */
    }


    void Config::parse_document() {
        xml::node::list root = m_doc.get_list();
        if(root.size() != 1 || root.front()->get_name() != GTKMAIL_CONFIG_ROOT_NODENAME) {
            std::cerr << "gtkmail::Config::parse_document(): error"<<std::endl;
            std::cerr << "\troot.size() = "<<root.size()<<std::endl;
            if(root.size() >= 1)
                std::cerr << "\troot.front()->get_name() = "<< root.front()->get_name() 
                          <<std::endl;
            exit(1);
        }
        
        m_mailboxes.clear();
        xml::node::list toplevel = root.front()->get_list();
        xml::node::list::iterator i = toplevel.begin();
        for(;i!=toplevel.end();i++) {
            xml::node::ptr node = *i;
            if(node->get_name() == GTKMAIL_CONFIG_MAILBOX_NODENAME) {
                m_mailboxes.push_back(MailBox(node));
            }
        }
        
    }

    void Config::clear() { 
        m_mailboxes.clear(); 
    }

    void Config::push_back(const_reference r) {
        m_mailboxes.push_back(r);
        xml::node::ptr node = r.get_node();
        m_doc.get_list().front()->add(node);
    }

    void Config::erase(Config::iterator i) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator j = find_mailbox_node(i->get_name());
        if(j != m_doc.get_list().front()->get_list().end()) {
            list.erase(j);
        }
    }
    
    void Config::replace(iterator i, const_reference r) {
        i->copy(r);
    }

    Config::iterator Config::find(std::string name) {
        iterator i = begin();

        for(;i!=end();i++) {
            if(i->get_name() == name)
                return i;
        }

        return end();
    }

    Config::const_iterator Config::find(std::string name) const {
        const_iterator i = begin();

        for(;i!=end();i++) {
            if(i->get_name() == name)
                return i;
        }

        return end();
    }

    xml::node::list::iterator Config::find_mailbox_node(std::string name) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::list::iterator i = list.begin();
        for(; i != list.end(); i++) {
            if((*i)->get_attribute(GTKMAIL_CONFIG_MAILBOX_NAME_ATTR) == name) {
                return i;
            }
        }
        return list.end();
    }

    /*
    Config::const_iterator Config::find(gtkmail::MailBox* box) const {
        for(const_iterator i=begin();i!=end();i++) {
            if(*i == box)
                return i;
        }
        return end();
    }

    Config::iterator Config::find(gtkmail::MailBox* box) {
        for(iterator i=begin();i!=end();i++) {
            if(*i == box)
                return i;
        }
        return end();
    }
    */

    std::string Config::get_gtkmail_dir() {
        return (std::string(getenv("HOME"))+"/.gtkmail");
    }

    std::string Config::get_message_font() {
        return get_pref("font", "Mono 10");
    }

    void Config::set_message_font(std::string font) {
        set_pref("font", font);
    }
    

    std::string Config::get_default_charset() {
        return get_pref("charset", "ISO-8859-1");
    }

    void Config::set_default_charset(std::string charset) {
        set_pref("charset", charset);
    }
    

    std::vector<std::string> Config::get_my_addrs() {
        std::vector<std::string> addrs;
        for(iterator i=begin();i!=end();i++) {
            if(std::find(addrs.begin(), addrs.end(), i->get_addr()) == addrs.end()) {
                addrs.push_back(i->get_addr());
            }
        }
        return addrs;
    }

    Gtk::ToolbarStyle Config::get_toolbar_style() {
        std::string style = lower(get_pref("toolbar_style", "both_horiz"));

        if(style == "icons") {
            return Gtk::TOOLBAR_ICONS;
        }
        else if(style == "text") {
            return Gtk::TOOLBAR_TEXT;
        }
        else if(style == "both") {
            return Gtk::TOOLBAR_BOTH;
        }
        else if(style == "both_horiz") {
            return Gtk::TOOLBAR_BOTH_HORIZ;
        }
        else {
            throw bad_style(style);
        }

        return Gtk::TOOLBAR_BOTH_HORIZ;
    }

    void Config::set_toolbar_style(Gtk::ToolbarStyle style) {
        std::string s;
        switch(style) {
        case Gtk::TOOLBAR_ICONS:
            s = "icons";
            break;
        case Gtk::TOOLBAR_TEXT:
            s = "text";
            break;
        case Gtk::TOOLBAR_BOTH:
            s = "both";
            break;
        case Gtk::TOOLBAR_BOTH_HORIZ:
            s = "both_horiz";
            break;
        default:
            throw bad_style("Unknown enumerated Gtk::ToolbarStyle value: " + string_value(static_cast<int>(style)));
        }

        set_pref("toolbar_style", s);
    }

    std::string Config::get_user_style() {
        return get_pref("user_style", "");
    }

    void Config::set_user_style(std::string s) {
        set_pref("user_style", s);
    }

    std::string Config::get_default_text() {
        return get_pref("default_text", "");
    }

    void Config::set_default_text(std::string s) {
        set_pref("default_text", s);
    }

    bool Config::get_auto_load() {
        return get_pref("auto_load", "false") == "true";
    }

    void Config::set_auto_load(bool b) {
        set_pref("auto_load", b ? "true" : "false");
    }

    jlib::util::xml::node::ptr Config::find_node(std::string name) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        for(xml::node::list::iterator i=list.begin();i!=list.end();i++) {
            if((*i)->get_name() == name) {
                return (*i);
            }
        }
        
        return jlib::util::xml::node::ptr();
    }

    jlib::util::xml::node::ptr Config::append(std::string name, std::string key, std::string val) {
        xml::node::list& list = m_doc.get_list().front()->get_list();
        xml::node::ptr node = xml::node::create(name); 

        if(key != "") {
            node->set_attribute(key, val);
        }
        list.insert(list.begin(), node);

        return node;
    }

    std::string Config::get_pref(std::string key, std::string def) {
        xml::node::ptr node = find_node("prefs");
        if(!node) {
            node = append("prefs", key, def);
        } else {
            xml::node::attributes& attrs = node->get_attributes();
            if(attrs.find(key) == attrs.end()) {
                node->set_attribute(key, def);
            }
        }

        return node->get_attribute(key);
    }

    void Config::set_pref(std::string key, std::string val) {
        xml::node::ptr node = find_node("prefs");
        if(node) {
            node->set_attribute(key, val);
        } else {
            node = append("prefs", key, val);
        }
    }



    
    std::istream& operator>>(std::istream& i, Config& config) {
        config.m_doc.load(i);
        config.parse_document();
        return i;
    }

    std::ostream& operator<<(std::ostream& o, const Config& config) {
        config.m_doc.save(o);
        return o;
    }


}
