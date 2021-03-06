/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#include "vcl/unohelp.hxx"
#include <DataFlavorMapping.hxx>
#include "HtmlFmtFlt.hxx"
#include "PictToBmpFlt.hxx"
#include "com/sun/star/datatransfer/UnsupportedFlavorException.hpp"
#include "com/sun/star/datatransfer/XMimeContentType.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/Sequence.hxx"

#include <rtl/ustring.hxx>
#include <rtl/memory.h>
#include <osl/endian.h>

#include <vector>
#include <stdio.h>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

using namespace ::com::sun::star::datatransfer;
using namespace rtl;
using namespace ::com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace cppu;
using namespace std;

namespace // private
{
  const Type CPPUTYPE_SEQINT8  = getCppuType((Sequence<sal_Int8>*)0);
  const Type CPPUTYPE_OUSTRING = getCppuType( (OUString*)0 );

  /* Determine whether or not a DataFlavor is valid. 
   */
  bool isValidFlavor(const DataFlavor& aFlavor)
  {
	size_t len = aFlavor.MimeType.getLength();
	Type dtype = aFlavor.DataType;
	return ((len > 0) && ((dtype == CPPUTYPE_SEQINT8) || (dtype == CPPUTYPE_OUSTRING)));
  }

  typedef vector<sal_Unicode> UnicodeBuffer;

  OUString NSStringToOUString(NSString* cfString)
  {
	BOOST_ASSERT(cfString && "Invalid parameter");

	const char* utf8Str = [cfString UTF8String];
	unsigned int len = rtl_str_getLength(utf8Str);

	return OUString(utf8Str, len, RTL_TEXTENCODING_UTF8);
  }

  NSString* OUStringToNSString(const OUString& ustring)
  {
	OString utf8Str = OUStringToOString(ustring, RTL_TEXTENCODING_UTF8);
	return [NSString stringWithCString: utf8Str.getStr() encoding: NSUTF8StringEncoding];
  }


  const NSString* PBTYPE_UT16 = @"CorePasteboardFlavorType 0x75743136";
  const NSString* PBTYPE_PICT = @"CorePasteboardFlavorType 0x50494354";
  const NSString* PBTYPE_HTML = @"CorePasteboardFlavorType 0x48544D4C";
  const NSString* PBTYPE_SODX = @"application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
  const NSString* PBTYPE_SESX = @"application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const NSString* PBTYPE_SLSDX = @"application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"";
  const NSString* PBTYPE_ESX = @"application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const NSString* PBTYPE_LSX = @"application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"";
  const NSString* PBTYPE_EOX = @"application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"";
  const NSString* PBTYPE_SVXB = @"application/x-openoffice-svbx;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"";
  const NSString* PBTYPE_GDIMF = @"application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"";
  const NSString* PBTYPE_WMF = @"application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
  const NSString* PBTYPE_EMF = @"application/x-openoffice-emf;windows_formatname=\"Image EMF\"";

  const NSString* PBTYPE_DUMMY_INTERNAL = @"application/x-openoffice-internal";

  const char* FLAVOR_SODX = "application/x-openoffice-objectdescriptor-xml;windows_formatname=\"Star Object Descriptor (XML)\"";
  const char* FLAVOR_SESX = "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const char* FLAVOR_SLSDX = "application/x-openoffice-linksrcdescriptor-xml;windows_formatname=\"Star Link Source Descriptor (XML)\"";
  const char* FLAVOR_ESX = "application/x-openoffice-embed-source-xml;windows_formatname=\"Star Embed Source (XML)\"";
  const char* FLAVOR_LSX = "application/x-openoffice-link-source-xml;windows_formatname=\"Star Link Source (XML)\"";
  const char* FLAVOR_EOX = "application/x-openoffice-embedded-obj-xml;windows_formatname=\"Star Embedded Object (XML)\"";
  const char* FLAVOR_SVXB = "application/x-openoffice-svbx;windows_formatname=\"SVXB (StarView Bitmap/Animation)\"";
  const char* FLAVOR_GDIMF = "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"";
  const char* FLAVOR_WMF = "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"";
  const char* FLAVOR_EMF = "application/x-openoffice-emf;windows_formatname=\"Image EMF\"";
  
