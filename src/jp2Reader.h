#ifndef __jp2Reader_h__
#define __jp2Reader_h__
/*

Copyright (C) 2010  Gaetan Guidet

This file is part of nukej2k.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

#include <DDImage/Reader.h>
#include <DDImage/Row.h>
#include <DDImage/ARRAY.h>
#include <DDImage/Thread.h>
#include <FreeImage.h>

class jp2Reader : public DD::Image::Reader
{
  public:
    
    static const Description Desc;
    
    jp2Reader(DD::Image::Read *iop);
    virtual ~jp2Reader();
    
    virtual void open();
    virtual void engine(int y, int x, int r, DD::Image::ChannelMask, DD::Image::Row &);
  
  protected:
    
    FREE_IMAGE_FORMAT mFIF;
    FIBITMAP *mBitmap;
    std::string mPath;
    FREE_IMAGE_TYPE mFIT;
    unsigned int mWidth;
    unsigned int mHeight;
    unsigned int mBPP;
    unsigned int mNumChannels;
    unsigned int mChannelBytes;
};


#endif
