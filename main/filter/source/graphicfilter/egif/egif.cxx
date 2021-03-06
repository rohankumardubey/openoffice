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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/window.hxx>
#include <svl/solar.hrc>
#include <svtools/fltcall.hxx>
#include <svtools/FilterConfigItem.hxx>
#include "giflzwc.hxx"

// -------------
// - GIFWriter -
// -------------

class GIFWriter
{
	Bitmap				aAccBmp;
	BitmapReadAccess*	pAcc;
	SvStream*			pGIF;
	sal_uLong				nMinPercent;
	sal_uLong				nMaxPercent;
	sal_uLong				nLastPercent;
	long				nActX;
	long				nActY;
	sal_Int32			nInterlaced;
	sal_Bool				bStatus;
	sal_Bool				bTransparent;

	void				MayCallback( sal_uLong nPercent );
	void				WriteSignature( sal_Bool bGIF89a );
	void				WriteGlobalHeader( const Size& rSize );
	void				WriteLoopExtension( const Animation& rAnimation );
	void				WriteLogSizeExtension( const Size& rSize100 );
	void				WriteImageExtension( long nTimer, Disposal eDisposal );
	void				WriteLocalHeader();
	void				WritePalette();
	void				WriteAccess();
	void				WriteTerminator();

	sal_Bool				CreateAccess( const BitmapEx& rBmpEx );
	void				DestroyAccess();

	void				WriteAnimation( const Animation& rAnimation );
	void				WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint, sal_Bool bExtended,
									   long nTimer = 0, Disposal eDisposal = DISPOSE_NOT );

	com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > xStatusIndicator;

public:

						GIFWriter() {}
						~GIFWriter() {}

	sal_Bool				WriteGIF( const Graphic& rGraphic, SvStream& rGIF,
										FilterConfigItem* pConfigItem );
};

// ------------------------------------------------------------------------

sal_Bool GIFWriter::WriteGIF( const Graphic& rGraphic, SvStream& rGIF,
								FilterConfigItem* pFilterConfigItem )
{
	if ( pFilterConfigItem )
	{
		xStatusIndicator = pFilterConfigItem->GetStatusIndicator();
		if ( xStatusIndicator.is() )
		{
			rtl::OUString aMsg;
			xStatusIndicator->start( aMsg, 100 );
		}
	}

	Size			aSize100;
	const MapMode	aMap( rGraphic.GetPrefMapMode() );
	sal_Bool			bLogSize = ( aMap.GetMapUnit() != MAP_PIXEL );

	if( bLogSize )
		aSize100 = Application::GetDefaultDevice()->LogicToLogic( rGraphic.GetPrefSize(), aMap, MAP_100TH_MM );

	pGIF = &rGIF;
	bStatus = sal_True;
	nLastPercent = 0;
	nInterlaced = 0;
	pAcc = NULL;

	if ( pFilterConfigItem )
		nInterlaced = pFilterConfigItem->ReadInt32( String( RTL_CONSTASCII_USTRINGPARAM( "Interlaced" ) ), 0 );

	pGIF->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );

	if( rGraphic.IsAnimated() )
	{
		const Animation& rAnimation = rGraphic.GetAnimation();

		WriteSignature( sal_True );

		if ( bStatus )
		{
			WriteGlobalHeader( rAnimation.GetDisplaySizePixel() );

			if( bStatus )
			{
				WriteLoopExtension( rAnimation );

				if( bStatus )
					WriteAnimation( rAnimation );
			}
		}
	}
	else
	{
		const sal_Bool bGrafTrans = rGraphic.IsTransparent();

		BitmapEx aBmpEx;

		if( bGrafTrans )
			aBmpEx = rGraphic.GetBitmapEx();
		else
			aBmpEx = BitmapEx( rGraphic.GetBitmap() );

		nMinPercent = 0;
		nMaxPercent = 100;

		WriteSignature( bGrafTrans || bLogSize );

		if( bStatus )
		{
			WriteGlobalHeader( aBmpEx.GetSizePixel() );

			if( bStatus )
				WriteBitmapEx( aBmpEx, Point(), bGrafTrans );
		}
	}

	if( bStatus )
	{
		if( bLogSize )
			WriteLogSizeExtension( aSize100 );

		WriteTerminator();
	}

	if ( xStatusIndicator.is() )
		xStatusIndicator->end();

	return bStatus;
}

