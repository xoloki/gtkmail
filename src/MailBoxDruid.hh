/* MailBoxDruid.hh
 * 
 * Copyright (C) 2001 Joe Yandle 
 *
 */

#ifndef GTKMAIL_MAILBOX_DRUID
#define GTKMAIL_MAILBOX_DRUID

#include <string>

#include <gnome--/dialog.h>
#include <gnome--/druid.h>
#include <gnome--/druid-page-start.h>
#include <gnome--/druid-page-standard.h>
#include <gnome--/druid-page-finish.h>

#include <gtk--/entry.h>

namespace gtkmail {
    class MailBoxDruid : public Gnome::Dialog {
    public:
        MailBoxDruid();
        MailBoxDruid(std::string name,std::string address,std::string url,std::string time);
        virtual ~MailBoxDruid();
        
        void cancel_call();
        void finish_call();

        std::string get_name();
        std::string get_address();
        std::string get_url();
        std::string get_time();
        bool is_finished();
        bool is_filled();
        
        void select_start_page();
        void select_info_page();
        void select_finish_page();


    protected:
        void init_ui();

        Gnome::Druid m_druid;
        
        Gnome::DruidPageStart m_start_page;
        Gnome::DruidPageFinish m_finish_page;
        Gnome::DruidPageStandard m_info_page;

        Gtk::Entry m_name,m_address,m_url,m_time;
           
        
        bool m_finished;
    };
}

#endif //GTKMAIL_MAILBOX_DRUID
