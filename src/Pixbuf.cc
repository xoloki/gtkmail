/* -*- mode: C++ c-basic-offset: 4  -*-
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

#include "Pixbuf.hh"

namespace Gdk {
    
    Pixbuf::Pixbuf(const std::string& p_file) throw(std::exception) {
        m_pixbuf = gdk_pixbuf_new_from_file((char*)(p_file.c_str()));
        if(m_pixbuf == NULL) {
            throw exception("error loading file "+p_file);
        }
        gdk_pixbuf_render_pixmap_and_mask(m_pixbuf, &m_pix, &m_bit, 100);
    }
    
    Gdk_Pixmap Pixbuf::pix() throw(std::exception){ 
        if(m_pixbuf == NULL) {
            throw exception("pixbuf was NULL");
        }
        return Gdk_Pixmap(m_pix); 
    }
    
    Gdk_Bitmap Pixbuf::bit() throw(std::exception) { 
        if(m_pixbuf == NULL) {
            throw exception("pixbuf was NULL");
        }
        return Gdk_Bitmap(m_bit); 
    }
    
    int Pixbuf::width() throw(std::exception) { 
        if(m_pixbuf == NULL) {
            throw exception("pixbuf was NULL");
        }
        return gdk_pixbuf_get_width(m_pixbuf);
    }
    
    int Pixbuf::height() throw(std::exception) { 
        if(m_pixbuf == NULL) {
            throw exception("pixbuf was NULL");
        }
        return gdk_pixbuf_get_height(m_pixbuf);
    }
    
    Pixbuf::~Pixbuf() { 
        if(m_pixbuf != NULL) {
            gdk_pixbuf_unref(m_pixbuf); 
        }
    }
    
}
