/* -*- mode: C++ c-basic-offset: 4  -*-
 * MailBox.cc - source file for class MailBox, which implements
 *              an entire mailbox viewing widget
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
#include "MailBox.hh"
#include "Dialog.hh"
#include "ReadWin.hh"
#include "WriteWin.hh"
#include "SignalChain.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtkmm/image.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/style.h>
#include <gtkmm/frame.h>
#include <gtkmm/main.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>

#include <sigc++/sigc++.h>

#include <jlib/net/MailBox.hh>

#include <jlib/sys/Directory.hh>

#include <jlib/util/util.hh>
#include <jlib/util/Date.hh>

#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include <cstring>
#include <cstdio>
#include <algorithm>
#include <string>
#include <sstream>

namespace gtkmail {

    struct iterator_data {
        Gtk::TreeModel::iterator i;
    };
    
    MailBox::MailBox(jlib::net::ASMailBox* box, std::string name, int timeout, std::string email_addr, std::string url) : Gtk::VBox(false,0) {
        m_box = box;
        m_name = name;
        m_email_addr = email_addr;
        m_url = url;
        m_time = timeout;
        //m_compare_func = &clist_compare;
        //m_sort_type = GTK_SORT_ASCENDING;
        m_have_sorted = false;

        m_sortable = false;
        m_ubersortable = false;
        m_threadable = true;
        m_save_menu = 0;

        m_search_field = "subject";

        m_box->status.connect(sigc::mem_fun(*this, &MailBox::on_status));
        m_box->initialized.connect(sigc::mem_fun(*this, &MailBox::on_initialized));
        m_box->error.connect(sigc::mem_fun(*this, &MailBox::on_error));
        
        m_box->login_error.connect(sigc::mem_fun(*this, &MailBox::on_login_error));
        
        m_box->folders_loaded.connect(sigc::mem_fun(*this, &MailBox::on_folders_loaded));
        
        m_box->folder_listed.connect(sigc::mem_fun(*this, &MailBox::on_folder_listed));
        m_box->folder_created.connect(sigc::mem_fun(*this, &MailBox::on_folder_created));
        m_box->folder_deleted.connect(sigc::mem_fun(*this, &MailBox::on_folder_deleted));
        m_box->folder_expunged.connect(sigc::mem_fun(*this, &MailBox::on_folder_expunged));
        m_box->folder_renamed.connect(sigc::mem_fun(*this, &MailBox::on_folder_renamed));

        m_box->message_listed.connect(sigc::mem_fun(*this, &MailBox::on_message_listed));
        m_box->message_loaded.connect(sigc::mem_fun(*this, &MailBox::on_message_loaded));

        m_box->message_flags_set.connect(sigc::mem_fun(*this, &MailBox::on_message_flags_set));
        m_box->message_flags_unset.connect(sigc::mem_fun(*this, &MailBox::on_message_flags_unset));

        Glib::signal_timeout().connect(sigc::mem_fun(*this,&gtkmail::MailBox::on_timeout), timeout);
        Glib::signal_io().connect(sigc::mem_fun(*this,&MailBox::on_input), 
                                  m_box->get_response_reader(),
                                  Glib::IO_IN);

        init_ui();
        init_box();

        show_all();
    }

    void MailBox::init_ui() {
        std::string fontName;	
        
        m_popup = manage(new Gtk::Menu());
        m_hbox = manage(new Gtk::HBox(false,0));

        m_paned = manage(new Gtk::HPaned());
        
        Gtk::ScrolledWindow* folder_win = manage(new Gtk::ScrolledWindow());
        folder_win->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);

        //m_folder_view = Gtk::manage(new FolderView());
        m_folder_view = Gtk::manage(new Gtk::TreeView());
        m_folder_model = Gtk::TreeStore::create(m_folder_cols);
        m_folder_view->set_model(m_folder_model);

        Gtk::TreeView::Column* icol = Gtk::manage( new Gtk::TreeView::Column("Folders") ); 
        icol->pack_start(m_folder_cols.m_icon, false);
        icol->pack_start(m_folder_cols.m_name);
        
        m_folder_view->append_column(*icol);
        
        m_folder_select = m_folder_view->get_selection();
        m_folder_select->set_mode(Gtk::SELECTION_BROWSE);

        m_selected_folder = m_folder_model->children().end();

        folder_win->add(*m_folder_view);

        Gtk::ScrolledWindow* message_win = manage(new Gtk::ScrolledWindow());
        message_win->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
        
        m_message_view = Gtk::manage(new Gtk::TreeView());
        
        m_message_model = MessageModel::create(m_message_cols);
        m_message_view->set_model(m_message_model);

        // set up columns.  since i have to enable strikethrough, i have to do this nasty shite
        Gtk::CellRendererText* crt;
        Gtk::CellRendererPixbuf* crp;
        Gtk::TreeView::Column* col = nullptr;
        Gtk::Image* col_image;

        Config::iterator ci = Config::global.find(m_name);
        if(ci == Config::global.end())
            throw std::runtime_error("No config for mailbox " + m_name);
                                     
        auto cols = ci->get_message_cols();

        for(auto column : cols) {
            if(column.title == "READ") {
                // icon col
                crp = Gtk::manage(new Gtk::CellRendererPixbuf());
                col_image = Gtk::manage( new Gtk::Image(Config::global.mail_new_buf->copy()));
                col_image->show();
                
                col = Gtk::manage( new Gtk::TreeView::Column() ); 
                col->pack_start(*crp, true);
                col->set_widget(*col_image);
                col->set_cell_data_func(*crp, sigc::mem_fun(*this, &MailBox::render_icon));
                crp->property_xalign() = 0.5;
                crp->property_yalign() = 0.5;
                m_message_view->append_column(*col);
            } else if(column.title == "ATTACH") {
                // attach col
                crp = Gtk::manage(new Gtk::CellRendererPixbuf());
                col_image = Gtk::manage( new Gtk::Image(Config::global.attachment_buf->copy()));
                col_image->show();
                
                col = Gtk::manage( new Gtk::TreeView::Column() ); 
                col->pack_start(*crp, true);
                col->set_widget(*col_image);
                col->set_cell_data_func(*crp, sigc::mem_fun(*this, &MailBox::render_attach));
                crp->property_xalign() = 0.5;
                crp->property_yalign() = 0.5;
                m_message_view->append_column(*col);
            } else if(column.title == "Date") {
                // date col
                crt = Gtk::manage(new Gtk::CellRendererText());
                col = Gtk::manage(new Gtk::TreeView::Column("Date", *crt)); 
                col->set_cell_data_func(*crt, sigc::mem_fun(*this, &MailBox::render_date));
                col->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MailBox::on_column_clicked), "date"));
                m_message_view->append_column(*col);
            } else if(column.title == "Size") {
                // size col
                crt = Gtk::manage(new Gtk::CellRendererText());
                crt->property_xalign() = 1.0;
                col = Gtk::manage(new Gtk::TreeView::Column("Size", *crt)); 
                col->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MailBox::on_column_clicked), "size"));
                col->set_cell_data_func(*crt, sigc::mem_fun(*this, &MailBox::render_size));
                m_message_view->append_column(*col);
            } else if(column.title == "From") {
                // from col
                crt = Gtk::manage(new Gtk::CellRendererText());
                col = Gtk::manage(new Gtk::TreeView::Column("From")); 
                col->pack_start(*crt, false);
                col->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MailBox::on_column_clicked), "from"));
                col->set_cell_data_func(*crt, sigc::mem_fun(*this, &MailBox::render_from));
                //col->property_fixed_width() = 200;
                m_message_view->append_column(*col);
            } else if(column.title == "Subject") {
                // subject col
                crt = Gtk::manage(new Gtk::CellRendererText());
                col = Gtk::manage(new Gtk::TreeView::Column("Subject")); 
                col->pack_start(*crt, false);
                col->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MailBox::on_column_clicked), "subject"));
                col->set_cell_data_func(*crt, sigc::mem_fun(*this, &MailBox::render_subject));
                m_message_view->append_column(*col);
            }                 
        }

        if(col != nullptr)
            m_message_view->set_expander_column(*col);
        
        Glib::ListHandle<Gtk::TreeViewColumn*> list = m_message_view->get_columns();
        Glib::ListHandle<Gtk::TreeViewColumn*>::iterator i = list.begin();
        for(; i != list.end(); i++) {
            if(i != list.begin()) {
                (*i)->set_reorderable(true);
            }
            (*i)->set_resizable(true);
            (*i)->set_alignment(0.5);
        }
        
        m_message_view->set_headers_clickable();
        m_message_view->set_enable_search();
        m_message_view->set_search_equal_func(sigc::mem_fun(*this, &MailBox::on_search));

        m_message_select = m_message_view->get_selection();
        m_message_select->set_mode(Gtk::SELECTION_MULTIPLE);
        
        m_selected_message = m_message_model->children().end();
        
        message_win->add(*m_message_view);
        
        m_status = manage(new Gtk::Statusbar());
        m_mail_view = manage(new MailView(m_status));
        
        m_message_paned = manage(new Gtk::VPaned());
        
        m_message_paned->add(*message_win);
        m_message_paned->add(*m_mail_view);
        
        m_paned->add1(*folder_win);
        m_paned->add2(*m_message_paned);
        pack_start(*m_paned, true, true, 0);
        
        pack_end(*m_status, false, false, 0);

        {
            Config::iterator i = Config::global.find(m_name);
            if(i != Config::global.end()) {
                m_message_paned->set_position(i->get_message_pos());
                m_paned->set_position(i->get_folder_pos());
            }
        }

        m_message_view->set_rules_hint();
        m_folder_view->set_rules_hint();
        
        std::list<Gtk::TargetEntry> message_targets;
        std::list<Gtk::TargetEntry> folder_targets;
        std::list<Gtk::TargetEntry> all_targets;

        message_targets.push_back( Gtk::TargetEntry("MAILBOX_MESSAGE", Gtk::TargetFlags(0), TARGET_MESSAGE) );
        folder_targets.push_back( Gtk::TargetEntry("MAILBOX_FOLDER", Gtk::TargetFlags(0), TARGET_FOLDER) );

        all_targets.push_back( Gtk::TargetEntry("MAILBOX_MESSAGE", Gtk::TargetFlags(0), TARGET_MESSAGE) );
        all_targets.push_back( Gtk::TargetEntry("MAILBOX_FOLDER", Gtk::TargetFlags(0), TARGET_FOLDER) );


        m_message_view->enable_model_drag_source(message_targets, 
                                                 (Gdk::BUTTON1_MASK|Gdk::BUTTON3_MASK),
                                                 Gdk::ACTION_COPY);
        m_folder_view->enable_model_drag_source(folder_targets, 
                                                (Gdk::BUTTON1_MASK|Gdk::BUTTON3_MASK),
                                                Gdk::ACTION_COPY);
        m_folder_view->enable_model_drag_dest(all_targets, Gdk::ACTION_COPY);
                                              
        /*
        m_message_view->signal_drag_drop().connect(sigc::mem_fun(*this, &MailBox::on_message_drag_drop));
        m_message_view->signal_drag_data_get().connect(sigc::mem_fun(*this, &MailBox::on_message_drag_data_get));
        m_folder_view->signal_drag_data_received().connect(sigc::mem_fun(*this, &MailBox::on_folder_drag_data_received));
        m_folder_view->signal_drag_data_get().connect(sigc::mem_fun(*this, &MailBox::on_folder_drag_data_get));
        m_folder_view->signal_drag_drop().connect(sigc::mem_fun(*this, &MailBox::on_folder_drag_drop));

        m_folder_view->signal_row_expanded().connect(sigc::mem_fun(*this, &MailBox::on_folder_expand));
        m_folder_view->signal_row_collapsed().connect(sigc::mem_fun(*this, &MailBox::on_folder_collapse));
        */

        m_message_view->signal_button_press_event().connect(sigc::bind(sigc::mem_fun(*this, 
                                                                                     &MailBox::menu_popup), 
                                                                       m_popup));
        
        //m_message_view->signal_select_cursor_row().connect(slot(*this, 
        //                                                        &MailBox::on_message_select_cursor_row));

        //m_folder_view->signal_select_cursor_row().connect(slot(*this, 
        //                                                        &MailBox::on_folder_select_cursor_row));
        //m_message_select->signal_changed().connect(slot(*this, &MailBox::on_message_select));
        //m_message_view->signal_cursor_changed().connect(slot(*this, &MailBox::on_message_select));
                                     
        m_message_view->signal_row_activated().connect(sigc::mem_fun(*this, &MailBox::on_message_activated));
                                     
        //m_messages->click_column.connect(slot(*this, &MailBox::column_click));

        m_message_view->set_scroll_adjustments(*message_win->get_hadjustment(),
                                               *message_win->get_vadjustment());
        
        m_message_select->signal_changed().connect(sigc::mem_fun(*this, &MailBox::on_message_selection_changed));
        m_folder_select->signal_changed().connect(sigc::mem_fun(*this, &MailBox::on_folder_select));

        //m_message_paned->signal_move_handle().connect(sigc::mem_fun(*this, &MailBox::on_message_move_handle));
        //m_paned->signal_move_handle().connect(sigc::mem_fun(*this, &MailBox::on_folder_move_handle));
    }

    void MailBox::init_box() {
        set_sensitive(false);
        write_status("Initializing mailbox...");

        m_message_model->clear();
        m_folder_model->clear();

        m_message_map.clear();
        m_message_id_map.clear();
        m_folder_map.clear();

        m_box->init();
    }

    void MailBox::refresh_folders() {
        
    }

    //void MailBox::check_box();
    /*
    void MailBox::scan_box(bool scan,jlib::net::MailFolder* box) {
        if(scan) {
            set_sensitive(false);
            write_status(" New messages found in "+jlib::net::MailNode::pathstr(get_path())+", scanning...");
            gtkmail::SignalChain0<void>* chain = 
                new gtkmail::SignalChain0<void>(sigc::mem_fun(box,&jlib::net::MailFolder::scan));
            chain->succeed.connect(sigc::mem_fun(*this,static_cast<void (::gtkmail::MailBox::*)()>(&gtkmail::MailBox::list_messages)));
            chain->error.connect(sigc::bind(sigc::mem_fun(&gtkmail::display_exception),this));
            chain->fail.connect(sigc::bind(sigc::mem_fun(*this,&gtkmail::MailBox::set_sensitive),true));
            chain->start();
        }
        else {
            set_sensitive(true);
            set_message_status();
            if(!m_messages->selection().empty()) {
                //m_messages->grab_focus();
                m_messages->selection().begin()->select();
                //m_messages->selection().begin()->focus();
            }
            else {
                
            }
        }
    }
    */
    
    /*
    int MailBox::timeout() {
        if(!m_folders->selection().empty() && is_sensitive() && m_queue.empty(QUEUE_CHECK_TYPE)) {
            std::list<SignalQueue::action::row_type> buf;
            m_queue.push(SignalQueue::action(QUEUE_CHECK_TYPE,get_path(),std::list<std::string>(),buf));
        }
        
        return 1;
    }
    */

    /*
    int MailBox::sent_mail_timeout() {
        if(is_sensitive()) {
            while(!m_sent_mail_queue.empty()) {
                m_sent_mail_queue.pop();
            }
        }
        
        return 1;
    }
    */
    
    void MailBox::message_save(jlib::net::Email e, std::list<std::string> path) {
        
    }


    void MailBox::write_status(std::string status) {
        m_status->pop();
        m_status->push(status);
    }
    

    /*            
            if(i->is_folder()) {
                if(i->is_parent()) {
                    rows.push_back(Gtk::CTree_Helpers::Element(row, m_mail_loader->pix(), m_mail_loader->bit(), m_mail_loader->pix(), m_mail_loader->bit(), false, false));
                    rows.back().set_data(new_node);
                
                    Gtk::CTree_Helpers::RowList sub = rows.back().subtree();
                    build_folders(sub, *i);
                }
                else {
                    rows.push_back(Gtk::CTree_Helpers::Element(row, m_mail_loader->pix(), m_mail_loader->bit(), m_mail_loader->pix(), m_mail_loader->bit(), true,false));
                    rows.back().set_data(new_node);
                }
            }
            else if(i->is_parent()) {
                rows.push_back(Gtk::CTree_Helpers::Element(row, m_close_loader->pix(), m_close_loader->bit(), m_open_loader->pix(), m_open_loader->bit(), false, false));
                rows.back().set_data(new_node);
                Gtk::CTree_Helpers::RowList sub = rows.back().subtree();
                build_folders(sub, *i);
            }
        }
    }
    */

    /*
    void MailBox::update_messages() {
        if(!m_folders->selection().empty()) {
            int n = FOLDER_SELECT_NONE;
            Gtk::CTree_Helpers::SelectionList::iterator i = m_messages->selection().begin();
            if(i != m_messages->selection().end()) {
                n = *static_cast<u_int*>(i->get_data());
            }
            
            folder_select();
            
            if(n != FOLDER_SELECT_NONE) {
                unsigned int message = n;
                if(message >= m_messages->rows().size()) {
                    message = m_messages->rows().size()-1;
                }
                //m_messages->grab_focus();
                m_messages->row(message).select();
                //m_messages->row(message).focus();
            }

            set_sensitive(true);
        }
    }
    */

    /*
    void MailBox::build_message_threads(Gtk::CTree_Helpers::RowList& rows, 
                                        std::vector<bool>& used,
                                        u_int p, u_int& ncount, u_int& tcount) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
            std::cout << "gtkmail::MailBox::build_message_threads(rows,used,"
                      <<p<<","<<ncount<<","<<tcount<<")"<<std::endl;
        jlib::net::MailFolder* folder = m_box->get_folder(get_path());
        for(u_int j=0;j<folder->size();j++) {
            if(j != p && !used[j] && folder->at(p)["MESSAGE-ID"] != "" && 
               (folder->at(j)["IN-REPLY-TO"].find(folder->at(p)["MESSAGE-ID"]) != std::string::npos ||
                folder->at(j)["REFERENCES" ].find(folder->at(p)["MESSAGE-ID"]) != std::string::npos)) {

                if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
                    std::cout << "\tadding email "<<j<<std::endl;
                add_message(rows,j,ncount,tcount);
                used[j]=true;
                
                Gtk::CTree_Helpers::RowList sub = rows.back().subtree();
                build_message_threads(sub,used,j,ncount,tcount);
            }
        }
    }
    */

    std::string MailBox::get_flag_text(jlib::net::Email e) {
        std::string ret;
        std::set<jlib::net::Email::flag_type> flags = e.get_flags();

        if(flags.find(jlib::net::Email::answered_flag)!=flags.end()) {
            ret += "A";
        } 
        
        if(flags.find(jlib::net::Email::seen_flag)==flags.end()) {
            ret += "N";
        } 
        
        if(flags.find(jlib::net::Email::deleted_flag) != flags.end()) {
            ret += "D";
        } 

        return ret;
    }

    void MailBox::add_message(Gtk::TreeModel::iterator i, int p, jlib::net::Email data) {
        Gtk::TreeModel::Row row = *i;
        std::set<jlib::net::Email::flag_type> flags = data.get_flags();
        std::string msgid = data["message-id"];

        m_message_map[p] = i;
        if(msgid != "") {
            m_message_id_map[msgid] = i;
        }
        
        try {
            row[m_message_cols.m_data]    = data;
            row[m_message_cols.m_indx]    = p;
            row[m_message_cols.m_load]    = false;

            if(flags.find(jlib::net::Email::seen_flag) == flags.end() && !get_selected_messages().size()) {
                m_message_select->select(row);
                m_message_view->scroll_to_row(m_message_model->get_path(i), 0.5);
            }
            
        } catch(Glib::Exception& e) {
            std::cerr << "gtkmail::MailBox::add_message: caught Glib::Exception: " << e.what() << std::endl;
        } catch(std::exception& e) {
            std::cerr << "gtkmail::MailBox::add_message: caught std::exception: " << e.what() << std::endl;
        } catch(...) {
            std::cerr << "gtkmail::MailBox::add_message: caught unknown exception" << std::endl;
        }
    }
    
    /*
    void MailBox::list_messages() {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
            std::cout << "gtkmail::MailBox::list_messages()"<<std::endl;
        //m_messages->freeze();
        m_messages->clear();
        bool found_new = false;
        unsigned int select_msg = 0;
        u_int ncount=0, tcount=0, j=0;
        jlib::net::MailFolder* folder = m_box->get_folder(get_path());
        m_have_new_message = false;
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "\t"<<"folder = "<<folder<<std::endl;
        if(folder != 0) {
            if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
                std::cout << "\t"<<"folder->size() = "<<folder->size()<<std::endl;

            if(is_sortable()) {
                std::multimap<jlib::net::Email,u_int> sorted;
                for(unsigned int i=0;i<folder->size();i++) {
                    if(!folder->at(i).internal()) {
                        jlib::net::Email buf(folder->at(i));
                        buf.sort(m_sort_field);
                        sorted.insert(std::make_pair(buf,i));
                    }
                }
         
                if(m_sort_type == GTK_SORT_ASCENDING) {
                    std::multimap<jlib::net::Email,u_int>::iterator i = sorted.begin();
                    Gtk::CTree_Helpers::RowList rows = m_messages->rows();
                    for(;i!=sorted.end();i++) {
                        add_message(rows,i->second,ncount,tcount);
                    }
                }
                else {
                    std::multimap<jlib::net::Email,u_int>::reverse_iterator i = sorted.rbegin();
                    Gtk::CTree_Helpers::RowList rows = m_messages->rows();
                    for(;i!=sorted.rend();i++) {
                        add_message(rows,i->second,ncount,tcount);
                    }
                }
            }
            else if(is_threadable()) {
                std::vector<bool> used(folder->size());
                for(u_int i=0;i<used.size();i++)
                    used[i]=false;
                Gtk::CTree_Helpers::RowList rows = m_messages->rows();
                for(unsigned int i=0;i<folder->size();i++) {
                    if(!folder->at(i).internal() && !used[i]) {
                        if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
                            std::cout << "\tadding email "<<i<<std::endl;
                        
                        used[i]=true;
                        add_message(rows,i,ncount,tcount);
                        Gtk::CTree_Helpers::RowList sub = rows.back().subtree();
                        build_message_threads(sub,used,i,ncount,tcount);
                    }
                    
                }
            }
            else {
                Gtk::CTree_Helpers::RowList rows = m_messages->rows();
                for(unsigned int i=0;i<folder->size();i++) {
                    if(!folder->at(i).internal()) {
                        if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
                            std::cout << "\tadding email "<<i<<std::endl;
                        add_message(rows,i,ncount,tcount);
                    }
                }
            }

        }
        set_message_status();
        //m_messages->thaw();
        Gtk::Window* top = get_toplevel();
        top->set_title(std::string(PACKAGE_NAME)+": "+m_name+" => "+get_pathstr());
        //m_messages->grab_focus();

        if(m_have_new_message) {
            m_new_message_row.select();
        }
        else if(tcount > 0) {
            m_messages->row(0).select();
            //m_messages->row(select_msg).focus();
        }
        
        std::ostringstream nstream, tstream;
        nstream << ncount; tstream << tcount;
        std::list<std::string> folder_sizes;
        folder_sizes.push_back(nstream.str());
        folder_sizes.push_back(tstream.str());
        set_folder_sizes(folder_sizes);
        
        m_queue.flush();
        //m_box_locks[m_box->get_folder(get_path())].unlock();
        
        set_sensitive(true);
    }
    */
    
    void MailBox::folder_call(std::string s) {
        if(s == "New") {
            folder_new_call();
        }
        else if(s == "Delete") {
            folder_delete_call();
        }
        else if(s == "Rename") {
            folder_rename_call();
        }
    }
    
    void MailBox::message_call(std::string s) {
        if(s == "Delete") {
            message_delete_call();
        }
        else if(s == "View") {
            message_view_call();
        }
        else if(s == "Reply") {
            message_reply_call();
        }
        else if(s == "Reply All") {
            message_reply_all_call();
        }
        else if(s == "Compose") {
            message_compose_call();
        }
        //         else if(s == "Save") {
        //             messageSaveCallback();
        //         }
    }
    
    /*
    void MailBox::message_select(gint row, gint column, GdkEvent* button) {
        write_status(message_status(row));
        
        if(button && button->type == GDK_2BUTTON_PRESS) {
            message_view_call();
        }
    }
    */
    
    /*
    void MailBox::message_deselect(gint row, gint column, GdkEvent* button) {
        //if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "message_deselect("<<row<<", "<<column<<", "<<button<<")\n";
        //messageSelect.erase(find(messageSelect.begin(), messageSelect.end(), row));
    }
    */
    
    //void MailBox::folder_select(gint row, gint column, GdkEvent* button) {
    /*
    void MailBox::folder_select_row(Gtk::CTree_Helpers::Row row, gint col) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "gtkmail::MailBox::folder_select_row()"<<std::endl;
        folder_select();
    }
    void MailBox::folder_select() {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "gtkmail::MailBox::folder_select()"<<std::endl;
        
        if(!is_ubersortable())
            m_sortable = false;

        m_sort_type = GTK_SORT_ASCENDING;
        m_have_sorted = false;

        if(m_folders->selection().empty()) {
            if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "\tm_folders->selection().empty()"<<std::endl;
            return;
        }

        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "\t!m_folders->selection().empty()"<<std::endl;

        std::string path = this->get_pathstr();
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "\tthis->get_pathstr() = "<<path << std::endl;
        
        try {
            if(!get_node()->is_folder()) {
                m_messages->clear();
                return;
            }
            jlib::net::MailFolder* box = m_box->get_folder(get_path());
            write_status(" Scanning "+path);
            set_sensitive(false);
            
            gtkmail::SignalChain0<void>* chain = 
                new gtkmail::SignalChain0<void>(sigc::mem_fun(box,&jlib::net::MailFolder::refresh));
            //chain->succeed.connect(sigc::mem_fun(*this,static_cast<void (::gtkmail::MailBox::*)()>(&gtkmail::MailBox::list_messages)));
            chain->succeed.connect(sigc::mem_fun(*this,&gtkmail::MailBox::list_messages));
            chain->fail.connect(sigc::bind(sigc::mem_fun(*this,&gtkmail::MailBox::set_sensitive),true));
            chain->error.connect(sigc::bind(sigc::mem_fun(&gtkmail::display_exception),this));
            chain->start();
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
        
    }
    */
    
    //     void MailBox::folder_deselect(gint row, gint column, GdkEvent* button) {
    //         m_folder_select = FOLDER_SELECT_NONE;
    //         m_dir_select = FOLDER_SELECT_NONE;
    //         m_messages->freeze();
    //         m_messages->clear();
    //         m_messages->thaw();
    //     }
    
    void MailBox::folder_new_call() {
        EntryDialog entry("Enter a name for your new folder");
        if(entry.run() == Gtk::RESPONSE_OK) {
            jlib::net::folder_info_type info;
            info.path = jlib::util::tokenize_list(entry.get_text());
            info.attr.is_parent = false;
            info.attr.is_select = true;
            m_box->create_folder(info);
        }
    }
    
    void MailBox::folder_delete_call() {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(!i) {
            return;
        }
        jlib::net::folder_info_type info = (*i)[m_folder_cols.m_info];
        m_box->delete_folder(info);
    }
    
    /*
    void MailBox::folder_expunge_exec(jlib::net::MailFolder* box) {
        box->expunge();
        box->scan();
    }
    */

    void MailBox::folder_expunge_call() {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(!i) {
            return;
        }
        
        jlib::net::folder_info_type info = (*i)[m_folder_cols.m_info];
        m_box->expunge_folder(info);
    }
    
    void MailBox::folder_rename_call() {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(!i) {
            return;
        }

        jlib::net::folder_info_type src = (*i)[m_folder_cols.m_info];
        
        EntryDialog entry("Enter a new path for folder", jlib::net::MailNode::pathstr(src.path));
        if(entry.run() == Gtk::RESPONSE_OK) {
            
            jlib::net::folder_info_type dst = src;
            dst.path = jlib::util::tokenize_list(entry.get_text());

            write_status(" Renaming folder to "+entry.get_text());

            m_box->rename_folder(src, dst);
        }

    }

    void MailBox::message_undelete_call() {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "gtkmail::MailBox::message_undelete_call()"<< std::endl;
        Gtk::TreeModel::iterator j = m_folder_select->get_selected();
        if(!j) return;

        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        jlib::net::folder_indx_type indx; 
        for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
            Gtk::TreeModel::Row row = **i;
            indx.insert(indx.begin(), row[m_message_cols.m_indx]);
        }

        Gtk::TreeModel::Row frow = *j;

        std::set<jlib::net::Email::flag_type> flags;
        flags.insert(flags.begin(), jlib::net::Email::deleted_flag);

        jlib::net::Email buf;
        buf.set_flags(flags);
        
        m_box->unset_message_flags(frow[m_folder_cols.m_info], indx, buf);
    }    

    void MailBox::message_view_thread_call() {
        /*
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "gtkmail::MailBox::message_delete_thread_call()"<< std::endl;
        Gtk::CTree_Helpers::SelectionList slist = m_messages->selection();
        if(!slist.empty() && is_sensitive()) {
            slist.front().select_recursive();
            message_view_call();
        } 
        */
    }

    void MailBox::message_delete_thread_call() {
        /*
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "gtkmail::MailBox::message_delete_thread_call()"<< std::endl;
        Gtk::CTree_Helpers::SelectionList slist = m_messages->selection();
        if(!slist.empty() && is_sensitive()) {
            slist.front().select_recursive();
            message_delete_call();
        }        
        */
    }


    void MailBox::message_delete_call() {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) std::cout << "gtkmail::MailBox::message_delete_call()"<< std::endl;
        
        Gtk::TreeModel::iterator j = m_folder_select->get_selected();
        if(!j) return;

        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        jlib::net::folder_indx_type indx; 
        for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
            Gtk::TreeModel::Row row = **i;
            indx.insert(indx.begin(), row[m_message_cols.m_indx]);
        }

        Gtk::TreeModel::Row frow = *j;

        std::set<jlib::net::Email::flag_type> flags;
        flags.insert(flags.begin(), jlib::net::Email::deleted_flag);

        jlib::net::Email buf;
        buf.set_flags(flags);
        
        m_box->set_message_flags(frow[m_folder_cols.m_info], indx, buf);
    }    
    
    void MailBox::message_compose_call() {
        WriteWin* write; 
        write = new WriteWin(jlib::net::Email(),this);
        //false,false,false,false,false,false,m_email_addr);
    }

    void MailBox::message_view_call() {
        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        jlib::net::folder_indx_type indx; 
        for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
            Gtk::TreeModel::Row row = **i;
            ReadWin* w = new ReadWin(row[m_message_cols.m_data], this);
        }
    }

    void MailBox::message_set_delete(std::list<u_int> d) {
        /*
        m_messages->freeze();
        for(std::list<u_int>::iterator i=d.begin();i!=d.end();i++) {
            m_messages->cell(*i,DELETED_FLAG_COLUMN).set_text("X");
        }
        m_messages->thaw();
        */
    }

    void MailBox::message_save_call(std::string s) {
        /*
        if(s == "") throw jlib::net::MailBox::exception("null mailbox at MailBox::message_save_to_folder()");
        try {
            std::list<std::string> dst = jlib::util::tokenize_list(s);

            set_flags(get_selected_message_logical_rows(),NEW_FLAG_COLUMN, ACTION_WAITING);
            m_queue.push(SignalQueue::action(QUEUE_SAVE_TYPE,get_path(),dst,get_selected_message_rows()));
        }
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
        */
    }
    
        /*
    void MailBox::message_save_exec(std::string s,std::list<u_int> d) {
        std::vector<jlib::net::Email> msg;
        jlib::net::MailFolder* box = m_box->get_folder(jlib::util::tokenize_list(s));
            
        if(box == 0) {
            throw jlib::net::MailBox::exception("no matching folder "+s+" in gtkmail::MailBox::message_save_exec");
        }

        for(std::list<u_int>::iterator i=d.begin();i!=d.end();i++) {
            msg.push_back(m_box->get_folder(get_path())->get(*i));
        }

        box->add(msg);
    }
        */

    void MailBox::message_reply_call() {
        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        jlib::net::folder_indx_type indx; 
        for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
            Gtk::TreeModel::Row row = **i;
            Gtk::TreeModel::Row frow = *m_selected_folder;
            WriteWin* r = new WriteWin(row[m_message_cols.m_data], this, frow[m_folder_cols.m_info], true, false, true);
        }
    }

    void MailBox::message_forward_call(bool in) {
        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        jlib::net::folder_indx_type indx; 
        for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
            Gtk::TreeModel::Row row = **i;
            Gtk::TreeModel::Row frow = *m_selected_folder;
            WriteWin* r = new WriteWin(row[m_message_cols.m_data], this, frow[m_folder_cols.m_info], 
                                       false, false, false, true, (!in));
        }
    }

    /*
    void MailBox::message_reply_exec(jlib::net::Email e) {
        WriteWin* temp; temp = new WriteWin(e,this,true,false,true,false,false,false);
    }
    */

    void MailBox::message_reply_all_call() {
        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        jlib::net::folder_indx_type indx; 
        for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
            Gtk::TreeModel::Row row = **i;
            Gtk::TreeModel::Row frow = *m_selected_folder;
            WriteWin* r = new WriteWin(row[m_message_cols.m_data], this, frow[m_folder_cols.m_info], true, true, true);
        }
    }

    /*
    void MailBox::message_reply_all_exec(jlib::net::Email e) {
        WriteWin* temp; temp = new WriteWin(e,this,true,true,true,false,false,false);
    }
    */

    /*
    void MailBox::fill_email(unsigned int i) { m_email_buf = m_box->get_folder(get_path())->get(i); }
    */
    void MailBox::show_email() { ReadWin* temp; temp = new ReadWin(m_email_buf,this); }

    /*
    void MailBox::column_click(gint column) {
        if(m_folders->selection().empty() || !get_node()->is_folder())
            return;

        if(m_have_sorted) {
            if(m_sort_type == GTK_SORT_ASCENDING) {
                m_sort_type = GTK_SORT_DESCENDING;
            }
            else {
                m_sort_type = GTK_SORT_ASCENDING;
            }
        }
        else {
            m_have_sorted = true;
        }
        

        m_sortable = true;
        std::string field = "DATE";
        if(column == SIZE_COLUMN) {
            m_sort_field = "SIZE";
        }
        else if(column == DATE_COLUMN) {
            m_sort_field = "DATE";
        }
        else if(column == FROM_COLUMN) {
            m_sort_field = "FROM";
        }
        else if(column == SUBJECT_COLUMN) {
            m_sort_field = "SUBJECT";
        }

        list_messages();
        
        // old style (in jlib) sorting
        //m_box->get_folder(get_path())->sort_field(field);
        //m_box->get_folder(get_path())->sort();
        //int tmp = m_folder_select;
        //folder_deselect(tmp, BOGON_COLUMN, NULL);
        //folder_select();
}
        */
    
    void MailBox::create_popup(Gtk::Menu* menu) {
        menu->items().clear();

        m_save_menu = manage(new Gtk::Menu());
        /*
        build_popup(m_save_menu, m_box->get_root());
        
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem(std::string("Save To")));
        menu->items().back()->set_submenu(*m_save_menu);
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("View", "<control>V", bind(sigc::mem_fun(*this, &MailBox::message_call), std::string("View"))));
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Reply", "<control>R", bind(sigc::mem_fun(*this, &MailBox::message_call), std::string("Reply"))));
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Reply All", bind(sigc::mem_fun(*this, &MailBox::message_call), std::string("Reply All"))));
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Delete", "<control>D", bind(sigc::mem_fun(*this, &MailBox::message_call), std::string("Delete"))));
        menu->items().push_back(Gtk::Menu_Helpers::MenuElem("Expunge", "<control>P", sigc::mem_fun(*this, &MailBox::folder_expunge_call)));
        
        //menu->popup(3,0);
        */
    }
    
    
    void MailBox::popup() {
        /*
        if(m_messages->selection().size() > 0) {
            m_popup->popup(3,0);
        }
        */
    }
    
    bool MailBox::menu_popup(GdkEventButton* event,Gtk::Menu* menu) {
        /*
        if(event && event->button == 3 && m_messages->selection().size() > 0) {
            m_popup->popup(3,0);
        }
        */
        return true;
    }
    
    void MailBox::key_popup(Gtk::Menu* menu) {
        create_popup(menu);
    }
    
    /*
    void MailBox::build_popup(Gtk::Menu* menu, jlib::net::MailNode& node) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
            std::cerr << "enter gtkmail::MailBox::build_popup("<<menu<<","<<(&node)<<")"<<std::endl;
        using namespace Gtk::Menu_Helpers;

        if(node.size() == 0) {
            if(getenv("GTKMAIL_MAILBOX_DEBUG")) {
                std::cerr << "\tnode.size() == 0"<<std::endl;
                std::cerr << "leave gtkmail::MailBox::build_popup("<<menu<<","<<(&node)<<")"<<std::endl;
            }
            return;
        }

        if(node.begin()->get_name() == "INBOX") {
            std::sort(node.begin()+1,node.end());
        }
        else {
            std::sort(node.begin(),node.end());
        }

        for(jlib::net::MailNode::iterator i=node.begin(); i != node.end(); i++) {
            std::string file = i->get_name();
            std::string path = jlib::net::MailNode::pathstr(i->get_path());

            if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
                std::cerr << "file = '"<<file<<"', path = '"<<path<<"'"<<std::endl;
            if(i->is_folder()) {
                menu->items().push_back(MenuElem(file, bind(sigc::mem_fun(*this, &MailBox::message_save_call), path)));
            }
            if(i->is_parent()) {
                Gtk::Menu* sub = manage(new Gtk::Menu());
                menu->items().push_back(MenuElem(file));
                menu->items().back()->set_submenu(*sub);
                build_popup(sub, *i);
            }
        }
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) 
            std::cerr << "leave gtkmail::MailBox::build_popup("<<menu<<","<<(&node)<<")"<<std::endl;
    }
    */
    
    std::string MailBox::checking_message() {
        return std::string(" Checking messages...");
    }
    
    std::string MailBox::message_status(int sel) {
        /*
        if(m_folders->selection().empty()) return std::string();
        
        std::ostringstream buf;
        int count = 0;
        for(unsigned int i=0;i<m_box->get_folder(get_path())->size();i++) {
            count += m_box->get_folder(get_path())->get_size(i);
        }

        if(sel >= 0) {
            buf << " " << sel+1 << ":";
        }
        else {
            buf << " ";
        }

        buf << m_messages->rows().size() << " messages; ";

        if(sel >= 0) {
            buf << m_box->get_folder(get_path())->get_size(sel) << ":";
        }        

        buf << count << " bytes" << std::ends;
        
        std::string ret = buf.str();
        */
        return "";
    }
    
    void MailBox::set_message_status(int sel) {
        write_status(message_status(sel));
    }
    
    std::list<std::string> MailBox::get_path() {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(!i) {
            return std::list<std::string>();
        }
        jlib::net::folder_info_type info = (*i)[m_folder_cols.m_info];
        
        return info.path;
    }
    
    std::string MailBox::get_pathstr() {
        return jlib::net::MailNode::pathstr(get_path());
    }
    
    std::string MailBox::slash_path() {
        return (get_pathstr()+"/");
    }

    void MailBox::set_sensitive(bool b) {
        m_folder_view->set_sensitive(b);
        m_message_view->set_sensitive(b);
    }
    
    bool MailBox::is_sensitive() {
        return (m_folder_view->is_sensitive() && m_message_view->is_sensitive());
    }

    /*
    void MailBox::set_folder_row(std::list<std::string> s, Gtk::CTree_Helpers::Row& row, bool begin) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "gtkmail::MailBox::set_folder_row()" << std::endl;
        // i list start point, j col start point, k number to copy
        u_int i=0,j=0,k=0;
        if(s.size() >= row.size()) {
            i = 0;
            j = 0;
            k = row.size();
        }
        else if(begin) {
            i = 0;
            j = 0;
            k = s.size();
        }
        else {
            i = 0;
            j = row.size()-s.size();
            k = s.size();
        }
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "\ti="<<i<<"; j="<<j<<"; k="<<k<<";" << std::endl;
        std::list<std::string>::iterator t = s.begin();
        for(u_int l=0;l<k;l++) {
            std::string str = *t; t++;
            set_folder_cell(str, row, j+l);
        }
    }

    void MailBox::set_folder_cell(std::string s, Gtk::CTree_Helpers::Row& row, u_int col) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "gtkmail::MailBox::set_folder_cell(\""<<s<<"\",row,"<<col<<")" << std::endl;
        Gtk::CTree_Helpers::CellIterator i = row.begin();
        u_int j = 0;
        for(; j != col && i != row.end(); i++) {
            j++;
            if(getenv("GTKMAIL_MAILBOX_DEBUG"))
                std::cout << "\tj++" << std::endl;
        }
        if(j == col) {
            if(getenv("GTKMAIL_MAILBOX_DEBUG"))
                std::cout << "\ti->set_text()" << std::endl;

            i->set_text(s);
        }
        else {
            if(getenv("GTKMAIL_MAILBOX_DEBUG"))
                std::cout << "error: j != col ("<<j<<" != "<<col<<")" << std::endl;
            
        }
    }

    void MailBox::set_folder_sizes(std::list<std::string> s) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "gtkmail::MailBox::set_folder_sizes()" << std::endl;
        
        if(m_folders->selection().empty()) {
            if(getenv("GTKMAIL_MAILBOX_DEBUG"))
                std::cout << "\tm_folders->selection().empty()" << std::endl;
            return;
        }

        if(s.size() != 2) {
            if(getenv("GTKMAIL_MAILBOX_DEBUG")) {
                std::cout << "\terror: incorrect list size: " << s.size() << std::endl;
            }
            return;
        }
        
        set_folder_row(s,*m_folders->selection().begin(), false);
    }
    */
    
    std::string MailBox::get_name() {
        return m_name;
    }

    std::string MailBox::get_address() {
        return m_email_addr;
    }

    std::string MailBox::get_url() {
        return m_url;
    }

    int MailBox::get_time() {
        return m_time;
    }

    int MailBox::get_sort_column() {
        return m_sort_column;
    }
    
    bool MailBox::is_sortable() {
        return m_sortable;
    }

    bool MailBox::is_ubersortable() {
        return m_ubersortable;
    }

    bool MailBox::is_threadable() {
        return m_threadable;
    }

    Gtk::Menu* MailBox::get_save_menu() { 
        return m_save_menu; 
    }

    void MailBox::add_sent_mail(jlib::net::Email e) {
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "gtkmail::MailBox::add_sent_mail()" << std::endl;

        jlib::net::folder_info_type info;
        info.path.push_back("Sent");

        m_box->append_message(info, e);
    }

    jlib::net::folder_info_type MailBox::get_selected_folder_info() {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(i) {
            Gtk::TreeModel::Row row = *i;
            return row[m_folder_cols.m_info];
        } else {
            return jlib::net::folder_info_type();
        }
    }

    bool MailBox::on_timeout() {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(i) {
            Gtk::TreeModel::Row row = *i;
            m_box->check_recent(row[m_folder_cols.m_info]);
        } 

        return true;
    }


    bool MailBox::on_input(Glib::IOCondition c) {
        jlib::sys::ASServent<jlib::net::MailBoxRequest,jlib::net::MailBoxResponse>* servent = 
            dynamic_cast<jlib::sys::ASServent<jlib::net::MailBoxRequest,jlib::net::MailBoxResponse>*>(m_box);

        if(servent) {
            servent->handle();
        }

        return true;
    }

    void MailBox::on_error(std::string text) {
        display_exception(text,this);
        write_status(text);
    }

    void MailBox::on_login_error() {
        PassDialog pass("Error logging in.  Please re-enter your password.");
        Gtk::Window* win = dynamic_cast<Gtk::Window*>(this->get_toplevel());
        if(win) {
            pass.set_transient_for(*win);
        }
        while(pass.run() != Gtk::RESPONSE_OK) {
            Gtk::MessageDialog dialog("You need to enter a password");
            dialog.run();
        }
        m_box->set_password(pass.get_text());
       
        init_box();
    }

    void MailBox::on_initialized() {
        m_box->list_folders();
    }

    void MailBox::on_status(std::string text) {
        write_status(text);
    }


    void MailBox::on_message_selection_changed() {
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "gtkmail::MailBox::on_message_selection_changed()" << std::endl;
        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();

        if(s.size() != 1) {
            // do nothing
            return;
        } 

        Gtk::TreeModel::iterator i = s.front();
        Gtk::TreeModel::iterator j = m_folder_select->get_selected();
        if(i && j && j == m_selected_folder) {
            m_selected_message = i;
            Gtk::TreeModel::Row row = *i;
            Gtk::TreeModel::Row frow = *j;

            if(row[m_message_cols.m_load]) {
                on_message_loaded(frow[m_folder_cols.m_info], 
                                  row[m_message_cols.m_indx], 
                                  row[m_message_cols.m_data]);
            }
            else {
                jlib::net::folder_indx_type indx;
                indx.push_back(row[m_message_cols.m_indx]);
                m_box->load_messages(frow[m_folder_cols.m_info], indx);
            }
        }
    }

    bool MailBox::on_message_select_cursor_row(bool edit) {
        std::cout << "MailBox::on_message_select_cursor_row(" << edit << ")" << std::endl;
        return true;
    }

    void MailBox::on_message_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* col) {
        /*
        std::cout << "MailBox::on_message_activated" << std::endl;
        Gtk::TreeModel::iterator i = m_message_model->get_iter(path);
        if(i) {
            Gtk::TreeModel::Row row(*i);
            int indx = row[m_message_cols.m_indx];
            m_messages_activated.insert(m_messages_activated.begin(), indx);
            on_message_selection_changed();
        }
        */
        message_view_call();
    }

    bool MailBox::on_folder_select_cursor_row(bool edit) {
        std::cout << "MailBox::on_folder_select_cursor_row(" << edit << ")" << std::endl;
        return true;
    }

    void MailBox::on_folder_select() {
        if(getenv("GTKMAIL_MAILBOX_DEBUG"))
            std::cout << "gtkmail::MailBox::on_folder_select()" << std::endl;
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(i && i != m_selected_folder) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::folder_info_type info = row[m_folder_cols.m_info];
            if(!info.attr.is_select) {
                if(m_folder_view->row_expanded(Gtk::TreePath(i))) {
                    m_folder_view->collapse_row(Gtk::TreePath(i));
                } else {
                    m_folder_view->expand_row(Gtk::TreePath(i), false);
                }
            } 
            else {
                m_selected_folder = i;
                m_message_model->clear();
                m_message_map.clear();
                m_message_id_map.clear();
                m_mail_view->clear();

                m_box->list_messages(row[m_folder_cols.m_info]);
            }
        }
    }

    void MailBox::on_folders_loaded(jlib::net::ASMailBox::folder_map_type map) {
        set_sensitive(true);
        write_status("Folder information loaded successfully");
        m_folder_select->select(Gtk::TreePath("0"));
    }

    void MailBox::on_folder_listed(jlib::net::folder_info_type info) {
        add_folder(info);
    }

    void MailBox::on_folder_created(jlib::net::folder_info_type info) {
        add_folder(info);
    }

    void MailBox::on_folder_deleted(jlib::net::folder_info_type info) {
        if(m_folder_map.find(info.path) != m_folder_map.end()) {
            m_folder_model->erase(*m_folder_map.find(info.path)->second);
            m_folder_map.erase(m_folder_map.find(info.path));
        }
    }

    void MailBox::on_folder_expunged(jlib::net::folder_info_type info) {
        Gtk::TreeModel::iterator i = m_folder_select->get_selected();
        if(i) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::folder_info_type sinfo = row[m_folder_cols.m_info];
            if(sinfo.path == info.path) {
                m_message_model->clear();
                m_message_map.clear();
                m_message_id_map.clear();
                m_mail_view->clear();

                m_box->list_messages(row[m_folder_cols.m_info]);
             }
        }
    }

    void MailBox::on_folder_renamed(jlib::net::folder_info_type src, 
                                    jlib::net::folder_info_type dst) {

        if(m_folder_map.find(src.path) != m_folder_map.end()) {
            Gtk::TreeModel::iterator i = *m_folder_map.find(src.path)->second;
            Gtk::TreeModel::Row row = *i;
            
            jlib::net::folder_path_type spath = src.path;
            jlib::net::folder_path_type dpath = dst.path;
            
            spath.pop_back();
            dpath.pop_back();

            /*
             * if the only difference is the last path element, just rename the existing
             * node, ow remove then add
             */
            if(spath == dpath) {
                row[m_folder_cols.m_name] = dst.path.back();
                row[m_folder_cols.m_info] = dst;
            } else {
                on_folder_deleted(src);
                add_folder(dst);
            }
        }
        
    }
    
    void MailBox::on_message_listed(jlib::net::folder_info_type info, int i, jlib::net::Email data) {
        if(m_message_map.find(i) == m_message_map.end() && get_selected_folder_info() == info) {
            Gtk::TreeModel::iterator j = insert(data);
            add_message(j, i, data);
        }
    }

    void MailBox::on_message_loaded(jlib::net::folder_info_type info, int i, jlib::net::Email data) {
        Gtk::TreeModel::iterator imsg = m_message_map[i];
        if(imsg) {
            Gtk::TreeModel::Row row = *imsg;
            
            row[m_message_cols.m_data] = data;
            row[m_message_cols.m_load] = true;
        }

        if(!is_message_selected(i)) {
            return;
        } 

        std::set<int>::iterator j = m_messages_activated.find(i);
        if(j != m_messages_activated.end()) {
            ReadWin* read = new ReadWin(data, this);
            read->show_all();
            m_messages_activated.erase(j);
        } else {
            try {
                m_mail_view->set_data(data);
            } catch(Glib::Exception& e) {
                Gtk::MessageDialog d(e.what(), Gtk::MESSAGE_ERROR);
                d.run();
            } catch(std::exception& e) {
                Gtk::MessageDialog d(e.what(), Gtk::MESSAGE_ERROR);
                d.run();
            }

        }            
    }

    void MailBox::on_message_flags_set(jlib::net::folder_info_type info, 
                                       jlib::net::folder_indx_type indx, 
                                       jlib::net::Email data)
    {
        for(jlib::net::folder_indx_type::iterator i = indx.begin(); i != indx.end(); i++) {
            if(m_message_map.find(*i) != m_message_map.end()) {
                Gtk::TreeModel::iterator j = m_message_map[*i];
                Gtk::TreeModel::Row row = *j;
                jlib::net::Email rdata = row[m_message_cols.m_data];
                std::set<jlib::net::Email::flag_type> dflags = data.get_flags();

                rdata.set_flags(dflags);
                row[m_message_cols.m_data] = rdata;
            }
        }
    }

    void MailBox::on_message_flags_unset(jlib::net::folder_info_type info, 
                                         jlib::net::folder_indx_type indx, 
                                         jlib::net::Email data)
    {
        for(jlib::net::folder_indx_type::iterator i = indx.begin(); i != indx.end(); i++) {
            if(m_message_map.find(*i) != m_message_map.end()) {
                Gtk::TreeModel::iterator j = m_message_map[*i];
                Gtk::TreeModel::Row row = *j;
                jlib::net::Email rdata = row[m_message_cols.m_data];
                std::set<jlib::net::Email::flag_type> dflags = data.get_flags();

                rdata.unset_flags(dflags);

                row[m_message_cols.m_data] = rdata;
            }
        }
    }


    Gtk::TreeModel::iterator MailBox::add_folder_to_parent(jlib::net::folder_info_type info,
                                                           Gtk::TreeModel::Children& kids) {
        Gtk::TreeModel::iterator i = kids.begin();
        Gtk::TreeModel::iterator j = kids.end();
        for(; i; i++) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::folder_info_type row_info = row[m_folder_cols.m_info];
            if(row_info.path == info.path) {
                return i;
            }
        }
        
        i = kids.begin();
        for(; i; i++) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::folder_info_type row_info = row[m_folder_cols.m_info];
            std::string back = info.path.back();
            std::string rback = row_info.path.back();

            if(info.path.size() == 1 && back == "INBOX") {
                j = i;
                break;
            }

            if(row_info.path.size() == 1 && rback == "INBOX") {
                continue;
            }

            if(rback > back) {
                j = i;
                break;
            }
        }
        
        
        if(j != kids.end()) {
            i = m_folder_model->insert(j);
        } else {
            i = m_folder_model->append(kids);
        }
        Gtk::TreeModel::Row row = *i;
        
        if(info.attr.is_select) {
            row[m_folder_cols.m_icon] = Config::global.text_buf->copy();
        } else {
            row[m_folder_cols.m_icon] = Config::global.dir_close_buf->copy();
        }
        row[m_folder_cols.m_name] = info.path.back();
        row[m_folder_cols.m_info] = info;

        m_folder_map[info.path] = i;

        return i;
    }


    Gtk::TreeModel::iterator MailBox::add_folder(jlib::net::folder_info_type info) {
        if(info.path.size() == 0) {
            return m_folder_model->children().end();
        }
        else if(info.path.size() == 1) {
            Gtk::TreeModel::Children kids = m_folder_model->children();
            return add_folder_to_parent(info, kids);
        } else {
            jlib::net::folder_info_type sub = info;
            sub.path.pop_back();

            Gtk::TreeModel::iterator i = add_folder(sub);
            Gtk::TreeModel::Row row = *i;
            Gtk::TreeModel::Children kids = row.children();
            
            return add_folder_to_parent(info, kids);
        }
    }

    void MailBox::display_images() {
        m_mail_view->display_images();
    }
    
    void MailBox::reset() {
        set_sensitive(false);
        write_status("Resetting mailbox...");

        m_message_model->clear();
        m_folder_model->clear();
        m_mail_view->clear();

        m_message_map.clear();
        m_message_id_map.clear();
        m_folder_map.clear();

        m_folder_select = m_folder_view->get_selection();
        m_folder_select->set_mode(Gtk::SELECTION_BROWSE);

        m_selected_folder = m_folder_model->children().end();

        m_box->reinit();
    }


    bool MailBox::on_folder_move_handle(Gtk::ScrollType stype) {
        std::cout << "MailBox::on_folder_move_handle("<<stype<<")"<<std::endl;
        if(stype == Gtk::SCROLL_END) {
            Config::iterator i = Config::global.find(m_name);
            if(i != Config::global.end()) {
                i->set_folder_pos(m_paned->get_position());
            }
        }
        return true;
    }

    bool MailBox::on_message_move_handle(Gtk::ScrollType stype) {
        if(stype == Gtk::SCROLL_END) {
            Config::iterator i = Config::global.find(m_name);
            if(i != Config::global.end()) {
                i->set_message_pos(m_message_paned->get_position());
            }
        }
    }


    void MailBox::on_folder_expand(const Gtk::TreeModel::iterator& i, const Gtk::TreeModel::Path& path) {
        Gtk::TreeModel::Row row = *i;
        jlib::net::folder_info_type folder = row[m_folder_cols.m_info];
        if(folder.attr.is_select) {
            row[m_folder_cols.m_icon] = Config::global.text_buf->copy();
        } else {
            row[m_folder_cols.m_icon] = Config::global.dir_open_buf->copy();
        }
    }

    void MailBox::on_folder_collapse(const Gtk::TreeModel::iterator& i, const Gtk::TreeModel::Path& path) {
        Gtk::TreeModel::Row row = *i;
        jlib::net::folder_info_type folder = row[m_folder_cols.m_info];
        if(folder.attr.is_select) {
            row[m_folder_cols.m_icon] = Config::global.text_buf->copy();
        } else {
            row[m_folder_cols.m_icon] = Config::global.dir_close_buf->copy();
        }
    }



    void MailBox::on_folder_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                          GtkSelectionData* s, guint info, guint time) {
        gtk_selection_data_set(s, s->target, 8, (guchar*)"folder", 6);
    }
        


    void MailBox::on_folder_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, 
                                               GtkSelectionData* s, guint info, guint time) {

        
        std::string type((char*)s->data, s->length);
        std::cout << "MailBox::on_folder_drag_data_received: type " << type << std::endl;
        int cx, cy;
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* col;
        Gtk::TreeViewDropPosition pos;
        if(!m_folder_view->get_dest_row_at_pos(x, y, path, pos)) {
            std::cout << "no folder at (" << x << "," << y << ")" << std::endl;
            return;
        } 

        if(type == "message") {
            std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
            jlib::net::folder_indx_type indx; 
            for(std::list<Gtk::TreeModel::iterator>::iterator i = s.begin(); i != s.end(); i++) {
                Gtk::TreeModel::Row row = **i;
                indx.insert(indx.begin(), row[m_message_cols.m_indx]);
            }

            Gtk::TreeModel::iterator k = m_folder_select->get_selected();
            Gtk::TreeModel::iterator j = m_folder_model->get_iter(path);
            if(indx.size() > 0 && j && k) {
                Gtk::TreeModel::Row src = *k;
                Gtk::TreeModel::Row dst = *j;
                jlib::net::Email flags; flags.set_flag(jlib::net::Email::deleted_flag);

                m_box->copy_messages(src[m_folder_cols.m_info], dst[m_folder_cols.m_info], indx);
                m_box->set_message_flags(src[m_folder_cols.m_info], indx, flags);

                ctx->drag_finish(true, false, time);
                return;
            }

        } else if(type == "folder") {
            Gtk::TreeModel::iterator i = m_folder_select->get_selected();
            Gtk::TreeModel::iterator j = m_folder_model->get_iter(path);
            if(i && j) {
                Gtk::TreeModel::Row src = *i;
                Gtk::TreeModel::Row dst = *j;

                jlib::net::folder_info_type sinfo = src[m_folder_cols.m_info];
                jlib::net::folder_info_type dinfo = dst[m_folder_cols.m_info];

                jlib::net::folder_path_type spath = sinfo.path;
                jlib::net::folder_path_type dpath = dinfo.path;

                // easy case
                if(dinfo.attr.is_parent && !sinfo.attr.is_parent) {
                    jlib::net::folder_path_type path = dpath; path.push_back(spath.back());

                    dinfo.path = path;
                    dinfo.attr = sinfo.attr;

                    m_box->rename_folder(sinfo, dinfo);
                }
                else if(!dinfo.attr.is_parent && !sinfo.attr.is_parent) {
                    jlib::net::folder_path_type path = dpath; 
                    path.pop_back();
                    path.push_back(spath.back());

                    dinfo.path = path;
                    dinfo.attr = sinfo.attr;

                    m_box->rename_folder(sinfo, dinfo);
                }

                //

                std::cout << "move folder " << src[m_folder_cols.m_name] 
                          << " to folder " << dst[m_folder_cols.m_name] << std::endl;

                ctx->drag_finish(true, false, time);
                return;
            }
        } else {
            
        }
        ctx->drag_finish(false, false, time);
    }

    bool MailBox::on_folder_drag_drop(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time) {
        std::cout << "on_folder_drag_drop" << std::endl;
        return true;
    }

    void MailBox::on_message_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                           GtkSelectionData* s, guint info, guint time) {
        std::cout << "on_message_drag_data_get " << info << std::endl;
        gtk_selection_data_set(s, s->target, 8, (guchar*)"message", 7);
    }
        
    bool MailBox::on_message_drag_drop(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time) {
        std::cout << "on_message_drag_drop" << std::endl;
        return true;
    }

    void MailBox::set_pane_pos() {
        Config::iterator i = Config::global.find(m_name);
        if(i != Config::global.end()) {
            i->set_folder_pos(m_paned->get_position());
            i->set_message_pos(m_message_paned->get_position());
        }
    }

    void MailBox::set_col_pos() {
        Config::iterator i = Config::global.find(m_name);
        if(i != Config::global.end()) {
            std::vector<Config::MailBox::Column> cols;
            int n = m_message_view->get_columns().size();
            for(guint x = 0; x < n; x++){
                Gtk::TreeView::Column* col = m_message_view->get_column(x);

                if(col) {
                    std::cout << "col[" << x << "]: " << col->get_title() << " width " << col->get_width() << std::endl;
                    cols.push_back(Config::MailBox::Column(col->get_title(), col->get_width()));
                }
            } 

            i->set_message_cols(cols);
        }
    }

    std::list<Gtk::TreeModel::iterator> MailBox::get_selected_messages() {
        std::list<Gtk::TreeModel::iterator> r;
        Gtk::TreeSelection::ListHandle_Path path = m_message_select->get_selected_rows();
        Gtk::TreeSelection::ListHandle_Path::iterator i = path.begin(); 
        for(; i != path.end(); i++) {
            r.push_back(m_message_model->get_iter(*i));
        }

        return r;
    }

    bool MailBox::is_message_selected(int i) {
        std::list<Gtk::TreeModel::iterator> s = get_selected_messages();
        for(std::list<Gtk::TreeModel::iterator>::iterator j = s.begin(); j != s.end(); j++) {
            if((**j)[m_message_cols.m_indx] == i) {
                return true;
            }
        }

        return false;

    }

    void MailBox::refresh_viewer() {
        m_mail_view->refresh();
    }

    void MailBox::render_icon(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i) {
        Gtk::CellRendererPixbuf* r = dynamic_cast<Gtk::CellRendererPixbuf*>(cr);
        if(r) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::Email data = row[m_message_cols.m_data];
            
            r->property_pixbuf() = create_icon(data);
            
        } else {
            std::cerr << "MailBox::render_date: dynamic_cast() failed" <<std::endl;
        }
    }

    void MailBox::render_attach(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i) {
        Gtk::CellRendererPixbuf* r = dynamic_cast<Gtk::CellRendererPixbuf*>(cr);
        if(r) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::Email data = row[m_message_cols.m_data];
            std::string ctype = jlib::util::lower(data["content-type"]);
            
            if(ctype.find("multipart") != std::string::npos) {
                r->property_pixbuf() = Config::global.attachment_buf->copy();
            } else {
                r->property_pixbuf() = Config::global.empty_buf->copy();
            }
            
        } else {
            std::cerr << "MailBox::render_date: dynamic_cast() failed" <<std::endl;
        }
    }

    void MailBox::render_date(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i) {
        Gtk::CellRendererText* r = dynamic_cast<Gtk::CellRendererText*>(cr);
        if(r) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::Email data = row[m_message_cols.m_data];
            std::set<jlib::net::Email::flag_type> flags = data.get_flags();
            
            if(flags.find(jlib::net::Email::deleted_flag) != flags.end()) {
                r->property_strikethrough() = true;
            } else {
                r->property_strikethrough() = false;
            }
            
            r->property_text() = MailView::get(make_date(data));
            
        } else {
            std::cerr << "MailBox::render_date: dynamic_cast() failed" <<std::endl;
        }
    }

    void MailBox::render_size(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i) {
        Gtk::CellRendererText* r = dynamic_cast<Gtk::CellRendererText*>(cr);
        if(r) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::Email data = row[m_message_cols.m_data];
            std::set<jlib::net::Email::flag_type> flags = data.get_flags();
            
            if(flags.find(jlib::net::Email::deleted_flag) != flags.end()) {
                r->property_strikethrough() = true;
            } else {
                r->property_strikethrough() = false;
            }
            
            r->property_text() = MailView::get(make_size(data));
            
        } else {
            std::cerr << "MailBox::render_date: dynamic_cast() failed" <<std::endl;
        }

    }

    void MailBox::render_from(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i) {
        Gtk::CellRendererText* r = dynamic_cast<Gtk::CellRendererText*>(cr);
        if(r) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::Email data = row[m_message_cols.m_data];
            std::set<jlib::net::Email::flag_type> flags = data.get_flags();
            
            if(flags.find(jlib::net::Email::deleted_flag) != flags.end()) {
                r->property_strikethrough() = true;
            } else {
                r->property_strikethrough() = false;
            }
            
            try {
                r->property_text() = MailView::get_header(data, "from");
            } catch(...) {
                r->property_text() = "Unknown";
            }
            
        } else {
            std::cerr << "MailBox::render_date: dynamic_cast() failed" <<std::endl;
        }

    }

    void MailBox::render_subject(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i) {
        Gtk::CellRendererText* r = dynamic_cast<Gtk::CellRendererText*>(cr);
        if(r) {
            Gtk::TreeModel::Row row = *i;
            jlib::net::Email data = row[m_message_cols.m_data];
            std::set<jlib::net::Email::flag_type> flags = data.get_flags();
            
            if(flags.find(jlib::net::Email::deleted_flag) != flags.end()) {
                r->property_strikethrough() = true;
            } else {
                r->property_strikethrough() = false;
            }
            
            try {
                r->property_text() = MailView::get_header(data, "subject");
            } catch(...) {
                r->property_text() = "Unknown";
            }
            
        } else {
            std::cerr << "MailBox::render_date: dynamic_cast() failed" <<std::endl;
        }
    }

    Glib::RefPtr<Gdk::Pixbuf> MailBox::create_icon(jlib::net::Email data) {
        Glib::RefPtr<Gdk::Pixbuf> pix;
        std::set<jlib::net::Email::flag_type> flags = data.get_flags();

        if(flags.find(jlib::net::Email::seen_flag) == flags.end()) {
            pix = Config::global.mail_new_buf->copy();
        } else if(flags.find(jlib::net::Email::answered_flag) != flags.end()) {
            pix = Config::global.mail_replied_buf->copy();
        } else {
            pix = Config::global.mail_read_buf->copy();
        }

        return pix;
    }

    void MailBox::set_answered_flag(jlib::net::folder_info_type info, int indx, std::string msgid) {
        jlib::net::Email data;
        jlib::net::Email buf; buf.set_flag(jlib::net::Email::answered_flag);
        int j = 0;
        if(m_message_map.find(indx) != m_message_map.end()) {
            Gtk::TreeModel::iterator i = m_message_map[indx];
            Gtk::TreeModel::Row row = *i;
            data = row[m_message_cols.m_data];
            
            if(data["message-id"] == msgid) {
                if(getenv("GTKMAIL_MAILBOX_DEBUG")) {
                    std::cout << "MailBox::set_answered_flag: found message-id " << msgid 
                              << " at correct index" << std::endl;
                }
                
                jlib::net::folder_indx_type findx; findx.push_back(indx);
                m_box->set_message_flags(info, findx, buf);
                return;
            }
        }

        Gtk::TreeModel::Path path; path.push_back(0);
        
        for(Gtk::TreeModel::iterator i = m_message_model->get_iter(path); i; i++,j++) {
            Gtk::TreeModel::Row row = *i;
            data = row[m_message_cols.m_data];
            if(data["message-id"] == msgid) {
                if(getenv("GTKMAIL_MAILBOX_DEBUG")) {
                    std::cout << "MailBox::set_answered_flag: found message-id " << msgid << " at index"
                              << j << " not " << indx << std::endl;
                }
                
                jlib::net::folder_indx_type findx; findx.push_back(j);
                m_box->set_message_flags(info, findx, buf);
                return;
            }
            
        }
        
        if(getenv("GTKMAIL_MAILBOX_DEBUG")) {
            std::cout << "MailBox::set_answered_flag: failed to find message-id " << msgid << std::endl;
        }
        
    }

    void MailBox::on_column_clicked(std::string field) {
        m_search_field = field;
        //m_message_view->set_search_column(*col);
    }

    bool MailBox::on_search(const Glib::RefPtr<Gtk::TreeModel>& model, int col, 
                            const Glib::ustring& key, const Gtk::TreeModel::iterator& i) {

        Gtk::TreeModel::Row row = *i;
        jlib::net::Email data = row[m_message_cols.m_data];
        std::string val = data[m_search_field];

        if(m_search_field == "date") {
            val = make_date(data);
        } else if(m_search_field == "size") {
            val = make_size(data);
        }

        return (val.find(key) == std::string::npos);
    }
    
    std::string MailBox::make_date(jlib::net::Email data) {
        try {
            jlib::util::Date date;
            date.set(data["date"]);

            return date.get("%b %d %Y %H:%M");
        } catch(...) {
            return "";
        }
    }

    std::string MailBox::make_size(jlib::net::Email data) {
        try {
            unsigned long int size = data.get_data_size();
            std::ostringstream os;
            
            if(size < 1024) {
                os << size << " B";
            }
            else if(size < (1024*1024)) {
                os << size/1024 << " KB";
            }
            else { //if(size < (1024*1024*1024)) {
                os << size/(1024*1024) << " MB";
            }
#if 0
            else if(size < (1024*1024*1024*1024)) {
                os << size/(1024*1024*1024) << " GB";
            }
            else { //if(size < (1024*1024*1024*1024*1024)) {
                os << size/(1024*1024*1024*1024) << " TB";
            }
#endif
            return os.str();
        } catch(...) {
            return "???";
        }
    }

    Gtk::TreeModel::iterator MailBox::insert(jlib::net::Email data) {
        std::string rto = data["in-reply-to"];
        if(m_message_id_map.find(rto) != m_message_id_map.end()) {
            Gtk::TreeModel::Row row = *m_message_id_map[rto];
            return m_message_model->append(row.children());
        } else {
            return m_message_model->append();
        }
    }

}