  const char* FLAVOR_DUMMY_INTERNAL = "application/x-openoffice-internal";


  struct FlavorMap
  {
	NSString* SystemFlavor;
	const char* OOoFlavor;
	const char* HumanPresentableName;
	Type DataType;
  };

  /* At the moment it appears as if only MS Office pastes "public.html" to the clipboard. 
   */
  FlavorMap flavorMap[] =
	{
	  { NSStringPboardType, "text/plain;charset=utf-16", "Unicode Text (UTF-16)", CPPUTYPE_OUSTRING },
	  { NSRTFPboardType, "text/richtext", "Rich Text Format", CPPUTYPE_SEQINT8 },
	  { NSTIFFPboardType, "image/bmp", "Windows Bitmap", CPPUTYPE_SEQINT8 },
	  { NSPICTPboardType, "image/bmp", "Windows Bitmap", CPPUTYPE_SEQINT8 },
	  { NSHTMLPboardType, "text/html", "Plain Html", CPPUTYPE_SEQINT8 },
	  { NSFilenamesPboardType, "application/x-openoffice-filelist;windows_formatname=\"FileList\"", "FileList", CPPUTYPE_SEQINT8 }, 
	  { PBTYPE_SESX, FLAVOR_SESX, "Star Embed Source (XML)", CPPUTYPE_SEQINT8 },
	  { PBTYPE_SLSDX, FLAVOR_SLSDX, "Star Link Source Descriptor (XML)", CPPUTYPE_SEQINT8 },
	  { PBTYPE_ESX, FLAVOR_ESX, "Star Embed Source (XML)", CPPUTYPE_SEQINT8 },
	  { PBTYPE_LSX, FLAVOR_LSX, "Star Link Source (XML)", CPPUTYPE_SEQINT8 },
	  { PBTYPE_EOX, FLAVOR_EOX, "Star Embedded Object (XML)", CPPUTYPE_SEQINT8 },
	  { PBTYPE_SVXB, FLAVOR_SVXB, "SVXB (StarView Bitmap/Animation", CPPUTYPE_SEQINT8 },
	  { PBTYPE_GDIMF, FLAVOR_GDIMF, "GDIMetaFile", CPPUTYPE_SEQINT8 },
	  { PBTYPE_WMF, FLAVOR_WMF, "Windows MetaFile", CPPUTYPE_SEQINT8 },
	  { PBTYPE_EMF, FLAVOR_EMF, "Windows Enhanced MetaFile", CPPUTYPE_SEQINT8 },
	  { PBTYPE_SODX, FLAVOR_SODX, "Star Object Descriptor (XML)", CPPUTYPE_SEQINT8 },
      { PBTYPE_DUMMY_INTERNAL, FLAVOR_DUMMY_INTERNAL, "internal data",CPPUTYPE_SEQINT8 }
	};


  #define SIZE_FLAVOR_MAP (sizeof(flavorMap)/sizeof(FlavorMap))


  inline bool isByteSequenceType(const Type& theType)
  {
	return (theType == CPPUTYPE_SEQINT8);
  }

  inline bool isOUStringType(const Type& theType)
  {
	return (theType == CPPUTYPE_OUSTRING);
  }

} // namespace private


//###########################

/* A base class for other data provider. 
 */
class DataProviderBaseImpl : public DataProvider
{
public:
  DataProviderBaseImpl(const Any& data);
  DataProviderBaseImpl(id data);
  virtual ~DataProviderBaseImpl();

protected:
  Any mData;
  //NSData* mSystemData;
  id mSystemData;
};

DataProviderBaseImpl::DataProviderBaseImpl(const Any& data) :
  mData(data),
  mSystemData(nil)
{
}

DataProviderBaseImpl::DataProviderBaseImpl(id data) : 
  mSystemData(data)
{
  [mSystemData retain];
}


DataProviderBaseImpl::~DataProviderBaseImpl()
{
  if (mSystemData)
	{
	  [mSystemData release];
	}
}

//#################################

class UniDataProvider : public DataProviderBaseImpl
{
public:
  UniDataProvider(const Any& data);
  
  UniDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

UniDataProvider::UniDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

UniDataProvider::UniDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}

NSData* UniDataProvider::getSystemData()
{
  OUString ustr;
  mData >>= ustr;

  OString strUtf8;
  ustr.convertToString(&strUtf8, RTL_TEXTENCODING_UTF8, OUSTRING_TO_OSTRING_CVTFLAGS);

  return [NSData dataWithBytes: strUtf8.getStr() length: strUtf8.getLength()];
}

Any UniDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
	{
	  oOOData = makeAny(OUString(reinterpret_cast<const sal_Char*>([mSystemData bytes]), 
								 [mSystemData length], 
								 RTL_TEXTENCODING_UTF8));
	}
  else
	{
	  oOOData = mData;
	}

  return oOOData;
}

//###########################

class ByteSequenceDataProvider : public DataProviderBaseImpl
{
public:
  ByteSequenceDataProvider(const Any& data);

  ByteSequenceDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

ByteSequenceDataProvider::ByteSequenceDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

ByteSequenceDataProvider::ByteSequenceDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}


NSData* ByteSequenceDataProvider::getSystemData()
{
   Sequence<sal_Int8> rawData;
   mData >>= rawData;

   return [NSData dataWithBytes: rawData.getArray()	length: rawData.getLength()];
}

Any ByteSequenceDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
	{
	  unsigned int flavorDataLength = [mSystemData length];
	  Sequence<sal_Int8> byteSequence;
	  byteSequence.realloc(flavorDataLength);
	  memcpy(byteSequence.getArray(), [mSystemData bytes], flavorDataLength);
	  oOOData = makeAny(byteSequence);
	}
  else
	{
	  oOOData =  mData;
	}
  
  return oOOData;
}


//###########################

class HTMLFormatDataProvider : public DataProviderBaseImpl
{
public:
  HTMLFormatDataProvider(const Any& data);

  HTMLFormatDataProvider(NSData* data);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

HTMLFormatDataProvider::HTMLFormatDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

HTMLFormatDataProvider::HTMLFormatDataProvider(NSData* data) :
  DataProviderBaseImpl(data)
{
}

NSData* HTMLFormatDataProvider::getSystemData()
{
  Sequence<sal_Int8> textHtmlData;
  mData >>= textHtmlData;

  Sequence<sal_Int8> htmlFormatData = TextHtmlToHTMLFormat(textHtmlData);

  return [NSData dataWithBytes: htmlFormatData.getArray() length: htmlFormatData.getLength()];
}

Any HTMLFormatDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
	{
	  unsigned int flavorDataLength = [mSystemData length];
	  Sequence<sal_Int8> unkHtmlData;

	  unkHtmlData.realloc(flavorDataLength);
	  memcpy(unkHtmlData.getArray(), [mSystemData bytes], flavorDataLength);

	  Sequence<sal_Int8>* pPlainHtml = &unkHtmlData;
	  Sequence<sal_Int8> plainHtml;

	  if (isHTMLFormat(unkHtmlData))
		{
		  plainHtml = HTMLFormatToTextHtml(unkHtmlData);
		  pPlainHtml = &plainHtml;
		}
	  
	  oOOData = makeAny(*pPlainHtml);
	}
  else
	{
	  oOOData = mData;
	}
  
  return oOOData;
}

//###########################

class BMPDataProvider : public DataProviderBaseImpl
{
    NSBitmapImageFileType meImageType;
public:
  BMPDataProvider(const Any& data, NSBitmapImageFileType eImageType );

  BMPDataProvider(NSData* data, NSBitmapImageFileType eImageType);

  virtual NSData* getSystemData();

  virtual Any getOOoData();
};

BMPDataProvider::BMPDataProvider(const Any& data, NSBitmapImageFileType eImageType) :
  DataProviderBaseImpl(data),
  meImageType( eImageType )
{
}

BMPDataProvider::BMPDataProvider(NSData* data, NSBitmapImageFileType eImageType) :
  DataProviderBaseImpl(data),
  meImageType( eImageType )
{
}

