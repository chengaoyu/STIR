/*
    Copyright (C) 2013, Institute for Bioengineering of Catalonia
    This file is part of STIR.

    This file is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.

    This file is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    See STIR/LICENSE.txt for details
*/
/*!

  \file
  \ingroup IO
  \brief Declaration of class stir::ITKOutputFileFormat

  \author Berta Marti Fuster
*/

#ifndef __stir_IO_ITKOutputFileFormat_H__
#define __stir_IO_ITKOutputFileFormat_H__

#include "stir/IO/OutputFileFormat.h"
#include "stir/RegisteredParsingObject.h"

START_NAMESPACE_STIR

template <int num_dimensions, typename elemT> class DiscretisedDensity;

/*!
  \ingroup InterfileIO
  \brief 
  Implementation of OutputFileFormat paradigm for the Interfile format.
 */
class ITKOutputFileFormat : 
  public RegisteredParsingObject<
        ITKOutputFileFormat,
        OutputFileFormat<DiscretisedDensity<3,float> >,
        OutputFileFormat<DiscretisedDensity<3,float> > >
{
 private:
  typedef 
     RegisteredParsingObject<
        ITKOutputFileFormat,
        OutputFileFormat<DiscretisedDensity<3,float> >,
        OutputFileFormat<DiscretisedDensity<3,float> > >
    base_type;
public :
    //! Name which will be used when parsing an OutputFileFormat object
  static const char * const registered_name;

  ITKOutputFileFormat(const NumericType& = NumericType::FLOAT, 
                   const ByteOrder& = ByteOrder::native);


  virtual ByteOrder set_byte_order(const ByteOrder&, const bool warn = false);
 protected:
  virtual Succeeded  
    actual_write_to_file(string& output_filename,
		  const DiscretisedDensity<3,float>& density) const;


  virtual void set_defaults();
  virtual void initialise_keymap();
  virtual bool post_processing();

};



END_NAMESPACE_STIR


#endif
