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

#ifndef __j2kWriter_h__
#define __j2kWriter_h__

#include <DDImage/Writer.h>
#include <DDImage/Row.h>
#include <DDImage/ARRAY.h>
#include <DDImage/Thread.h>
#include <FreeImage.h>
#include <cmath>

class j2kWriter : public DD::Image::Writer
{
  public:
    
    static const Description Desc;
    
    j2kWriter(DD::Image::Write *iop);
    virtual ~j2kWriter();
    
    virtual void execute();
    virtual void knobs(DD::Image::Knob_Callback f);
  
  protected:
    
    inline int getCompression()
    {
      // [0, 1] -> [1, 512]
      return 1 + int(floor((1.0 - mQuality) * 511.0));
    }
    
    inline void setCompression(int v)
    {
      // [1, 512] -> [0, 1]
      mQuality = 1.0 - double(v - 1) / 511.0;
    }
    
    int mDataType;
    double mQuality;
};

#endif
