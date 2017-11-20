/* MailBoxDruid.hh
 * 
 * Copyright (C) 2001 Joe Yandle 
 *
 */

#include "MailBoxDruid.hh"

#include <gtk--/box.h>
#include <gtk--/label.h>

namespace gtkmail {
    MailBoxDruid::MailBoxDruid() {
        m_finished = false;
        init_ui();
    }
    
    MailBoxDruid::MailBoxDruid(std::string name,std::string address,std::string url,std::string time) {
        m_finished = false;
        init_ui();
        m_name.set_text(name);
        m_address.set_text(address);
        m_url.set_text(url);
        m_time.set_text(time);
    }
    
    void MailBoxDruid::init_ui() {
        set_title("MailBox Druid");
        set_default_size(400, 400);
        
        m_start_page.set_title("MailBox Druid - Introduction");
        //m_start_page.set_text("The MailBox Druid will help you create and edit mailboxes for gtkmail. On the next page, you can enter or update the information for your mailbox.  The final page will allow you to either save the updated mailbox or cancel without saving.");
        m_start_page.set_text("The MailBox Druid will help you create and edit mailboxes for gtkmail.");
        
        m_info_page.set_title("MailBox Druid - Enter Information");
        Gtk::VBox* vbox = m_info_page.get_vbox();
        if(vbox) {
            vbox->pack_start(*manage(new Gtk::Label("",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  Enter a name for this mailbox",0)),true,true);
            vbox->pack_start(m_name,false);
            vbox->pack_start(*manage(new Gtk::Label("",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  Enter an email address for this mailbox",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  This can be in one of the following forms:",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("      user@host.net",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("      First Last <user@host.net>",0)),true,true);
            vbox->pack_start(m_address,false);
            vbox->pack_start(*manage(new Gtk::Label("",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  Enter a URL for this mailbox",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  This should be in the form:",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("      protocol://user@host/path",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  For example:",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("      simap://foo@bar.com",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("      mbox:///home/foo/mail",0)),true,true);
            vbox->pack_start(m_url,false);
            vbox->pack_start(*manage(new Gtk::Label("",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  How often do you want to check this mailbox?",0)),true,true);
            vbox->pack_start(*manage(new Gtk::Label("  Enter a period in milliseconds",0)),true,true);
            vbox->pack_start(m_time,false);
        }
        
        m_finish_page.set_title("MailBox Druid - Finished");
        m_finish_page.set_text("To save this mailbox to the config file, click 'Finish' below.");
        
        m_druid.pages().push_back(m_start_page);
        m_druid.pages().push_back(m_info_page);
        m_druid.pages().push_back(m_finish_page);
        
        m_druid.show_all();
        get_vbox()->pack_start(m_druid);
        
        m_druid.cancel.connect(slot(this, &MailBoxDruid::cancel_call));
        m_finish_page.finish.connect(slot(this, &MailBoxDruid::finish_call));
    }

    MailBoxDruid::~MailBoxDruid() {}
    
    void MailBoxDruid::cancel_call(){
        m_finished = false;
        cause_close();
    }

    void MailBoxDruid::finish_call() {
        m_finished = true;
        cause_close();
    }
    
    std::string MailBoxDruid::get_name() { return m_name.get_text(); }
    std::string MailBoxDruid::get_address() { return m_address.get_text(); }
    std::string MailBoxDruid::get_url() { return m_url.get_text(); }
    std::string MailBoxDruid::get_time() { return m_time.get_text(); }
    bool MailBoxDruid::is_finished() { return m_finished; }
    bool MailBoxDruid::is_filled() {
        return (get_name() != "" && 
                get_address() != "" && 
                get_url() != "" && 
                get_time() != "");
    }

    void MailBoxDruid::select_start_page() { m_druid.set_page(m_start_page); }
    void MailBoxDruid::select_info_page() { m_druid.set_page(m_info_page); }
    void MailBoxDruid::select_finish_page() { m_druid.set_page(m_finish_page); }


}
