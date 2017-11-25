/* -*- mode: C++ c-basic-offset: 4  -*-
 * GtkMainWin.h - header file for class GtkMainWin
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * GtkMainWin provides an implementation of the abstract class View, using
 * Gtk::Window as one of its base classes.  It uses the gtkmm event handling
 * model (the connect_to_* functions)..
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

#ifndef GTKMAIL_MAILBOX_HH
#define GTKMAIL_MAILBOX_HH

#include <gtkmm/fileselection.h>
#include <gtkmm/menu.h>
#include <gtkmm/menubar.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/paned.h>
#include <gtkmm/image.h>
#include <gtkmm/notebook.h>
#include <gtkmm/statusbar.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/textview.h>
#include <gtkmm/texttag.h>
#include <gtkmm/fontselection.h>

#include <glibmm/thread.h>

#include <jlib/net/ASMailBox.hh>

#include <jlib/sys/Directory.hh>

#include "MailView.hh"

#include <queue>

namespace gtkmail {
    
    class MailBox : public Gtk::VBox {
    public:
        MailBox(jlib::net::ASMailBox* box, std::string name, int timeout, std::string email_addr, std::string url);

        void init_folders_ui();
        void update_messages();
        void list_messages();

        void write_status(std::string status);
        std::string message_status(int sel=-1);
        std::string checking_message();
        
        void set_message_status(int sel=-1);

        void on_folder_select();

        void on_message_selection_changed();
        void on_message_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* col);

        bool on_folder_select_cursor_row(bool edit);
        bool on_message_select_cursor_row(bool edit);

        bool on_input(Glib::IOCondition c);
        bool on_timeout();

        void on_error(std::string text);
        void on_login_error();
        void on_initialized();
        void on_status(std::string text);

        void on_folders_loaded(jlib::net::ASMailBox::folder_map_type map);

        void on_folder_listed(jlib::net::folder_info_type info);
        void on_folder_created(jlib::net::folder_info_type info);
        void on_folder_deleted(jlib::net::folder_info_type info);
        void on_folder_expunged(jlib::net::folder_info_type info);
        void on_folder_renamed(jlib::net::folder_info_type src, 
                               jlib::net::folder_info_type dst);

        void on_message_listed(jlib::net::folder_info_type info, int i, jlib::net::Email data);
        void on_message_loaded(jlib::net::folder_info_type info, int i, jlib::net::Email data);

        void on_message_flags_set(jlib::net::folder_info_type info, 
                                  jlib::net::folder_indx_type indx, jlib::net::Email data);
        void on_message_flags_unset(jlib::net::folder_info_type info, 
                                    jlib::net::folder_indx_type indx, jlib::net::Email data);

        Gtk::TreeModel::iterator add_folder(jlib::net::folder_info_type info);
        Gtk::TreeModel::iterator add_folder_to_parent(jlib::net::folder_info_type info,
                                                      Gtk::TreeModel::Children& kids);
        void folder_call(std::string s);
        void message_call(std::string s);
        
        void column_click(gint column);
        
        void folder_new_call();
        void folder_delete_call();
        void folder_rename_call();
        void folder_expunge_call();
        //void folder_expunge_exec(jlib::net::MailFolder* box);
        
        //void folder_new_exec(std::string s);
        //void folder_delete_exec();
        //void folder_rename_exec(std::string s,std::string old);
        
        void message_view_call();
        void message_view_thread_call();
        //void message_view_exec();
        void message_save_call(std::string s);
        //void message_save_exec(std::string s,std::list<u_int> d);
        void message_reply_call();
        void message_forward_call(bool in=false);
        void message_reply_all_call();
        void message_delete_call();
        void message_delete_thread_call();
        void message_undelete_call();
        void message_compose_call();

        void message_save(jlib::net::Email e, std::list<std::string> path);

        //void message_reply_exec(jlib::net::Email e);
        //void message_reply_all_exec(jlib::net::Email e);

        void message_set_delete(std::list<u_int> d);

        void make_folders();
        
        //void build_folders(Gtk::CTree_Helpers::RowList& rows,jlib::net::MailNode& node);
        //void build_folders(Gtk::Tree_Helpers::RowList& rows, jlib::net::MailGroup* group, const std::string& root="");
        bool menu_popup(GdkEventButton* event,Gtk::Menu* menu);
        //void build_popup(Gtk::Menu* menu, jlib::net::MailNode& node);
        //void build_popup(Gtk::Menu* menu, jlib::net::MailGroup* group, std::string base="");
        void create_popup(Gtk::Menu* menu);
        void key_popup(Gtk::Menu* menu);

        int timeout();
        int sent_mail_timeout();
        
        void popup();

        std::list<std::string> get_path();
        std::string get_pathstr();
        std::string slash_path();
        //jlib::net::MailNode* get_node();

        void fill_email(unsigned int i);
        void show_email();

        //void scan_box(bool scan, jlib::net::MailFolder* box);

//         void check_box();
        //void scan_box(bool scan,jlib::net::MailBox* box);

        void set_sensitive(bool b);
        bool is_sensitive();

        void set_flags(unsigned int row, unsigned int col, std::string flag);
        void set_flags(std::list<u_int> rows, unsigned int col, std::string flag);

        //void set_flags(SignalQueue::action::row_logical_type row, unsigned int col, std::string flag);
        //void set_flags(std::list<SignalQueue::action::row_logical_type> rows, unsigned int col, std::string flag);

        void set_new_flags(std::list<u_int> rows, std::string flag);
        void set_del_flags(std::list<u_int> rows, std::string flag);

        //void set_new_flags(std::list<SignalQueue::action::row_logical_type> rows, std::string flag);
        //void set_del_flags(std::list<SignalQueue::action::row_logical_type> rows, std::string flag);

        //void set_folder_row(std::list<std::string> s, Gtk::CTree_Helpers::Row& row, bool begin=true);
        //void set_folder_cell(std::string s, Gtk::CTree_Helpers::Row& row, u_int col);
        void set_folder_sizes(std::list<std::string> s);

        /*
        void build_message_threads(Gtk::CTree_Helpers::RowList& rows, 
                                   std::vector<bool>& used,
                                   u_int p, u_int& ncount, u_int& tcount);

        void add_message(Gtk::CTree_Helpers::RowList& rows, u_int p, u_int& ncount, u_int& tcount);
        */
        void add_message(Gtk::TreeModel::iterator i, int p, jlib::net::Email email);

        std::string get_name();
        std::string get_address();
        std::string get_url();
        int get_time();

        int get_sort_column();

        bool is_sortable();
        bool is_ubersortable();
        bool is_threadable();

        Gtk::Menu* get_save_menu();
        
        void add_sent_mail(jlib::net::Email e);
        void set_answered_flag(jlib::net::folder_info_type info, int indx, std::string msgid);

        jlib::net::folder_info_type get_selected_folder_info();

        void reset();
        
        void display_images();
        
        void set_pane_pos();
        void set_col_pos();

        void refresh_viewer();
        
    private:
        void init_ui();
        void init_box();

        void refresh_folders();
        std::string get_flag_text(jlib::net::Email e);

        Gtk::HBox* m_hbox;
        Gtk::Statusbar* m_status;
        Gtk::HPaned* m_paned;
        Gtk::VPaned* m_message_paned;

        bool on_folder_move_handle(Gtk::ScrollType stype);
        bool on_message_move_handle(Gtk::ScrollType stype);

        std::list<Gtk::TreeModel::iterator> get_selected_messages();
        bool is_message_selected(int i);

        enum {
            TARGET_MESSAGE,
            TARGET_FOLDER
        };

        
        /*
        void on_message_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                      GtkSelectionData* s, guint info, guint time);
        void on_message_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, 
                                           GtkSelectionData* s, guint info, guint time);
        void on_message_drag_data_delete(const Glib::RefPtr<Gdk::DragContext>& ctx);

        void on_folder_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, 
                                           GtkSelectionData* s, guint info, guint time);
        void on_folder_drag_data_delete(const Glib::RefPtr<Gdk::DragContext>& ctx);
        */

        void on_message_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                      GtkSelectionData* s, guint info, guint time);
        
        void on_folder_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                     GtkSelectionData* s, guint info, guint time);
        
        void on_folder_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, 
                                          GtkSelectionData* s, guint info, guint time);
        
        bool on_message_drag_drop(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time);
        bool on_folder_drag_drop(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time);

        void on_folder_expand(const Gtk::TreeModel::iterator& i, const Gtk::TreeModel::Path& path);
        void on_folder_collapse(const Gtk::TreeModel::iterator& i, const Gtk::TreeModel::Path& path);


