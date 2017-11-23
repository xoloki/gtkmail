/* -*- mode: C++ c-basic-offset: 4  -*-
 * MailView.hh - header file for class MailView
 * Copyright (c) 2003 Joe Yandle <jwy@divisionbyzero.com>
 *
 * class MailView provide both a text-based and an html-based view 
 * of a jlib::net::Email data object.  It can switch between the two at
 * will, or only use one (in case you don't even want html mail running).
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


#include "MailView.hh"
#include "Config.hh"
#include "Dialog.hh"
#include "AddressBook.hh"
#include "ProtocolHandlerMap.hh"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtkmm/widget.h>
#include <gtkmm/label.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/table.h>
#include <gtkmm/separator.h>
#include <gtkmm/image.h>
#include <gtkmm/stock.h>
#include <gtkmm/button.h>

#include <jlib/sys/auto.hh>
#include <jlib/sys/tfstream.hh>
#include <jlib/sys/sys.hh>
#include <jlib/util/util.hh>
#include <jlib/net/net.hh>


namespace gtkmail {
    
MailView::MailView()
{        
    m_box = Gtk::manage(new Gtk::VBox());
    
    this->add(*m_box);
    this->set_policy(Gtk::POLICY_AUTOMATIC,Gtk::POLICY_AUTOMATIC);
    this->show_all();
}
    
Gtk::Widget* MailView::wrap_html(std::string data, std::string from) {
    std::string style = "file:" + Config::global.get_user_style();
    Gtk::Widget* w = 0;
    bool load = Config::global.get_auto_load();
    gboolean auto_load = FALSE;
    WebKitWebSettings* settings;
    
    if(load && AddressBook::global.find_addr(jlib::net::extract_address(from)) != AddressBook::global.end())
        auto_load = TRUE;
    
    GtkWidget* view = webkit_web_view_new();
    
    settings = webkit_web_settings_new();
    g_object_set(G_OBJECT(settings), 
                 "user-stylesheet-uri", style.data(),
                 "auto-load-images", auto_load,
                 "enable-private-browsing", TRUE,
                 "enable-caret-browsing", FALSE,
                 "enable-html5-database", FALSE,
                 "enable-html5-local-storage", FALSE,
                 "enable-scripts", FALSE,
                 "enable-plugins", FALSE,
                 "default-font-size", 12,
                 "minimum-font-size", 12,
                 NULL);
    
    webkit_web_view_set_settings(WEBKIT_WEB_VIEW(view), settings);
    webkit_web_view_set_editable(WEBKIT_WEB_VIEW(view), FALSE);
    webkit_web_view_load_html_string(WEBKIT_WEB_VIEW(view), data.data(), "base uri");
    
    g_object_unref(G_OBJECT(settings));

    g_signal_connect(G_OBJECT(view), "navigation-requested", (GCallback)&MailView::on_navigation_requested, nullptr);
        
    w = Gtk::manage(Glib::wrap(view));
    
    return w;
}

WebKitNavigationResponse MailView::on_navigation_requested(WebKitWebView*web_view, WebKitWebFrame *frame, WebKitNetworkRequest *request)
{
    std::cout << "Got request for " << webkit_network_request_get_uri(request) << std::endl;

    const char* uri = webkit_network_request_get_uri(request);
    jlib::util::URL url(uri);
    std::string protocol = url.get_protocol();
    auto i = ProtocolHandlerMap::global.find(protocol);
    if(i != ProtocolHandlerMap::global.end()) {
        ProtocolHandler& handler = i->second;

        std::cout << "Found handler " << handler.get_handler() << std::endl;
        std::ostringstream cmd;
        cmd << handler.get_handler() << " '" << uri << "'" << std::endl;

        try {
            jlib::sys::shell(cmd.str());
        } catch(...) {
            std::cout << "Handler failed" << std::endl;
        }

        return WEBKIT_NAVIGATION_RESPONSE_IGNORE;
    } else {
        return WEBKIT_NAVIGATION_RESPONSE_ACCEPT;
    }
}

Gtk::Widget* MailView::wrap_html(jlib::net::Email data, std::string from) {
        std::string htmldata;
        std::string ctype = get_trim_ctype(data);

        if(ctype == "text/html") {
            htmldata = data.data();
        }
        else if(ctype == "text/plain") {
            htmldata = "<html><head></head><body><pre>"+data.data()+"</pre></body></html>";
        } else {
            throw exception("unknown content-type " + ctype);
        }

        return wrap_html(htmldata, from);
    }

    std::string MailView::get_trim_ctype(const jlib::net::Email& e) {
        std::string ctype = jlib::util::lower(e["CONTENT-TYPE"]);

        if(ctype.find(";") != std::string::npos) {
            std::vector<std::string> ctypev = jlib::util::tokenize(ctype, ";");
            ctype = jlib::util::trim(ctypev[0]);
        }

        return ctype;
    }
    

    void MailView::set_data(jlib::net::Email data, int level) {
        std::string ctype = get_trim_ctype(data);

        if(!level) {
            print_headers_text(data);
        }
        
        if(ctype.find("multipart/") == 0 || ctype.find("message/") == 0) {
            if(ctype == "multipart/alternative") {
                jlib::net::Email::iterator i, text = data.attach().end(), html = data.attach().end();
                for(i = data.attach().begin(); i != data.attach().end(); i++) {
                    std::string ctype = get_trim_ctype(*i);
                    
                    if(ctype == "text/html") {
                        html = i;
                    }
                    else if(ctype == "text/plain") {
                        text = i;
                    }
                }

                if(Config::global.get_default_text() == "text/plain" && text != data.attach().end()) {
                    set_data_text(*text, level+1);
                } else if(Config::global.get_default_text() == "text/html" && html != data.attach().end()) {
                    set_data_text(*html, level+1);
                } else if(text != data.attach().end()) {
                    set_data_text(*text, level+1);
                } else if(html != data.attach().end()) {
                    set_data_text(*html, level+1);
                }
            }
            else if(ctype == "multipart/encrypted" && data.attach().size() == 2 && data[0]["Content-Type"] == "application/pgp-encrypted" && data[1]["Content-Type"] == "application/octet-stream") {
                using namespace jlib::crypt;
                gpg::ctx ctx;
                gpg::data::ptr plain = gpg::data::create();
                bool show = false;
                
                gpg::data::ptr cipher = gpg::data::create(data[1].data());
                gpgme_verify_result_t result = ctx.op_decrypt_verify(cipher, plain);
                set_data(plain->read(), level+1);
            }
            else {
                if(level && ctype.find("message/") == 0) {
                    print_headers_text(data);
                }
                jlib::net::Email::iterator i;
                for(i = data.attach().begin(); i != data.attach().end(); i++) {
                    set_data(*i, level+1);
                }
            }
        }
        else {
            set_data_text(data, level);
        }
    }

    void MailView::set_data_text(jlib::net::Email data, int level) {
        std::string body;
        std::string ctype = get_trim_ctype(data);

        if(getenv("GTKMAIL_MAILVIEW_DEBUG"))
            std::cerr << "gtkmail::MailView::set_data_text(data, "<<level<<"): ctype " << ctype << std::endl;
        
        if(ctype.find("text/") == 0 && ctype != "text/html") {
            if(getenv("GTKMAIL_MAILVIEW_DEBUG"))
                std::cerr << "gtkmail::MailView::set_data_text: text, not html" << std::endl;

            body = get_data(data);
            //body += "\n";
            
            Gtk::TextView* text = create_text_view(body);
            m_box->pack_start(*text, Gtk::PACK_SHRINK);

        } else if(ctype == "text/html") {
            if(getenv("GTKMAIL_MAILVIEW_DEBUG"))
                std::cerr << "gtkmail::MailView::set_data_text: text/html" << std::endl;

            Gtk::Widget* child = wrap_html(data, m_data["from"]);

            child->show();
            m_box->pack_start(*child, Gtk::PACK_SHRINK);

        } else {
            if(getenv("GTKMAIL_MAILVIEW_DEBUG"))
                std::cerr << "gtkmail::MailView::set_data_text: other, print_ctype_text" << std::endl;

            print_ctype_text(data);
        }

    }

    
    std::string MailView::get_ctype_desc(std::string ctype) {
        ctype = jlib::util::lower(ctype);
        std::string ret;
        if(ctype == "text/plain") {
            ret = "Plain text";
        }
        else if(ctype == "text/html") {
            ret = "HTML text";
        }
        else if(ctype.find("text/") == 0) {
            ret = jlib::util::upper(ctype.substr(ctype.find("/")+1)) + " text";
        }
        else if(ctype.find("image/") == 0) {
            ret = jlib::util::upper(ctype.substr(ctype.find("/")+1)) + " image";
        }
        else if(ctype.find("audio/") == 0) {
            ret = jlib::util::upper(ctype.substr(ctype.find("/")+1)) + " audio";
        }
        else if(ctype == "application/octet-stream") {
            ret = "Binary data";
        }
        else if(ctype.find("application/") == 0) {
            ret = jlib::util::upper(ctype.substr(ctype.find("/")+1)) + " data";
        }
        else {
            u_int p = ctype.find("/");
            if(p != std::string::npos) {
                std::string type = ctype.substr(0, p);
                std::string subtype = jlib::util::upper(ctype.substr(p+1));
                ret = subtype + " " + type;
            } else {
                ret = "Unknown";
            }

        }

        ret += " attachment";

        return ret;
    }


    void MailView::set_data(jlib::net::Email data) {
        m_data = data;
        clear();
        set_data(data, 0);
    }

    void on_remove(Gtk::Widget& w, Gtk::VBox* vbox) {
        gtk_container_remove(GTK_CONTAINER(vbox->gobj()), w.gobj());
    }

    void MailView::clear() {
        //m_box->foreach(sigc::mem_fun(m_box, &Gtk::VBox::remove));
        m_box->foreach(sigc::bind(sigc::ptr_fun(&gtkmail::on_remove), m_box));
    }

    Glib::ustring MailView::get_header(jlib::net::Email data, std::string header) {
        std::string charset, val;
        val = data.headers().get(header, charset);
        
        return get(val, charset);
    }

    Glib::ustring MailView::get_data(jlib::net::Email data) {
        return get(data.data(), data.headers().get_charset());
    }
    
    Glib::ustring MailView::get(std::string data, std::string charset) {
        if(charset == "") {
            charset = Config::global.get_default_charset();
        }
        Glib::ustring ret;
        try {
            ret = Glib::convert(data, "UTF-8", charset);
        } catch(Glib::Exception& e) {
            ret = data;
            Glib::ustring::iterator i;
            while(!ret.validate(i)) { 
                ret.replace(i++, i, "*");
            }
        }
         
        return ret;
    }

    std::string MailView::unget(std::string data, std::string charset) {
        if(charset == "") {
            charset = Config::global.get_default_charset();
        }
        Glib::ustring ret;
        try {
            ret = Glib::convert_with_fallback(data, charset, "UTF-8");
        } catch(Glib::Exception& e) {
            ret = data;
            Glib::ustring::iterator i;
            while(!ret.validate(i)) { 
                ret.replace(i++, i, "*");
            }
        }
         
        return ret;
    }

    void MailView::print_headers_text(jlib::net::Email data) {
        Glib::ustring header, body;
        std::string ctype = get_trim_ctype(data);
        
        if((header = get_header(data, "date")) != "") {
            body += "   Date: " + header + "\n";
        }
        if((header = get_header(data, "from")) != "") {
            body += "   From: " + header + "\n";
            }
        if((header = get_header(data, "to")) != "") {
            body += "     To: " + header + "\n";
        }
        if((header = get_header(data, "cc")) != "") {
            body += "     Cc: " + header + "\n";
        }
        if((header = get_header(data, "subject")) != "") {
            body += "Subject: " + header + "\n";
        }
        
        if(body.length() > 0) {
            //body += "\n";
        }
        
        Gtk::TextView* text = create_text_view(body);
        m_box->pack_start(*text, Gtk::PACK_SHRINK);
    }

    void MailView::print_ctype_text(jlib::net::Email data) {
        Glib::ustring body;
        std::string ctype = get_trim_ctype(data);
        std::string filename = data.get_filename();
        std::string name = data.get_name();

        if(getenv("GTKMAIL_MAILVIEW_DEBUG"))
            std::cerr << "gtkmail::MailView::print_ctype_text: ctype " << ctype << std::endl;

        append_separator_text();

        Gtk::Button* button = Gtk::manage(new Gtk::Button());
        Gtk::Button* save_button = Gtk::manage(new Gtk::Button());
        Gtk::Image* image = Gtk::manage(new Gtk::Image(Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_BUTTON));
        Gtk::Image* save_image = Gtk::manage(new Gtk::Image(Gtk::Stock::GOTO_BOTTOM, Gtk::ICON_SIZE_BUTTON));
        Gtk::HBox* bbox = Gtk::manage(new Gtk::HBox(false, 10));
        Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox(false, 10));
        Gtk::HBox* hbox0 = Gtk::manage(new Gtk::HBox(false, 0));
        Gtk::Label* label = Gtk::manage(new Gtk::Label(get_ctype_desc(ctype) + 
                                                       (name != "" ? (" (" + name + ")") : "") +
                                                       (filename != "" ? (" \"" + filename + "\"") : "")));
        Gtk::Widget* child = 0;

        bbox->set_border_width(5);
        bbox->pack_start(*image);

        Glib::RefPtr<Gtk::RcStyle> rcstyle = label->get_modifier_style();
        rcstyle->set_font(Pango::FontDescription(Config::global.get_message_font()));
        label->modify_style(rcstyle);
        label->set_alignment(0.0, 0.5);

        if(ctype.find("image/") == 0) {
            jlib::sys::tfstream fs;
            fs << data.data();
            fs.close();
            
            child = Gtk::manage(new Gtk::Image(fs.get_path()));

            Glib::RefPtr<Gdk::Pixbuf> buf = Gdk::Pixbuf::create_from_file(fs.get_path());
            bbox->pack_start(*Gtk::manage(new Gtk::Image(buf->scale_simple(32, 32, Gdk::INTERP_NEAREST))));
        } 
        else if(ctype == "text/html") {
            child = wrap_html(data, m_data["from"]);

            Glib::RefPtr<Gdk::Pixbuf> buf = Config::global.text_buf;
            bbox->pack_start(*Gtk::manage(new Gtk::Image(buf->scale_simple(16, 16, Gdk::INTERP_BILINEAR))));
        }
        else {
            Glib::RefPtr<Gdk::Pixbuf> buf = Config::global.app_buf;
            bbox->pack_start(*Gtk::manage(new Gtk::Image(buf->scale_simple(16, 16, Gdk::INTERP_BILINEAR))));

            button->set_sensitive(false);
        }

        button->add(*bbox);
        save_button->add(*save_image);

        image->property_user_data() = (void*)"forward";

        hbox0->pack_start(*button, Gtk::PACK_SHRINK);
        hbox0->pack_start(*save_button, Gtk::PACK_SHRINK);

        hbox->pack_start(*hbox0, Gtk::PACK_SHRINK);
        hbox->pack_start(*label, Gtk::PACK_SHRINK);
        hbox->show_all();

        append_widget_text(hbox);
        if(child)
            append_widget_text(child);

        button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MailView::on_show_clicked),
                                                    image, child));
        
        save_button->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, &MailView::on_save_clicked),
                                                         data));
        
        append_separator_text();
    }

    void MailView::append_widget_text(Gtk::Widget* w, bool endl) {
        m_box->pack_start(*w, Gtk::PACK_SHRINK);
    }

    void MailView::append_separator_text() {
        Gtk::HSeparator* s = Gtk::manage(new Gtk::HSeparator());
        s->show();
        m_box->pack_start(*s, Gtk::PACK_SHRINK);
    }

    void MailView::on_show_clicked(Gtk::Image* image, Gtk::Widget* child) {
        void* prop = image->property_user_data();
        std::string s((char*)prop);

        if(s == "forward") {
            if(child) {
                child->show();
            }

            image->property_user_data() = (void*)"down";
            image->set(Gtk::Stock::GO_DOWN, Gtk::ICON_SIZE_BUTTON);
        } 
        else if(s == "down") {
            if(child) {
                child->hide();
            }

            image->property_user_data() = (void*)"forward";
            image->set(Gtk::Stock::GO_FORWARD, Gtk::ICON_SIZE_BUTTON);
        } 
        else {
            std::cout << "gtkmail::MailView::on_show_clicked: unknown data '" << s << "'" << std::endl;
        }
    }

    void MailView::on_save_clicked(jlib::net::Email data) {
        std::string file = (data.get_filename() != "" ? data.get_filename() : data.get_name());
        std::string title = "Save attachment" + (file != "" ? (" \"" + file + "\"") : "");
        Gtk::FileSelection* fileSel = new Gtk::FileSelection(title);
        fileSel->set_position(Gtk::WIN_POS_CENTER);

        if(file != "") {
            fileSel->set_filename(file);
        } 

        fileSel->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::mem_fun(*this, 
                                                                                    &MailView::on_save),
                                                                      fileSel, data));

        fileSel->get_ok_button()->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&operator delete), 
                                                                      fileSel));

        fileSel->get_cancel_button()->signal_clicked().connect(sigc::bind(sigc::ptr_fun(&operator delete), 
                                                                          fileSel));

        fileSel->show_all();    

    }

    void MailView::on_save(Gtk::FileSelection* fileSel, jlib::net::Email data) {
        try {
            std::string path = fileSel->get_filename();
            std::ofstream fs(path.c_str(), std::ios_base::out | std::ios_base::trunc);
            fs << data.data();
            fs.close();
        }
        catch(Glib::Exception& e) {
            display_exception(e.what(),this);
        }
        
        catch(std::exception& e) {
            display_exception(e.what(),this);
        }
        
        catch(...) {
            display_exception("Unknown exception while saving attachment" ,this);
        }
        
    }

    void MailView::refresh() {
        /*
          Glib::RefPtr<Gtk::RcStyle> rcstyle = m_text->get_modifier_style();
          rcstyle->set_font(Pango::FontDescription(Config::global.get_message_font()));
          m_text->modify_style(rcstyle);
        */
    }

    void MailView::on_html_view_show(Gtk::Widget* w) {
    }

    Gtk::TextView* MailView::create_text_view(std::string data) {
        Gtk::TextView* text = Gtk::manage(new Gtk::TextView());

        Glib::RefPtr<Gtk::RcStyle> rcstyle = text->get_modifier_style();
        rcstyle->set_font(Pango::FontDescription(Config::global.get_message_font()));
        text->modify_style(rcstyle);

        text->show();
        text->set_editable(FALSE);
        text->set_wrap_mode(Gtk::WRAP_WORD);
        text->get_buffer()->set_text(data);

        return text;
    }
}