NSData* BMPDataProvider::getSystemData()
{
  Sequence<sal_Int8> bmpData;
  mData >>= bmpData;

  Sequence<sal_Int8> pictData;
  NSData* sysData = NULL;

  if (BMPToImage(bmpData, pictData, meImageType))
	{
	  sysData = [NSData dataWithBytes: pictData.getArray() length: pictData.getLength()];
	}

  return sysData;
}

/* At the moment the OOo 'PCT' filter is not good enough to be used
   and there is no flavor defined for exchanging 'PCT' with OOo so
   we will at the moment convert 'PCT' to a Windows BMP and provide
   this to OOo 
*/
Any BMPDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
	{
	  unsigned int flavorDataLength = [mSystemData length];
	  Sequence<sal_Int8> pictData(flavorDataLength);

	  memcpy(pictData.getArray(), [mSystemData bytes], flavorDataLength);

	  Sequence<sal_Int8> bmpData;
	  
	  if (ImageToBMP(pictData, bmpData, meImageType))
		{
		  oOOData = makeAny(bmpData);
		}
	}
  else
	{
	  oOOData = mData;
	}
  
  return oOOData;
}

//######################

class FileListDataProvider : public DataProviderBaseImpl
{
public:
  FileListDataProvider(const Any& data);
  FileListDataProvider(NSArray* data);

  virtual NSData* getSystemData();
  virtual Any getOOoData();
};

FileListDataProvider::FileListDataProvider(const Any& data) :
  DataProviderBaseImpl(data)
{
}

FileListDataProvider::FileListDataProvider(NSArray* data) :
  DataProviderBaseImpl(data)
{
}

NSData* FileListDataProvider::getSystemData()
{
  return [NSData data];
}

Any FileListDataProvider::getOOoData()
{
  Any oOOData;

  if (mSystemData)
	{
	  size_t length = [mSystemData count];
	  size_t lenSeqRequired = 0;

	  for (size_t i = 0; i < length; i++)
		{
		  NSString* fname = [mSystemData objectAtIndex: i];
		  lenSeqRequired += [fname maximumLengthOfBytesUsingEncoding: NSUnicodeStringEncoding] + sizeof(unichar);
		}

	  Sequence<sal_Int8> oOOFileList(lenSeqRequired);
	  unichar* pBuffer = reinterpret_cast<unichar*>(oOOFileList.getArray());
	  rtl_zeroMemory(pBuffer, lenSeqRequired);
	  
	  for (size_t i = 0; i < length; i++)
		{
		  NSString* fname = [mSystemData objectAtIndex: i];
		  [fname getCharacters: pBuffer];
		  size_t l = [fname length];
		  pBuffer += l + 1;
		}

	  oOOData = makeAny(oOOFileList);
	}
  else
	{
	  oOOData = mData;
	}
  
  return oOOData;
}

//###########################

