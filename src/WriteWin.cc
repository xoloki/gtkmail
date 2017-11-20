/* -*- mode: C++ c-basic-offset: 4  -*-
 * WriteWin.C - source file for class WriteWin
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

#include "WriteWin.hh"
#include "Dialog.hh"
#include "SignalChain.hh"
#include "Config.hh"
#include "AddressBook.hh"
#include "MailBox.hh"
#include "MailView.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtkmm/main.h>
#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>

#include <sigc++/bind.h>

#include <jlib/crypt/crypt.hh>

#include <jlib/net/net.hh>

#include <jlib/sys/sys.hh>
#include <jlib/sys/tfstream.hh>

#include <jlib/util/util.hh>
#include <jlib/util/MimeType.hh>
#include <jlib/util/Regex.hh>
#include <jlib/util/Date.hh>

#include <list>
#include <algorithm>

const int BSIZE=1024;
const bool DEBUG = false;

const int CONTENT_TYPE_COL=0;
const int ENCODING_COL=1;
const int FILENAME_COL=2;

const unsigned int REPLY_ATTR = 0;
const unsigned int REPLY_ALL_ATTR = 1;
const unsigned int REPLY_QUOTE_ATTR = 2;
const unsigned int FORWARD_ATTR = 3;
const unsigned int FORWARD_ATTACH_ATTR = 4;
const unsigned int FORWARD_SPAM_ATTR = 5;

const u_int REPLY_AUTO_BREAK_THRESHOLD = 80;
const u_int REPLY_AUTO_BREAK_AT = 72;

namespace gtkmail {
    
    WriteWin::WriteWin(const jlib::net::Email& e, 
                       MailBox* box, 
                       jlib::net::folder_info_type info,
                       bool reply, 
                       bool reply_all, 
                       bool reply_quote, 
                       bool forward, 
                       bool forward_attach, 
                       bool forward_spam)
        : MessageWin(jlib::net::Email(), box, info)
    {
        
        jlib::net::Email& email = const_cast<jlib::net::Email&>(e);
        m_forward = email;
        m_attr.push_back(reply);
        m_attr.push_back(reply_all);
        m_attr.push_back(reply_quote);
        m_attr.push_back(forward);
        m_attr.push_back(forward_attach);
        m_attr.push_back(forward_spam);
        
        m_headers = manage(new Gtk::Table(4, 2, false));
        
        Gtk::Label* from_label = manage(new Gtk::Label("From: ",1));
        Gtk::Label* to_label = manage(new Gtk::Label("To: ",1));
        Gtk::Label* cc_label = manage(new Gtk::Label("Cc: ",1));
        Gtk::Label* subj_label = manage(new Gtk::Label("Subject: ",1));
        
        m_headers->attach(*from_label,     0, 1, 0, 1, Gtk::FILL);
        m_headers->attach(*to_label,     0, 1, 1, 2, Gtk::FILL);
        m_headers->attach(*cc_label,     0, 1, 2, 3, Gtk::FILL);
        m_headers->attach(*subj_label,   0, 1, 3, 4, Gtk::FILL);
        //m_headers->attach(attach_label, 0, 1, 3, 4, Gtk::FILL);
        
        m_headers->attach(*m_from,     1, 2, 0, 1);
        m_headers->attach(*m_to,     1, 2, 1, 2);
        m_headers->attach(*m_cc,     1, 2, 2, 3);
        m_headers->attach(*m_subj,   1, 2, 3, 4);
        //m_headers->attach(attachCombo, 1, 2, 3, 4); 
        
        m_notebook->pages().push_front(Gtk::Notebook_Helpers::TabElem(*m_headers, "Headers"));
        //m_headers->pack_start(*m_headers, true, true, 10);
        
        m_from->set_popdown_strings(Config::global.get_my_addrs());
        if(box != 0 && box->get_address() != "") {
            m_from->get_entry()->set_text(box->get_address());
        }
        m_message->set_editable(true);
        
        m_to->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &WriteWin::complete_address), m_to));
        m_cc->signal_activate().connect(sigc::bind(sigc::mem_fun(*this, &WriteWin::complete_address), m_cc));
        
        build_tree();
        //Gtk::CTree_Helpers::RowList rows = m_attachlist->rows();
        //build_tree(rows, m_email);
        
        init_menus();
        init_toolbar();
        
        //m_vbox->pack_start(*m_headers);
        Gtk::ScrolledWindow* message_win = manage(new Gtk::ScrolledWindow());
        message_win->add(*m_message);
        m_vbox->pack_start(*message_win, true, true, 0);
        
        //show_all();
        
        m_message->set_sensitive(false);
        std::pair< std::string, std::vector<std::string> > host_info;
        if(!forward_spam) {
            init_fields("","");
        }
        else {
            /*
             * we're trying to do a spam lookup, so let's start with the final
             * Received: header and work backwards until we find something with an
             * IP addr we can lookup 
             *
             * *** now done in jlib::net::Email::get_received_ip() ***
             */
            
            std::string addr = m_forward.get_received_ip();
            //std::cerr << "m_forward.get_received_ip() = "<<addr <<std::endl;
            
            if(addr != "") {
                sigc::slot0< std::pair< std::string,std::vector<std::string> > > s0 
                    = sigc::bind(sigc::ptr_fun(&jlib::net::get_host), addr);
                gtkmail::SignalChain0< std::pair< std::string,std::vector<std::string> > >* siggy 
                    = new gtkmail::SignalChain0< std::pair< std::string,std::vector<std::string> > >(s0);
                
                siggy->succeed.connect(sigc::mem_fun(*this,&gtkmail::WriteWin::init_fields_from_dns));
                siggy->fail.connect(sigc::mem_fun(*this,&gtkmail::WriteWin::begin_whois_chain));
                siggy->error.connect(sigc::bind(sigc::ptr_fun(&gtkmail::display_exception),this));
                siggy->start();
            }
            else {
                init_fields("","");
            }
        }
        
        
    }
    
    /*
      WriteWin::~WriteWin()
    */
    WriteWin::~WriteWin() {
        
    }
    
    void WriteWin::init_fields_from_dns(std::pair<std::string,std::vector<std::string> > dns) {
        std::string host = dns.first;
        std::string addr;
        std::string body;
        
        if(host != "" && dns.second.size() > 0) {
            std::string ip = dns.second[0];
            std::string domain;
            std::string::size_type i = host.rfind(".");
            if(i != std::string::npos) {
                i = host.rfind(".",i-1);
                if(i != std::string::npos) {
                    domain = host.substr(i+1);
                    addr = "abuse@"+domain;
                }
            }
            
            body = (std::string("\t")+host+" ["+ip+ std::string("]\n\n"));
        }
        
        init_fields(addr,body);
    }
    
    void WriteWin::begin_whois_chain() {
        std::string tmp;
        sigc::slot0<std::string> s0 = sigc::mem_fun(*this,&gtkmail::WriteWin::get_whois);
        gtkmail::SignalChain0<std::string>* chain = new gtkmail::SignalChain0<std::string>(s0);
        
        chain->succeed.connect(sigc::mem_fun(*this,&gtkmail::WriteWin::init_fields_from_whois));
        chain->fail.connect(sigc::bind(sigc::mem_fun(*this,&gtkmail::WriteWin::init_fields), tmp,tmp));
        chain->error.connect(sigc::bind(sigc::ptr_fun(&gtkmail::display_exception),this));;
        chain->start();
    }
    
    std::string WriteWin::get_whois() {
        std::string whois, addr, err;
        addr = m_forward.get_received_ip();
        
        jlib::sys::shell("whois "+addr,whois,err);
        
        return whois;
    }
    
    void WriteWin::init_fields_from_whois(std::string whois) {
        std::string addr, body = "\t["+m_forward.get_received_ip()+
            "]\n\nWHOIS maps this to you guys:\n\n"+whois+"\n";
        
        u_int i,j, k;
        u_int l = std::string("NETBLK-COM-").length();
        std::string domain;
        if((i=whois.find("NETBLK-COM-")) != whois.npos) {
            k = i+l;
            j = whois.find_first_of("- ",k);
            if(j != whois.npos)
                domain = jlib::util::lower(whois.substr(k,j-k))+".com";
        }
        else if((i=whois.find("NETBLK-NET-")) != whois.npos) {
            k = i+l;
            j = whois.find_first_of("- ",k);
            if(j != whois.npos)
                domain = jlib::util::lower(whois.substr(k,j-k))+".net";
        }
        else if((i=whois.find("NETBLK-ORG-")) != whois.npos) {
            k = i+l;
            j = whois.find_first_of("- ",k);
            if(j != whois.npos)
                domain = jlib::util::lower(whois.substr(k,j-k))+".org";
        }
        else {
            jlib::util::Regex addr_exp("([[:alnum:]]+@[[:alnum:].]+)");
            if(addr_exp(whois)) {
                addr = addr_exp[1];
                std::string::size_type i = addr.find("@");
                if(i != std::string::npos) {
                    domain = addr.substr(i+1);
                }
            }
        }
        
        addr = "abuse@"+domain;
        init_fields(addr,body);
    }
    
    void WriteWin::init_fields(std::string abuse_addr, std::string spam_body) {
        Glib::ustring body, usubj, uto, ucc;
        std::string subj = m_forward["subject"], ex_to;
        
        
        usubj = MailView::get_header(m_forward, "subject");
        
        // set up Subject: entry
        if(m_attr[FORWARD_ATTR]) {
            if(jlib::util::lower(subj).find("fwd:") != 0) {
                usubj = "Fwd: " + usubj;
            }
            
        }
        else if(m_attr[REPLY_ATTR]) {
            if(jlib::util::lower(subj).find("re:") != 0) {
                usubj = "Re: " + usubj;
            }
        }
        m_subj->set_text(usubj);
        
        // set up content-type
        if(m_attr[FORWARD_ATTR] && m_attr[FORWARD_ATTACH_ATTR]) {
            std::string bound = jlib::util::valueOf(rand())+"gtkmail"+jlib::util::valueOf(rand());
            m_email.set("CONTENT-TYPE", "multipart/mixed; boundary=\""+bound+"\"");
            m_email.push_back(m_forward);
        }
        else {
            m_email.set("CONTENT-TYPE","text/plain");
        }
        
        // get the .sig
        std::string sig_path = std::string(getenv("HOME")) + "/.signature";
        std::string sig_buf;
        try {
            jlib::util::file::getstat(sig_path);
            std::ifstream sig(sig_path.c_str());
            jlib::sys::getstring(sig, sig_buf);
        }
        catch(std::exception& e) {
            
        }
        std::string sig_body = "\n\n"+sig_buf;
        
        // set up body
        if(m_attr[FORWARD_ATTR]) {
            if(m_attr[FORWARD_SPAM_ATTR]) {
                body="Hi, I received the following spam today from a computer in your network:\n\n"+
                    spam_body+
                    ("Thanks for taking care of this for me.\n\ncheers,\n"+
                     sig_buf+"\nForwarded Message\n-----------------\n"+m_forward.raw());
            }
            else if(!m_attr[FORWARD_ATTACH_ATTR]) {
                body = sig_body+"\nForwarded Message\n-----------------\n"+m_forward.raw();
            }
            else {
                body = sig_body;
            }
        }
        else if(m_attr[REPLY_ATTR]) {
            if(m_attr[REPLY_QUOTE_ATTR]) {
                std::list<std::string> lines = 
                    jlib::util::tokenize_list(m_forward.get_primary_text_html_render(), "\n", true);
                std::list<std::string>::iterator i,j,k;
                std::string p,q;
                u_int t;
                for(i = lines.begin(); i != lines.end(); i++) {
                    j = i; k=j; k++;
                    while(j->length() > REPLY_AUTO_BREAK_THRESHOLD) {
                        t = j->rfind(' ', REPLY_AUTO_BREAK_AT);
                        if(t == j->npos)
                            break;
                        else {
                            p = j->substr(0,t);
                            q = j->substr(t+1);
                            *j = p;
                            j = lines.insert(k,q);
                        }
                    }
                }
                for(i = lines.begin(); i != lines.end(); i++) {
                    body += ("> "+ *i + "\n");
                }
                body += sig_body;
            }
            else {
                body = sig_body;
            }
        }
        else {
            body = sig_body;
        }
        
        set_text(MailView::get(body));
        
        // set To: field 
        if(m_attr[FORWARD_ATTR]) {
            if(m_attr[FORWARD_SPAM_ATTR]) {
                uto = MailView::get(abuse_addr);
            }
        }
        else if(m_attr[REPLY_ATTR]) {
            if(m_forward["REPLY-TO"] != "") {
                uto = (MailView::get_header(m_forward, "REPLY-TO"));
            }
            else {
                uto = (MailView::get_header(m_forward, "FROM"));
            }
        }
        else {
            
        }
        
        m_to->set_text(uto);
        ex_to = jlib::net::extract_address(jlib::util::lower(uto));
        
        // set Cc: field
        if(m_attr[FORWARD_ATTR]) {
            if(m_attr[FORWARD_SPAM_ATTR]) {
                /*
                  if(abuse_addr.find("abuse") != 0) {
                  std::string::size_type i = abuse_addr.find("@");
                  if(i != std::string::npos) {
                  if(i != abuse_addr.length()-1) {
                  std::string domain = abuse_addr.substr(i+1);
                  m_cc->set_text("abuse@"+domain);
                  }
                  else {
                  
                  }
                  
                  }
                  
                  }
                */
            }            
        }
        else if(m_attr[REPLY_ATTR]) {
            if(m_attr[REPLY_ALL_ATTR]) {
                std::string to_charset, cc_charset;
                std::string to = m_forward.headers().get("to", to_charset);
                std::string cc = m_forward.headers().get("cc", cc_charset);
                
                Glib::ustring ucc;
                
                std::list<std::string> to_split = jlib::net::split_addresses(to);
                std::list<std::string> cc_split = jlib::net::split_addresses(cc);
                
                if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
                    std::cerr << "to: " << to << std::endl
                              << "cc: " << cc << std::endl;
                
                std::vector<std::string> my_addr = Config::global.get_my_addrs();
                std::vector<std::string> up_addr;
                std::vector<std::string> ex_addr;
                
                transform(my_addr.begin(), my_addr.end(), back_inserter(up_addr), jlib::util::lower);
                transform(up_addr.begin(), up_addr.end(), back_inserter(ex_addr), jlib::net::extract_address);
                
                for(std::list<std::string>::iterator i = to_split.begin(); i != to_split.end(); i++) {
                    std::string ex = jlib::util::lower(jlib::net::extract_address(*i));
                    if(find(ex_addr.begin(), ex_addr.end(), ex) == ex_addr.end()) {
                        ucc += MailView::get((ucc != "" ?  (", " + *i) : *i), to_charset);
                    }
                }
                
                for(std::list<std::string>::iterator i = cc_split.begin(); i != cc_split.end(); i++) {
                    std::string ex = jlib::util::lower(jlib::net::extract_address(*i));
                    if(find(ex_addr.begin(), ex_addr.end(), ex) == ex_addr.end()) {
                        ucc += MailView::get((ucc != "" ?  (", " + *i) : *i), cc_charset);
                    }
                }
                
                m_cc->set_text(ucc);
            }
            else {
                
            }
        }
        else {
            
        }        
        m_message->set_sensitive(true);
        show_all();
    }
    
    std::string WriteWin::get_signature() {
        std::string sig_path = std::string(getenv("HOME")) + "/.signature";
        std::string sig_buf;
        try {
            jlib::util::file::getstat(sig_path);
            std::ifstream sig(sig_path.c_str());
            jlib::sys::getstring(sig, sig_buf);
        }
        catch(std::exception& e) {
            
        }
        return ("\n\n"+sig_buf);
    }
    
    
    /*
      void messageSend_call();
    */
    void WriteWin::message_send_call() {
        if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
            std::cout << "gtkmail::WriteWin::message_send_call()" << std::endl;
        try {
            
            std::string bound;
            std::string rcpt = jlib::util::Headers::encode(MailView::unget(m_to->get_text()),
                                                           Config::global.get_default_charset());
            std::string cc = jlib::util::Headers::encode(MailView::unget(m_cc->get_text()),
                                                         Config::global.get_default_charset());
            std::string mail = jlib::util::Headers::encode(MailView::unget(m_from->get_entry()->get_text()),
                                                           Config::global.get_default_charset());
            std::string subj = jlib::util::Headers::encode(MailView::unget(m_subj->get_text()),
                                                           Config::global.get_default_charset());
            
            std::string data = MailView::unget(get_text());
            std::string msgid;
            
            rcpt.erase(rcpt.find_last_not_of("\r\n")+1);
            cc.erase(cc.find_last_not_of("\r\n")+1);
            mail.erase(mail.find_last_not_of("\r\n")+1);
            subj.erase(subj.find_last_not_of("\r\n")+1);
            
            m_email.set("FROM", mail);
            m_email.set("TO", rcpt);
            if(cc != "")
                m_email.set("CC", cc);
            m_email.set("SUBJECT", subj);
            if( (msgid=m_forward.find("MESSAGE-ID")) != "" ) {
                m_email.set("IN-REPLY-TO", msgid);
            }
            
            if(cc.length() > 0) {
                rcpt += (","+cc);
            }
            
            if(m_email["CONTENT-TYPE"] == "text/plain") {
                m_email.set("CONTENT-TYPE", "text/plain; charset="+Config::global.get_default_charset());
                m_email.data(data);
            }
            else {
                jlib::net::Email email;
                email.set("CONTENT-TYPE", "text/plain; charset="+Config::global.get_default_charset());
                email.data(data);
                m_email.attach().insert(m_email.attach().begin(), email);
            }
            
            if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
                std::cout << "gtkmail::WriteWin::message_send_call: build email"  << std::endl;
            
            m_email.build();
            
            if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
                std::cout << m_email.raw() << std::endl;
            
            Config::iterator i = Config::global.find(m_box->get_name());
            sigc::slot5<void,std::string,std::string,std::string,std::string,unsigned int> s5;

            if(i->get_smtp_auth()) {
                std::string user, pass;

                if(i->get_smtp_auth_same()) {
                    util::URL url = i->get_url();
                    user = url.get_user();
                    pass = url.get_pass();
                } else {
                    user = i->get_smtp_user();
                    pass = i->get_smtp_pass();
                }
                s5 = sigc::bind(sigc::ptr_fun(&jlib::net::smtp::send_tls_auth), user, pass);
            } else {
                s5 = sigc::ptr_fun(i->get_smtp_tls() ? &jlib::net::smtp::send_tls : &jlib::net::smtp::send);
            }
            
            sigc::slot3<void,std::string,std::string,std::string> s3;
            s3 = sigc::bind(s5, i->get_smtp(), i->get_smtp_port());
            
            sigc::slot1<void,std::string> s1 = sigc::bind(s3,rcpt,m_email.raw());
            sigc::slot0<void> s0 = sigc::bind(s1,mail);
            
            gtkmail::SignalChain0<void>* siggy = new gtkmail::SignalChain0<void>(s0);
            
            set_sensitive(false);
            
            jlib::util::Date date;
            if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
                std::cout << "gtkmail::WriteWin::message_send_call: build email with date"  << std::endl;
            
            jlib::net::Email sent("Date: "+date.get()+"\n"+m_email.raw());
            
            if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
                std::cout << sent.raw() << std::endl;
            
            siggy->succeed.connect(sigc::bind(sigc::mem_fun(*this, &gtkmail::WriteWin::on_send), sent));
            siggy->succeed.connect(sigc::mem_fun(*this, &gtkmail::WriteWin::destroy_));
            
            siggy->error.connect(sigc::bind(sigc::ptr_fun(&gtkmail::display_exception),this));
            siggy->fail.connect(sigc::bind(sigc::mem_fun(*this,&gtkmail::WriteWin::set_sensitive),true));
            
            siggy->start();
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
        
        
    }
    
    void WriteWin::init_menus() {
        static const Glib::ustring ui =
            "<ui>"
            "  <menubar name='MenuBar'>"
            "    <menu action='Message'>"
            "      <menuitem action='Send'/>"
            "      <menuitem action='Resume'/>"
            "    </menu>"
            "    <menu action='Attach'>"
            "      <menuitem action='AttachFile'/>"
            "    </menu>"
            "    <menu action='Crypt'>"
            "      <menuitem action='Sign'/>"
            "      <menuitem action='Encrypt'/>"
            "      <menuitem action='SignEncrypt'/>"
            "    </menu>"
            "  </menubar>"
            "</ui>";
        
        
        m_write_actions = Gtk::ActionGroup::create();
        m_write_actions->add(Gtk::Action::create("Send", "_Send"), Gtk::AccelKey("<control>Return"),
                       sigc::bind(sigc::mem_fun(*this, &WriteWin::message_call), std::string("Send")));
        m_write_actions->add(Gtk::Action::create("Resume", "_Attach Resume"), 
                       sigc::bind(sigc::mem_fun(*this, &WriteWin::message_call), std::string("Attach Resume")));
        m_write_actions->add(Gtk::Action::create("AttachFile", "Attach File"), 
                       sigc::bind(sigc::mem_fun(*this, &WriteWin::attach_call), std::string("Attach File")));
        m_write_actions->add(Gtk::Action::create("Sign", "_Sign"), Gtk::AccelKey("<control>I"), 
                       sigc::bind(sigc::mem_fun(*this, &WriteWin::crypt_call), std::string("Sign")));
        m_write_actions->add(Gtk::Action::create("Encrypt", "_Encrypt"), Gtk::AccelKey("<control>N"), 
                       sigc::bind(sigc::mem_fun(*this, &WriteWin::crypt_call), std::string("Encrypt")));
        m_write_actions->add(Gtk::Action::create("SignEncrypt", "Sign/Encrypt"), Gtk::AccelKey("<control>I"), 
                       sigc::bind(sigc::mem_fun(*this, &WriteWin::crypt_call), std::string("Sign/Encrypt")));
        
        m_ui->insert_action_group(m_write_actions);
        m_ui->add_ui_from_string(ui);
        
        add_accel_group(m_ui->get_accel_group());
    }
    
    void WriteWin::init_toolbar() {
#if 0
        using namespace Gtk::Toolbar_Helpers;
        
        m_tools->tools().push_back(ButtonElem("Compose",
                                              *manage(new Gtk::Image(Config::global.mail_compose_buf->copy())),
                                              sigc::bind(sigc::mem_fun(*this,&WriteWin::message_call),
                                                         std::string("Compose")),
                                              "Compose",
                                              "Compose new mail"));
        
        m_tools->tools().push_back(Space());
        
        m_tools->tools().push_back(ButtonElem("Send",
                                              *manage(new Gtk::Image(Config::global.mail_forward_buf->copy())),
                                              sigc::bind(sigc::mem_fun(*this,&WriteWin::message_call),
                                                         std::string("Send")),
                                              "Send",
                                              "Send mail"));
#endif
    }
    
    void WriteWin::message_call(std::string s) {
        if(s == "Send") {
            message_send_call();
        }
        else if(s == "Attach Resume") {
            std::string body = "Hi, I'm writing in reference to a job posting I saw on ";
            if(getenv("GTKMAIL_RESUME_HOST")) {
                body += getenv("GTKMAIL_RESUME_HOST");
            }
            else {
                body += "dice.com";
            }
            
            body +=".\n\nI'm attaching my resume in plain text; below is a link to a MS Word\nformatted version.\n\n";
            
            if(getenv("GTKMAIL_RESUME_LINK")) {
                body += (std::string("    ")+getenv("GTKMAIL_RESUME_LINK"));
            }
            else {
                body += ("    http://www.divisionbyzero.com/jwy/resume.doc");
            }
            
            body += ("\n\ncheers,"+get_signature().substr(1));
            set_text(body);
            
            if(getenv("GTKMAIL_RESUME_PATH")) {
                attach_file(getenv("GTKMAIL_RESUME_PATH"));
            }
            else {
                attach_file("/home/jwy/doc/resume.txt");
            }
        }
        else {
            MessageWin::message_call(s);
        }
    }
    
    void WriteWin::attach_call(std::string s) {
        if(s == "Attach File") {
            attach_file_call();
        }
    }
    
    void WriteWin::crypt_call(std::string s) {
        using namespace jlib::crypt;
        try {
            gpg::ctx ctx;
            static bool init = false;
            
            if(!init) {
                gpg::init(GPGME_PROTOCOL_OpenPGP);
                init = true;
            }
            
            ctx.set_armor();
            ctx.set_passphrase_cb(gtkmail_passphrase_cb);
            ctx.set_keylist_mode(1);
            
            if(s == "Encrypt") {
                gpg::key::list rcpts;
                std::list<std::string> addrs = jlib::net::extract_addresses(m_to->get_text());
                for(std::list<std::string>::iterator i = addrs.begin(); i != addrs.end(); i++) {
                    gpg::key::list keys = gpg::list_keys(*i);
                    if(!keys.size()) {
                        display_exception("Unable to encrypt: no key for " + *i);
                        return;
                    }
                    gpg::key::ptr key = keys.front();
                    rcpts.push_back(key);
                }
                
                gpg::data::ptr plain = gpg::data::create(this->get_text());
                gpg::data::ptr cipher = gpg::data::create();
                
                ctx.op_encrypt(rcpts, plain, cipher);
                
                set_text(cipher->read());
            }
            if(s == "Sign" || s == "Sign/Encrypt") {
                std::string afrom = jlib::net::extract_address(m_from->get_entry()->get_text());
                gpg::key::list kfrom = gpg::list_keys(afrom);
                
                gpg::data::ptr plain = gpg::data::create(this->get_text());
                gpg::data::ptr sign = gpg::data::create();
                
                if(!kfrom.size()) {
                    display_exception("Unable to sign: no key for " + afrom);
                    return;
                }
                
                ctx.signers_add(kfrom.front());
                
                if(s == "Sign") {
                    ctx.op_sign(plain, sign, GPGME_SIG_MODE_CLEAR);
                    set_text(sign->read());
                }
                else if(s == "Sign/Encrypt") {
                    gpg::key::list rcpts;
                    std::list<std::string> addrs = jlib::net::extract_addresses(m_to->get_text());
                    for(std::list<std::string>::iterator i = addrs.begin(); i != addrs.end(); i++) {
                        gpg::key::list keys = gpg::list_keys(*i);
                        if(!keys.size()) {
                            display_exception("Unable to encrypt: no key for " + *i);
                            return;
                        }
                        gpg::key::ptr key = keys.front();
                        rcpts.push_back(key);
                    }
                    ctx.op_encrypt_sign(rcpts, plain, sign);
                    set_text(sign->read());
                }
            }
            
        }
        catch(std::exception& e) {
            display_exception(e.what(), this);
        }
    }
    
    void WriteWin::attach_file_call() {
        Gtk::FileSelection* file_sel = manage(new Gtk::FileSelection("Attach File"));
        
        file_sel->set_position(Gtk::WIN_POS_CENTER);
        file_sel->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &WriteWin::attach_file_exec), file_sel));
        file_sel->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&operator delete), file_sel));
        file_sel->get_cancel_button()->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&operator delete), file_sel));
        
        file_sel->show_all();    
    }
    
    void WriteWin::attach_file_exec(Gtk::FileSelection* file_sel) {
        std::string file = file_sel->get_filename();
        attach_file(file);
    }
    
    void WriteWin::attach_file(std::string file) {
        try {
            jlib::net::Email email;
            std::string name = file;
            std::string::size_type p = file.rfind("/");
            if(p != std::string::npos) {
                name = file.substr(p+1);
            }
            std::string type = jlib::util::MimeType::get_type_from_file(file);
            email.set("CONTENT-TYPE", type+"; name=\""+name+"\"");
            email.set("CONTENT-TRANSFER-ENCODING", "base64");
            email.set("CONTENT-DISPOSITION", "attachment; filename=\""+name+"\"");
            std::ifstream fs(file.c_str());
            std::string b;
            jlib::sys::read(fs,b);
            email.data(b);
            m_email.attach().push_back(email);
            std::string bound = jlib::util::valueOf(rand())+"gtkmail"+jlib::util::valueOf(rand());
            m_email.set("CONTENT-TYPE", "multipart/mixed; boundary=\""+bound+"\"");
            build_tree();
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
        
    }
    
    void WriteWin::complete_address(Gtk::Entry* field) {
        std::vector<std::string> rcpts = jlib::util::tokenize(field->get_text(), ",");
        std::string entry_text;
        
        if(DEBUG) std::cout << "rcpts.size() == "<<rcpts.size()<<std::endl;
        for(unsigned int i=0; i<rcpts.size(); i++) {
            rcpts[i] = jlib::util::trim(rcpts[i]);
            AddressBook::iterator j = AddressBook::global.find(rcpts[i]);
            if(j != AddressBook::global.end()) {
                rcpts[i] = j->get_full();
            }
            if(DEBUG) std::cout << "rcpts["<<i<<"] == "<<rcpts[i]<<std::endl;
        }
        
        for(unsigned int i=0; i<rcpts.size(); i++) {
            entry_text += (i ?  ", " : "") + rcpts[i];
        }
        
        field->set_text(entry_text);
    }
    
    void WriteWin::on_send(jlib::net::Email data) {
        if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
            std::cout << "gtkmail::WriteWin::on_send(): enter" << std::endl;
        
        m_box->add_sent_mail(data);
        if(m_forward.get_indx() != -1) {
            m_box->set_answered_flag(m_info, m_forward.get_indx(), m_forward["message-id"]);
        }
        
        if(getenv("GTKMAIL_WRITEWIN_DEBUG"))
            std::cout << "gtkmail::WriteWin::on_send(): leave" << std::endl;
    }
    
}