#if 0
        class MessageView : public Gtk::TreeView {
        public:
            MessageView();

        protected:
            /*
            void on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& ctx);
            bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time);
            bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time);
            void on_drag_end(const Glib::RefPtr<Gdk::DragContext>& ctx);
            */
            void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                  GtkSelectionData* s, guint info, guint time);
            void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, 
                                       GtkSelectionData* s, guint info, guint time);
            void on_drag_data_delete(const Glib::RefPtr<Gdk::DragContext>& ctx);


            std::list<Gtk::TargetEntry> m_targets;
        };

        class FolderView : public Gtk::TreeView {
        public:
            FolderView();

        protected:
            /*
            void on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& ctx);
            bool on_drag_motion(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time);
            bool on_drag_drop(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, guint time);
            void on_drag_end(const Glib::RefPtr<Gdk::DragContext>& ctx);
            */
            void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& ctx, 
                                     GtkSelectionData* s, guint info, guint time);
            void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& ctx, int x, int y, 
                                          GtkSelectionData* s, guint info, guint time);
            void on_drag_data_delete(const Glib::RefPtr<Gdk::DragContext>& ctx);

            std::list<Gtk::TargetEntry> m_targets;
        };
#endif
        class MessageModel : public Gtk::TreeStore {
        public:
            bool row_draggable(const TreeModel::Path& path) const { return true; }
        protected:
            explicit MessageModel(const Gtk::TreeModelColumnRecord& columns) 
                :  Gtk::TreeStore(columns) {}
        };

        class FolderModelCols : public Gtk::TreeModel::ColumnRecord {
        public:
            FolderModelCols() { add(m_icon); add(m_info); add(m_name); }

            Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> > m_icon;
            Gtk::TreeModelColumn<jlib::net::folder_info_type> m_info;
            Gtk::TreeModelColumn<Glib::ustring> m_name;
        };

        FolderModelCols m_folder_cols;
        Glib::RefPtr<Gtk::TreeStore> m_folder_model;
        Gtk::TreeView* m_folder_view;
        Glib::RefPtr<Gtk::TreeSelection> m_folder_select;

        class MessageModelCols : public Gtk::TreeModel::ColumnRecord {
        public:
            MessageModelCols() { 
                add(m_indx); add(m_load); add(m_data); 
            }

            Gtk::TreeModelColumn<int>              m_indx;
            Gtk::TreeModelColumn<bool>             m_load;
            Gtk::TreeModelColumn<jlib::net::Email> m_data;
        };

        MessageModelCols m_message_cols;
        Glib::RefPtr<Gtk::TreeStore> m_message_model;
        Gtk::TreeView* m_message_view;
        Glib::RefPtr<Gtk::TreeSelection> m_message_select;

        void render_icon(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i);
        void render_attach(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i);
        void render_date(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i);
        void render_size(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i);
        void render_from(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i);
        void render_subject(Gtk::CellRenderer *cr, const Gtk::TreeModel::iterator& i);

        void on_column_clicked(std::string field);
        bool on_search(const Glib::RefPtr<Gtk::TreeModel>& model, int col, const Glib::ustring& key, 
                       const Gtk::TreeModel::iterator& i);

        Gtk::TreeModel::iterator insert(jlib::net::Email data);

        std::string make_date(jlib::net::Email data);
        std::string make_size(jlib::net::Email data);

        Glib::RefPtr<Gdk::Pixbuf> create_icon(jlib::net::Email data);

        Gtk::TreeModel::iterator m_selected_folder;
        Gtk::TreeModel::iterator m_selected_message;

        MailView* m_mail_view;

        std::set<int> m_messages_activated;

        Gtk::Menu* m_popup;
        Gtk::Menu* m_save_menu;

        jlib::net::ASMailBox* m_box;
        Glib::Mutex m_ops_lock;

        std::string m_name,m_url;
        //std::vector<u_int> m_del;

        jlib::net::Email m_email_buf;

        std::string m_email_addr;

        int m_time;

        bool m_sortable, m_threadable, m_ubersortable;
        int m_sort_column;
        std::string m_sort_field;

        //GtkCListCompareFunc m_compare_func;
        //GtkSortType m_sort_type;
        bool m_have_sorted;

        bool m_have_new_message;
        //Gtk::TreeView_Helpers::Row m_new_message_row;

        std::queue<jlib::net::Email> m_sent_mail_queue;
        std::map<jlib::net::folder_path_type, Gtk::TreeModel::iterator> m_folder_map;
        std::map<int, Gtk::TreeModel::iterator> m_message_map;
        std::map<std::string, Gtk::TreeModel::iterator> m_message_id_map;
        std::string m_search_field;
    };
    
}

#endif //GTKMAIL_MAILBOX_HH
