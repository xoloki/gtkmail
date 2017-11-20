// -*- mode: C++ c-basic-offset: 4  -*-
#ifndef _GDK_PIXBUF_H
#define _GDK_PIXBUF_H

/*
 * pixbuf.h
 *
 * Copyright 2000 Joe Yandle <jwy@divisionbyzero.com> 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk--/bitmap.h>
#include <gdk--/pixmap.h>

#include <exception>
#include <string>
#include <fstream>

namespace Gdk {
    class Pixbuf {
    public:
        class exception : public std::exception {
        public:
            exception(const std::string& msg = "pixbuf exception") : m_msg(msg) {}
            virtual ~exception() throw() {}
            virtual const char* what() const throw() { return m_msg.c_str(); }
        protected:
            std::string m_msg;
        };

        Pixbuf(const std::string& p_file) throw(std::exception);
        ~Pixbuf();
        Gdk_Pixmap pix() throw(std::exception);
        Gdk_Bitmap bit() throw(std::exception);
        
        int width() throw(std::exception);
        int height() throw(std::exception);
        
    protected:
        GdkPixmap* m_pix;
        GdkBitmap* m_bit;
        GdkPixbuf* m_pixbuf;
        
    };
}
 /* namespace Gdk */

#endif /* _GDK_PIXBUF_H */

