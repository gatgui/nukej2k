#ifndef __fiWriter_h__
#define __fiWriter_h__

#include <DDImage/Writer.h>
#include <DDImage/Row.h>
#include <DDImage/ARRAY.h>
#include <DDImage/Thread.h>
#include <DDImage/Knobs.h>
#include <DDImage/LUT.h>
#include <FreeImage.h>
#include <string>
#include "fiMeta.h"

class FIWriter : public DD::Image::Writer
{
public:
  
  FIWriter(DD::Image::Write *iop)
    : DD::Image::Writer(iop), mDataType(0)
  {
  }
  
  virtual ~FIWriter()
  {
  }
  
  virtual void execute()
  {
    int w = width();
    int h = height();
    
    DD::Image::Channel ch[4] = {DD::Image::Chan_Black, DD::Image::Chan_Black, DD::Image::Chan_Black, DD::Image::Chan_Black};
    
    int ired = -1;
    int igreen = -1;
    int iblue = -1;
    int ialpha = -1;
    int depth = 0;
    
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
              to_byte(DD::Image::Chan_Red-1,   scanline+FI_RGBA_RED, row[ch[ired]],   alpha, w, imgDepth);
            }
            if (igreen != -1)
            {
              to_byte(DD::Image::Chan_Green-1, scanline+FI_RGBA_GREEN, row[ch[igreen]], alpha, w, imgDepth);
            }
            if (iblue != -1)
            {
              to_byte(DD::Image::Chan_Blue-1,  scanline+FI_RGBA_BLUE,   row[ch[iblue]],  alpha, w, imgDepth);
            }
            to_byte(DD::Image::Chan_Alpha-1, scanline+FI_RGBA_ALPHA, row[ch[ialpha]], 0, w, imgDepth);
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
              to_byte(DD::Image::Chan_Red-1,   scanline+FI_RGBA_RED, row[ch[ired]],   0, w, imgDepth);
            }
            if (igreen != -1)
            {
              to_byte(DD::Image::Chan_Green-1, scanline+FI_RGBA_GREEN, row[ch[igreen]], 0, w, imgDepth);
            }
            if (iblue != -1)
            {
              to_byte(DD::Image::Chan_Blue-1,  scanline+FI_RGBA_BLUE,   row[ch[iblue]],  0, w, imgDepth);
            }
          }
        }
      }
    }
    else
    {
      // RGB(A)
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
              to_short(DD::Image::Chan_Red-1,   scanline, row[ch[ired]],   alpha, w, 16, imgDepth);
            }
            if (igreen != -1)
            {
              to_short(DD::Image::Chan_Green-1, scanline+1, row[ch[igreen]], alpha, w, 16, imgDepth);
            }
            if (iblue != -1)
            {
              to_short(DD::Image::Chan_Blue-1,  scanline+2,   row[ch[iblue]],  alpha, w, 16, imgDepth);
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
              to_short(DD::Image::Chan_Red-1,   scanline, row[ch[ired]],   0, w, 16, imgDepth);
            }
            if (igreen != -1)
            {
              to_short(DD::Image::Chan_Green-1, scanline+1, row[ch[igreen]], 0, w, 16, imgDepth);
            }
            if (iblue != -1)
            {
              to_short(DD::Image::Chan_Blue-1,  scanline+2,   row[ch[iblue]],  0, w, 16, imgDepth);
            }
          }
        }
      }
    }
    
    setMetadata(bitmap);
    
    if (!FreeImage_Save(fif, bitmap, filename(), getFIOptions()))
    {
      iop->error("Could not save file: \"%s\"", filename());
    }
    
    progressFraction(1.0);
    
    FreeImage_Unload(bitmap);
  }
  
  virtual void knobs(DD::Image::Knob_Callback f)
  {
    static const char* const dtypes[] = {"8 bit", "16 bit", 0};
    
    DD::Image::Enumeration_knob(f, &mDataType, dtypes, "datatype", "data type");
  }
  
  virtual int getFIOptions()
  {
    return 0;
  }
  
  virtual void setMetadata(FIBITMAP *img)
  {
    setMetadata(img, META_PREMULT, premult(), META_PREMULT_DESC);
    setMetadata(img, META_COLORSPACE, getColorSpace(), META_COLORSPACE_DESC);
    setMetadata(img, META_SCRIPT, getScriptPath(), META_SCRIPT_DESC);
  }
  
protected:
  
  void setMetadata(FIBITMAP *img, 
                   FREE_IMAGE_MDMODEL model, FREE_IMAGE_MDTYPE dtype,
                   const std::string &key, int count, int length, void *data,
                   const std::string &desc="")
  {
    FITAG *tag = FreeImage_CreateTag();
    if (tag)
    {
      FreeImage_SetTagKey(tag, key.c_str());
      FreeImage_SetTagDescription(tag, desc.c_str());
      FreeImage_SetTagType(tag, dtype);
      FreeImage_SetTagCount(tag, count);
      FreeImage_SetTagLength(tag, length);
      FreeImage_SetTagValue(tag, data);
      FreeImage_SetMetadata(model, img, FreeImage_GetTagKey(tag), tag);
      FreeImage_DeleteTag(tag);
    }
  }
  
  void setMetadata(FIBITMAP *img, const std::string &key, bool val, const std::string &desc="")
  {
    unsigned char byte = (val ? 1 : 0);
    setMetadata(img, FIMD_IPTC, FIDT_BYTE, key, 1, 1, (void*)&byte, desc);
  }
  
  void setMetadata(FIBITMAP *img, const std::string &key, const std::string &val, const std::string &desc="")
  {
    setMetadata(img, FIMD_IPTC, FIDT_ASCII, key, val.length()+1, val.length()+1, (void*) val.c_str(), desc);
  }
  
  std::string getColorSpace()
  {
    // no better way?
    DD::Image::LUT *csl = lut();
    
    const char **n = &(DD::Image::LUT::builtin_names[0]);
    
    while (*n != 0)
    {
      if (csl == DD::Image::LUT::builtin(*n))
      {
        return *n;
      }
      ++n;
    }
    
    return "";
  }
  
  std::string getScriptPath()
  {
    // couldn't figure out that one yet
    return "";
  }
  
protected:
  
  int mDataType;
};

#endif
