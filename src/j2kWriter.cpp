#include "fiWriter.h"

class j2kWriter : public FIWriter
{
  public:
    
    j2kWriter(DD::Image::Write *iop)
      : FIWriter(iop)
    {
      setCompression(16);
    }
    
    virtual ~j2kWriter()
    {
    }
    
    virtual void knobs(DD::Image::Knob_Callback f)
    {
      FIWriter::knobs(f);
      
      DD::Image::Float_knob(f, &mQuality, "quality", "quality");
      DD::Image::SetRange(f, 0.0, 1.0);
      DD::Image::SetFlags(f, DD::Image::Knob::SLIDER);
    }
    
    virtual int getFIOptions()
    {
      return getCompression();
    }
    
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
    
    double mQuality;
};

DD::Image::Writer* Build(DD::Image::Write *iop)
{
  return new j2kWriter(iop);
}

const DD::Image::Writer::Description j2kWriterDesc("j2k\0", Build);

