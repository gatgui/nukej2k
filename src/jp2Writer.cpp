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

#include "jp2Writer.h"
#include <DDImage/Knobs.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>

jp2Writer::jp2Writer(DD::Image::Write *iop)
  : DD::Image::Writer(iop), mDataType(0)
{
  setCompression(16);
}

jp2Writer::~jp2Writer()
{
}

void jp2Writer::execute()
{
  int w = width();
  int h = height();
  
  DD::Image::Channel ch[4] = {DD::Image::Chan_Black, DD::Image::Chan_Black, DD::Image::Chan_Black, DD::Image::Chan_Black};
  
  int ired = -1;
  int igreen = -1;
  int iblue = -1;
  int ialpha = -1;
  int depth = 0;
  
  //for (int i=0; i<4; ++i)
  for (int i=0; i<iop->depth() && depth < 4; ++i)
  {
    DD::Image::Channel c = iop->channel_written_to(i);
    
    switch (c)
    {
    case DD::Image::Chan_Red:
      if (ired == -1)
      {
        ired = depth;
        ch[depth++] = c;
      }
      break;
    case DD::Image::Chan_Green:
      if (igreen == -1)
      {
        igreen = depth;
        ch[depth++] = c;
      }
      break;
    case DD::Image::Chan_Blue:
      if (iblue == -1)
      {
        iblue = depth;
        ch[depth++] = c;
      }
      break;
    case DD::Image::Chan_Alpha:
      if (ialpha == -1)
      {
        ialpha = depth;
        ch[depth++] = c;
      }
    default:
      break;
    }
  }
  
  if (depth == 0)
  {
    iop->error("No RGB, nor Alpha channel to write");
    return;
  }
  
  // build channel mask
  
  DD::Image::ChannelSet channels(ch, depth);
  
  
  
  FIBITMAP *bitmap = 0;
  
  FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(filename());
  
  if (fif == FIF_UNKNOWN)
  {
    iop->error("Unsupported file format");
    return;
  }
  
  // set data to DIB
  input0().request(0, 0, w, h, channels, 1);
  
  DD::Image::Row row(0, w);
  
  progressFraction(0.0);
  
  int imgDepth = (ialpha != -1 ? 4 : 3);
  
  const float *alpha = 0;
  
  
  if (mDataType == 0)
  {
    BYTE *scanline = 0;
    
    bitmap = FreeImage_AllocateT(FIT_BITMAP, w, h, imgDepth*8);
    
    if (!bitmap)
    {
      iop->error("Could not create FreeImage bitmap");
      return;
    }
    
    if (ired == -1)
    {
      // no colour channels, only alpha
      
      for (int y=0; y<h; ++y)
      {
        if (aborted())
        {
          FreeImage_Unload(bitmap);
          return;
        }
        
        progressFraction(double(y) / h);
        
        get(y, 0, w, channels, row);
        
        scanline = FreeImage_GetScanLine(bitmap, y);
        memset(scanline, 0, w*imgDepth);
        
        to_byte(DD::Image::Chan_Alpha-1, scanline+3, row[ch[ialpha]], 0, w, imgDepth);
      }
    }
    else
    {
      // colours channels (maybe duplicated) + option alpha
      
      if (ialpha != -1)
      {
        // alpha channel present
        
        for (int y=0; y<h; ++y)
        {
          if (aborted())
          {
            FreeImage_Unload(bitmap);
            return;
          }
          
          progressFraction(double(y) / h);
          
          get(y, 0, w, channels, row);
          
          alpha = row[ch[ialpha]];
          
          scanline = FreeImage_GetScanLine(bitmap, y);
          memset(scanline, 0, w*imgDepth);
          
          if (ired != -1)
          {
            to_byte(DD::Image::Chan_Red-1,   scanline+2, row[ch[ired]],   alpha, w, imgDepth);
          }
          if (igreen != -1)
          {
            to_byte(DD::Image::Chan_Green-1, scanline+1, row[ch[igreen]], alpha, w, imgDepth);
          }
          if (iblue != -1)
          {
            to_byte(DD::Image::Chan_Blue-1,  scanline,   row[ch[iblue]],  alpha, w, imgDepth);
          }
          to_byte(DD::Image::Chan_Alpha-1, scanline+3, row[ch[ialpha]], 0, w, imgDepth);
        }
      }
      else
      {
        // no alpha channel
        
        for (int y=0; y<h; ++y)
        {
          if (aborted())
          {
            FreeImage_Unload(bitmap);
            return;
          }
          
          progressFraction(double(y) / h);
          
          get(y, 0, w, channels, row);
          
          scanline = FreeImage_GetScanLine(bitmap, y);
          memset(scanline, 0, w*imgDepth);
          
          if (ired != -1)
          {
            to_byte(DD::Image::Chan_Red-1,   scanline+2, row[ch[ired]],   0, w, imgDepth);
          }
          if (iblue != -1)
          {
            to_byte(DD::Image::Chan_Green-1, scanline+1, row[ch[igreen]], 0, w, imgDepth);
          }
          if (igreen != -1)
          {
            to_byte(DD::Image::Chan_Blue-1,  scanline,   row[ch[iblue]],  0, w, imgDepth);
          }
        }
      }
    }
  }
  else
  {
    U16 *scanline = 0;
    
    bitmap = FreeImage_AllocateT((imgDepth == 4 ? FIT_RGBA16 : FIT_RGB16), w, h, imgDepth*16);
    
    if (!bitmap)
    {
      iop->error("Could not create FreeImage bitmap");
      return;
    }
    
    if (ired == -1)
    {
      // no colour channels, only alpha
      
      for (int y=0; y<h; ++y)
      {
        if (aborted())
        {
          FreeImage_Unload(bitmap);
          return;
        }
        
        progressFraction(double(y) / h);
        
        get(y, 0, w, channels, row);
        
        scanline = (U16*) FreeImage_GetScanLine(bitmap, y);
        memset(scanline, 0, w*imgDepth*2);
        
        to_short(DD::Image::Chan_Alpha-1, scanline+3, row[ch[ialpha]], 0, w, 16, imgDepth);
      }
    }
    else
    {
      // colours channels (maybe duplicated) + option alpha
      
      if (ialpha != -1)
      {
        // alpha channel present
        
        for (int y=0; y<h; ++y)
        {
          if (aborted())
          {
            FreeImage_Unload(bitmap);
            return;
          }
          
          progressFraction(double(y) / h);
          
          get(y, 0, w, channels, row);
          
          alpha = row[ch[ialpha]];
          
          scanline = (U16*) FreeImage_GetScanLine(bitmap, y);
          memset(scanline, 0, w*imgDepth*2);
          
          if (ired != -1)
          {
            to_short(DD::Image::Chan_Red-1,   scanline+2, row[ch[ired]],   alpha, w, 16, imgDepth);
          }
          if (igreen != -1)
          {
            to_short(DD::Image::Chan_Green-1, scanline+1, row[ch[igreen]], alpha, w, 16, imgDepth);
          }
          if (iblue != -1)
          {
            to_short(DD::Image::Chan_Blue-1,  scanline,   row[ch[iblue]],  alpha, w, 16, imgDepth);
          }
          to_short(DD::Image::Chan_Alpha-1, scanline+3, row[ch[ialpha]], 0, w, 16, imgDepth);
        }
      }
      else
      {
        // no alpha channel
        
        for (int y=0; y<h; ++y)
        {
          if (aborted())
          {
            FreeImage_Unload(bitmap);
            return;
          }
          
          progressFraction(double(y) / h);
          
          get(y, 0, w, channels, row);
          
          scanline = (U16*) FreeImage_GetScanLine(bitmap, y);
          memset(scanline, 0, w*imgDepth*2);
          
          if (ired != -1)
          {
            to_short(DD::Image::Chan_Red-1,   scanline+2, row[ch[ired]],   0, w, 16, imgDepth);
          }
          if (igreen != -1)
          {
            to_short(DD::Image::Chan_Green-1, scanline+1, row[ch[igreen]], 0, w, 16, imgDepth);
          }
          if (iblue != -1)
          {
            to_short(DD::Image::Chan_Blue-1,  scanline,   row[ch[iblue]],  0, w, 16, imgDepth);
          }
        }
      }
    }
  }
  
  if (!FreeImage_Save(fif, bitmap, filename(), getCompression()))
  {
    iop->error("Could not save file: \"%s\"", filename());
  }
  
  progressFraction(1.0);
  
  FreeImage_Unload(bitmap);
}

void jp2Writer::knobs(DD::Image::Knob_Callback f)
{
  static const char* const dtypes[] = {"8 bit", "16 bit", 0};
  
  DD::Image::Enumeration_knob(f, &mDataType, dtypes, "datatype", "data type");
  
  DD::Image::Float_knob(f, &mQuality, "quality", "quality");
  DD::Image::SetRange(f, 0.0, 1.0);
  DD::Image::SetFlags(f, DD::Image::Knob::SLIDER);
  // set range [1, 512]
  // set default 16
}

static DD::Image::Writer* build(DD::Image::Write *iop)
{
  return new jp2Writer(iop);
}

const DD::Image::Writer::Description jp2Writer::Desc("jp2\0", build);