DataFlavorMapper::DataFlavorMapper()
{
    Reference<XMultiServiceFactory> mrServiceManager = vcl::unohelper::GetMultiServiceFactory();
    mrXMimeCntFactory = Reference<XMimeContentTypeFactory>(mrServiceManager->createInstance(
	   OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.datatransfer.MimeContentTypeFactory"))), UNO_QUERY);

  if (!mrXMimeCntFactory.is())
	throw RuntimeException(OUString(RTL_CONSTASCII_USTRINGPARAM("AquaClipboard: Cannot create com.sun.star.datatransfer.MimeContentTypeFactory")), NULL);
}

DataFlavorMapper::~DataFlavorMapper()
{
    // release potential NSStrings
    for( OfficeOnlyTypes::iterator it = maOfficeOnlyTypes.begin(); it != maOfficeOnlyTypes.end(); ++it )
    {
        [it->second release];
        it->second = nil;
    }    
}

DataFlavor DataFlavorMapper::systemToOpenOfficeFlavor(NSString* systemDataFlavor) const
{
  DataFlavor oOOFlavor;

  for (size_t i = 0; i < SIZE_FLAVOR_MAP; i++)
	{
	  if ([systemDataFlavor caseInsensitiveCompare: flavorMap[i].SystemFlavor] == NSOrderedSame)
		{
		  oOOFlavor.MimeType = OUString::createFromAscii(flavorMap[i].OOoFlavor);
		  oOOFlavor.HumanPresentableName = OUString(RTL_CONSTASCII_USTRINGPARAM(flavorMap[i].HumanPresentableName));
		  oOOFlavor.DataType = flavorMap[i].DataType;
		  return oOOFlavor;
		}
	} // for

	// look if this might be an internal type; if it comes in here it must have
	// been through openOfficeToSystemFlavor before, so it should then be in the map
	rtl::OUString aTryFlavor( NSStringToOUString( systemDataFlavor ) );
	if( maOfficeOnlyTypes.find( aTryFlavor ) != maOfficeOnlyTypes.end() )
	{
	    oOOFlavor.MimeType = aTryFlavor;
	    oOOFlavor.HumanPresentableName = rtl::OUString();
	    oOOFlavor.DataType = CPPUTYPE_SEQINT8;
	}
	
	return oOOFlavor;
}

NSString* DataFlavorMapper::openOfficeToSystemFlavor(const DataFlavor& oOOFlavor) const
{
    NSString* sysFlavor = NULL;
    
	for( size_t i = 0; i < SIZE_FLAVOR_MAP; ++i )
	{
	    if (oOOFlavor.MimeType.compareToAscii(flavorMap[i].OOoFlavor, strlen(flavorMap[i].OOoFlavor)) == 0)
		{
		    sysFlavor = flavorMap[i].SystemFlavor;
		}
	}
	
	if( ! sysFlavor )
	{
		OfficeOnlyTypes::const_iterator it = maOfficeOnlyTypes.find( oOOFlavor.MimeType );
		if( it == maOfficeOnlyTypes.end() )
			sysFlavor = maOfficeOnlyTypes[ oOOFlavor.MimeType ] = OUStringToNSString( oOOFlavor.MimeType );
		else
			sysFlavor = it->second;
	}
	
	return sysFlavor;
}

NSString* DataFlavorMapper::openOfficeImageToSystemFlavor(NSPasteboard* pPasteboard) const
{
    NSArray *supportedTypes = [NSArray arrayWithObjects: NSTIFFPboardType, NSPICTPboardType, nil];
    NSString *sysFlavor = [pPasteboard availableTypeFromArray:supportedTypes];
    return sysFlavor;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(NSString* systemFlavor, Reference<XTransferable> rTransferable) const
{
  DataProviderPtr_t dp;

  try
	{
	  DataFlavor oOOFlavor = systemToOpenOfficeFlavor(systemFlavor);

	  Any data = rTransferable->getTransferData(oOOFlavor);
	  
	  if (isByteSequenceType(data.getValueType()))
		{
		  /*
		     the HTMLFormatDataProvider prepends segment information to HTML
		     this is useful for exchange with MS Word (which brings this stuff from Windows)
		     but annoying for other applications. Since this extension is not a standard datatype
		     on the Mac, let us not provide but provide normal HTML
		     
		  if ([systemFlavor caseInsensitiveCompare: NSHTMLPboardType] == NSOrderedSame)
			{
			  dp = DataProviderPtr_t(new HTMLFormatDataProvider(data));
			}
		  else
		  */
		  if ([systemFlavor caseInsensitiveCompare: NSPICTPboardType] == NSOrderedSame)
			{
			  dp = DataProviderPtr_t(new BMPDataProvider(data, PICTImageFileType));
			}
		  else if ([systemFlavor caseInsensitiveCompare: NSTIFFPboardType] == NSOrderedSame)
			{
			  dp = DataProviderPtr_t(new BMPDataProvider(data, NSTIFFFileType));
			}
		  else if ([systemFlavor caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
			{
			  dp = DataProviderPtr_t(new FileListDataProvider(data));
			}
		  else
			{
			  dp = DataProviderPtr_t(new ByteSequenceDataProvider(data));
			}
		}
	  else // Must be OUString type
		{
		  BOOST_ASSERT(isOUStringType(data.getValueType()));
		  dp = DataProviderPtr_t(new UniDataProvider(data));
		}
	}
  catch(UnsupportedFlavorException&)
	{
	  // Somebody violates the contract of the clipboard
	  // interface @see XTransferable
	}

  return dp;
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(const NSString* /*systemFlavor*/, NSArray* systemData) const
{
  return DataProviderPtr_t(new FileListDataProvider(systemData));
}

DataProviderPtr_t DataFlavorMapper::getDataProvider(const NSString* systemFlavor, NSData* systemData) const
{
  DataProviderPtr_t dp;

  if ([systemFlavor caseInsensitiveCompare: NSStringPboardType] == NSOrderedSame)
	{
	  dp = DataProviderPtr_t(new UniDataProvider(systemData));
	}
  else if ([systemFlavor caseInsensitiveCompare: NSHTMLPboardType] == NSOrderedSame)
	{
	  dp = DataProviderPtr_t(new HTMLFormatDataProvider(systemData));
	}
  else if ([systemFlavor caseInsensitiveCompare: NSPICTPboardType] == NSOrderedSame)
	{
	  dp = DataProviderPtr_t(new BMPDataProvider(systemData, PICTImageFileType));
	}
  else if ([systemFlavor caseInsensitiveCompare: NSTIFFPboardType] == NSOrderedSame)
	{
	  dp = DataProviderPtr_t(new BMPDataProvider(systemData, NSTIFFFileType));
	}
  else if ([systemFlavor caseInsensitiveCompare: NSFilenamesPboardType] == NSOrderedSame)
	{
	  //dp = DataProviderPtr_t(new FileListDataProvider(systemData));
	}
  else
	{
	  dp = DataProviderPtr_t(new ByteSequenceDataProvider(systemData));
	}

  return dp;
}

bool DataFlavorMapper::isValidMimeContentType(const rtl::OUString& contentType) const
{
  bool result = true;

  try
	{
	  Reference<XMimeContentType> xCntType(mrXMimeCntFactory->createMimeContentType(contentType));
	}
  catch( IllegalArgumentException& )
	{
	  result = false;
	}

  return result;
}

NSArray* DataFlavorMapper::flavorSequenceToTypesArray(const com::sun::star::uno::Sequence<com::sun::star::datatransfer::DataFlavor>& flavors) const
{
  sal_uInt32 nFlavors = flavors.getLength();
  NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: 1];

  for (sal_uInt32 i = 0; i < nFlavors; i++)
  {
      if( flavors[i].MimeType.compareToAscii( "image/bmp", 9 ) == 0 )
      {
          [array addObject: NSTIFFPboardType];
          [array addObject: NSPICTPboardType];
      }
      else
      {
          NSString* str = openOfficeToSystemFlavor(flavors[i]);
          
          if (str != NULL)
          {
              [str retain];
              [array addObject: str];
          }
      }
  }

   // #i89462# #i90747#
   // in case no system flavor was found to report
   // report at least one so D&D between OOo targets works 
  if( [array count] == 0 )
  {
      [array addObject: PBTYPE_DUMMY_INTERNAL];
  }

  return [array autorelease];
}

com::sun::star::uno::Sequence<com::sun::star::datatransfer::DataFlavor> DataFlavorMapper::typesArrayToFlavorSequence(NSArray* types) const
{
  int nFormats = [types count];
  Sequence<DataFlavor> flavors;

  for (int i = 0; i < nFormats; i++)
	{
	  NSString* sysFormat = [types objectAtIndex: i];
	  DataFlavor oOOFlavor = systemToOpenOfficeFlavor(sysFormat);

	  if (isValidFlavor(oOOFlavor))
		{
		  flavors.realloc(flavors.getLength() + 1);
		  flavors[flavors.getLength() - 1] = oOOFlavor;
		}
	}

  return flavors;
}


NSArray* DataFlavorMapper::getAllSupportedPboardTypes() const
{
  NSMutableArray* array = [[NSMutableArray alloc] initWithCapacity: SIZE_FLAVOR_MAP];

  for (sal_uInt32 i = 0; i < SIZE_FLAVOR_MAP; i++)
	{
	  [array addObject: flavorMap[i].SystemFlavor];
	}

  return [array autorelease];
}