// ------------------------------------------------------------------------

void GIFWriter::WriteBitmapEx( const BitmapEx& rBmpEx, const Point& rPoint,
							   sal_Bool bExtended, long nTimer, Disposal eDisposal )
{
	if( CreateAccess( rBmpEx ) )
	{
		nActX = rPoint.X();
		nActY = rPoint.Y();

		if( bExtended )
			WriteImageExtension( nTimer, eDisposal );

		if( bStatus )
		{
			WriteLocalHeader();

			if( bStatus )
			{
				WritePalette();

				if( bStatus )
					WriteAccess();
			}
		}

		DestroyAccess();
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteAnimation( const Animation& rAnimation )
{
	const sal_uInt16	nCount = rAnimation.Count();

	if( nCount )
	{
		const double fStep = 100. / nCount;

		nMinPercent = 0L;
		nMaxPercent = (sal_uLong) fStep;

		for( sal_uInt16 i = 0; i < nCount; i++ )
		{
			const AnimationBitmap& rAnimBmp = rAnimation.Get( i );

			WriteBitmapEx( rAnimBmp.aBmpEx, rAnimBmp.aPosPix, sal_True,
						   rAnimBmp.nWait, rAnimBmp.eDisposal );
			nMinPercent = nMaxPercent;
			nMaxPercent = (sal_uLong) ( nMaxPercent + fStep );
		}
	}
}

// ------------------------------------------------------------------------

void GIFWriter::MayCallback( sal_uLong nPercent )
{
	if ( xStatusIndicator.is() )
	{
		if( nPercent >= nLastPercent + 3 )
		{
			nLastPercent = nPercent;
			if ( nPercent <= 100 )
				xStatusIndicator->setValue( nPercent );
		}
	}
}

// ------------------------------------------------------------------------

sal_Bool GIFWriter::CreateAccess( const BitmapEx& rBmpEx )
{
	if( bStatus )
	{
		Bitmap aMask( rBmpEx.GetMask() );

		aAccBmp = rBmpEx.GetBitmap();
		bTransparent = sal_False;

		if( !!aMask )
		{
			if( aAccBmp.Convert( BMP_CONVERSION_8BIT_TRANS ) )
			{
				aMask.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
				aAccBmp.Replace( aMask, BMP_COL_TRANS );
				bTransparent = sal_True;
			}
			else
				aAccBmp.Convert( BMP_CONVERSION_8BIT_COLORS );
		}
		else
			aAccBmp.Convert( BMP_CONVERSION_8BIT_COLORS );

		pAcc = aAccBmp.AcquireReadAccess();

		if( !pAcc )
			bStatus = sal_False;
	}

	return bStatus;
}

// ------------------------------------------------------------------------

void GIFWriter::DestroyAccess()
{
	aAccBmp.ReleaseAccess( pAcc );
	pAcc = NULL;
}

// ------------------------------------------------------------------------

void GIFWriter::WriteSignature( sal_Bool bGIF89a )
{
	if( bStatus )
	{
		pGIF->Write( bGIF89a ? "GIF89a" : "GIF87a" , 6 );

		if( pGIF->GetError() )
			bStatus = sal_False;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteGlobalHeader( const Size& rSize )
{
	if( bStatus )
	{
		// 256 Farben
		const sal_uInt16	nWidth = (sal_uInt16) rSize.Width();
		const sal_uInt16	nHeight = (sal_uInt16) rSize.Height();
		const sal_uInt8		cFlags = 128 | ( 7 << 4 );

		// Werte rausschreiben
		*pGIF << nWidth;
		*pGIF << nHeight;
		*pGIF << cFlags;
		*pGIF << (sal_uInt8) 0x00;
		*pGIF << (sal_uInt8) 0x00;

		// Dummy-Palette mit zwei Eintraegen (Schwarz/Weiss) schreiben;
		// dieses nur wegen Photoshop-Bug, da die keine Bilder ohne
		// globale Farbpalette lesen koennen
		*pGIF << (sal_uInt16) 0;
		*pGIF << (sal_uInt16) 255;
		*pGIF << (sal_uInt16) 65535;

		if( pGIF->GetError() )
			bStatus = sal_False;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLoopExtension( const Animation& rAnimation )
{
	DBG_ASSERT( rAnimation.Count() > 0, "Animation has no bitmaps!" );

	sal_uInt16 nLoopCount = (sal_uInt16) rAnimation.GetLoopCount();

	// falls nur ein Durchlauf stattfinden soll,
	// wird keine LoopExtension geschrieben;
	// Default ist dann immer ein Durchlauf
	if( nLoopCount != 1 )
	{
		// Netscape interpretiert den LoopCount
		// als reine Anzahl der _Wiederholungen_
		if( nLoopCount )
			nLoopCount--;

		const sal_uInt8 cLoByte = (const sal_uInt8) nLoopCount;
		const sal_uInt8 cHiByte = (const sal_uInt8) ( nLoopCount >> 8 );

		*pGIF << (sal_uInt8) 0x21;
		*pGIF << (sal_uInt8) 0xff;
		*pGIF << (sal_uInt8) 0x0b;
		pGIF->Write( "NETSCAPE2.0", 11 );
		*pGIF << (sal_uInt8) 0x03;
		*pGIF << (sal_uInt8) 0x01;
		*pGIF << cLoByte;
		*pGIF << cHiByte;
		*pGIF << (sal_uInt8) 0x00;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLogSizeExtension( const Size& rSize100 )
{
	// PrefSize in 100th-mm als ApplicationExtension schreiben
	if( rSize100.Width() && rSize100.Height() )
	{
		*pGIF << (sal_uInt8) 0x21;
		*pGIF << (sal_uInt8) 0xff;
		*pGIF << (sal_uInt8) 0x0b;
		pGIF->Write( "STARDIV 5.0", 11 );
		*pGIF << (sal_uInt8) 0x09;
		*pGIF << (sal_uInt8) 0x01;
		*pGIF << (sal_uInt32) rSize100.Width();
		*pGIF << (sal_uInt32) rSize100.Height();
		*pGIF << (sal_uInt8) 0x00;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteImageExtension( long nTimer, Disposal eDisposal )
{
	if( bStatus )
	{
		const sal_uInt16	nDelay = (sal_uInt16) nTimer;
		sal_uInt8			cFlags = 0;

		// Transparent-Flag setzen
		if( bTransparent )
			cFlags |= 1;

		// Disposal-Wert setzen
		if( eDisposal == DISPOSE_BACK )
			cFlags |= ( 2 << 2 );
		else if( eDisposal == DISPOSE_PREVIOUS )
			cFlags |= ( 3 << 2 );

		*pGIF << (sal_uInt8) 0x21;
		*pGIF << (sal_uInt8) 0xf9;
		*pGIF << (sal_uInt8) 0x04;
		*pGIF << cFlags;
		*pGIF << nDelay;
		*pGIF << (sal_uInt8) pAcc->GetBestPaletteIndex( BMP_COL_TRANS );
		*pGIF << (sal_uInt8) 0x00;

		if( pGIF->GetError() )
			bStatus = sal_False;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteLocalHeader()
{
	if( bStatus )
	{
		const sal_uInt16	nPosX = (sal_uInt16) nActX;
		const sal_uInt16	nPosY = (sal_uInt16) nActY;
		const sal_uInt16	nWidth = (sal_uInt16) pAcc->Width();
		const sal_uInt16	nHeight = (sal_uInt16) pAcc->Height();
		sal_uInt8			cFlags = (sal_uInt8) ( pAcc->GetBitCount() - 1 );

		// Interlaced-Flag setzen
		if( nInterlaced )
			cFlags |= 0x40;

		// Flag fuer lokale Farbpalette setzen
		cFlags |= 0x80;

		// alles rausschreiben
		*pGIF << (sal_uInt8) 0x2c;
		*pGIF << nPosX;
		*pGIF << nPosY;
		*pGIF << nWidth;
		*pGIF << nHeight;
		*pGIF << cFlags;

		if( pGIF->GetError() )
			bStatus = sal_False;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WritePalette()
{
	if( bStatus && pAcc->HasPalette() )
	{
		const sal_uInt16 nCount = pAcc->GetPaletteEntryCount();
		const sal_uInt16 nMaxCount = ( 1 << pAcc->GetBitCount() );

		for ( sal_uInt16 i = 0; i < nCount; i++ )
		{
			const BitmapColor& rColor = pAcc->GetPaletteColor( i );

			*pGIF << rColor.GetRed();
			*pGIF << rColor.GetGreen();
			*pGIF << rColor.GetBlue();
		}

		// Rest mit 0 auffuellen
		if( nCount < nMaxCount )
			pGIF->SeekRel( ( nMaxCount - nCount ) * 3 );

		if( pGIF->GetError() )
			bStatus = sal_False;
	}
}

// ------------------------------------------------------------------------

void GIFWriter::WriteAccess()
{
	GIFLZWCompressor	aCompressor;
	const long			nWidth = pAcc->Width();
	const long			nHeight = pAcc->Height();
	sal_uInt8*				pBuffer = NULL;
	const sal_uLong			nFormat = pAcc->GetScanlineFormat();
	long				nY;
	long				nT;
	long				i;
	sal_Bool				bNative = ( BMP_FORMAT_8BIT_PAL == nFormat );

	if( !bNative )
		pBuffer = new sal_uInt8[ nWidth ];

	if( bStatus && ( 8 == pAcc->GetBitCount() ) && pAcc->HasPalette() )
	{
		aCompressor.StartCompression( *pGIF, pAcc->GetBitCount() );

		for( i = 0; i < nHeight; i++ )
		{
			if( nInterlaced )
			{
				nY = i << 3;

				if( nY >= nHeight )
				{
					nT = i - ( ( nHeight + 7 ) >> 3 );
					nY= ( nT << 3 ) + 4;

					if( nY >= nHeight )
					{
						nT -= ( nHeight + 3 ) >> 3;
						nY = ( nT << 2 ) + 2;

						if ( nY >= nHeight )
						{
							nT -= ( ( nHeight + 1 ) >> 2 );
							nY = ( nT << 1 ) + 1;
						}
					}
				}
			}
			else
				nY = i;

			if( bNative )
				aCompressor.Compress( pAcc->GetScanline( nY ), nWidth );
			else
			{
				for( long nX = 0L; nX < nWidth; nX++ )
					pBuffer[ nX ] = (sal_uInt8) pAcc->GetPixel( nY, nX );

				aCompressor.Compress( pBuffer, nWidth );
			}

			if ( pGIF->GetError() )
				bStatus = sal_False;

			MayCallback( nMinPercent + ( nMaxPercent - nMinPercent ) * i / nHeight );

			if( !bStatus )
				break;
		}

		aCompressor.EndCompression();

		if ( pGIF->GetError() )
			bStatus = sal_False;
	}

	delete[] pBuffer;
}

// ------------------------------------------------------------------------

void GIFWriter::WriteTerminator()
{
	if( bStatus )
	{
		*pGIF << (sal_uInt8) 0x3b;

		if( pGIF->GetError() )
			bStatus = sal_False;
	}
}

// ------------------------------------------------------------------------

extern "C" sal_Bool __LOADONCALLAPI GraphicExport( SvStream& rStream, Graphic& rGraphic,
											   FilterConfigItem* pConfigItem, sal_Bool )
{
	return GIFWriter().WriteGIF( rGraphic, rStream, pConfigItem );
}

// ------------------------------------------------------------------------
