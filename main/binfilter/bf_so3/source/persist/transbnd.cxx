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



#define _TRANSBND_CXX "$Revision: 1.5 $"

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.h>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <bf_svtools/bf_solar.h>
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _ERRCODE_HXX
#include <tools/errcode.hxx>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif
#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _SFXCANCEL_HXX
#include <bf_svtools/cancel.hxx>
#endif
#ifndef _INETHIST_HXX
#include <bf_svtools/inethist.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <bf_svtools/svarray.hxx>
#endif

#ifndef _BINDING_HXX
#include <bf_so3/binding.hxx>
#endif
#ifndef _BINDDATA_HXX
#include <binddata.hxx>
#endif
#ifndef _TRANSBND_HXX
#include <bf_so3/transbnd.hxx>
#endif
#ifndef _TRANSPRT_HXX
#include <bf_so3/transprt.hxx>
#endif
#ifndef _TRANSUNO_HXX
#include <transuno.hxx>
#endif
#ifndef _SO2DEFS_HXX
#include <bf_so3/so2defs.hxx>
#endif

#include <algorithm>
 
using namespace com::sun::star::sdbc;
using namespace com::sun::star::beans;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::container;

namespace binfilter {


/*========================================================================
 *
 * SvBinding implementation.
 *
 *======================================================================*/
/*
 * SvBinding.
 */
/*
 * ~SvBinding.
 */
SvBinding::~SvBinding (void)
{
	delete m_pTransport;
	delete m_pCancelable;
}

/*
 * OnStart.
 */
void SvBinding::OnStart (void)
{
	SvBindingRef xThis (this);
	if (m_xCallback.Is())
	{
		vos::OGuard aAppGuard (Application::GetSolarMutex());
		if (m_xCallback.Is())
			m_xCallback->InitStartTime();
	}
}

/*
 * OnError.
 */
void SvBinding::OnError (ErrCode eErrCode)
{
	SvBindingRef xThis (this);
	m_eErrCode = eErrCode;

	if (m_xCallback.Is())
	{
		vos::OGuard aAppGuard (Application::GetSolarMutex());
		if (m_xCallback.Is())
			m_xCallback->OnStopBinding (m_eErrCode, String());
	}

	DELETEZ (m_pTransport);
	DELETEZ (m_pCancelable);
}

/*
 * OnMimeAvailable.
 */
void SvBinding::OnMimeAvailable (const String &rMime)
{
	m_aMime = rMime;
	m_bMimeAvail = TRUE;
}

/*
 * OnExpiresAvailable.
 */
void SvBinding::OnExpiresAvailable (const DateTime &rExpires)
{
	m_aExpires = rExpires;
}

/*
 * OnHeaderAvailable.
 */
void SvBinding::OnHeaderAvailable (const String &rName, const String &rValue)
{
	if (!m_xHeadIter.Is())
		m_xHeadIter = new SvKeyValueIterator;

	m_xHeadIter->Append (SvKeyValue (rName, rValue));
}

/*
 * OnDataAvailable.
 */
void SvBinding::OnDataAvailable (
	SvStatusCallbackType eType, ULONG nSize, SvLockBytes *pLockBytes)
{
	SvBindingRef xThis (this);

	if (!m_xLockBytes.Is())
		m_xLockBytes = pLockBytes;

	switch (eType)
	{
		case SVBSCF_FIRSTDATANOTIFICATION:
		case SVBSCF_INTERMEDIATEDATANOTIFICATION:
			if (m_bMimeAvail && m_xLockBytes.Is() && nSize)
			{
				vos::IMutex &rAppMutex = Application::GetSolarMutex();
				if (m_xCallback.Is() && rAppMutex.tryToAcquire())
				{
					m_xCallback->OnDataAvailable (
						eType, nSize, *m_xLockBytes);
					rAppMutex.release();
				}
			}
			break;

		case SVBSCF_LASTDATANOTIFICATION:
			m_bComplete = TRUE;
			OnError (ERRCODE_NONE);
			break;

		default: // Ignored.
			break;
	}
}

/*
 * OnProgress.
 */
void SvBinding::OnProgress (
	ULONG nNow, ULONG nEnd, SvBindStatus eStat)
{
	SvBindingRef xThis (this);
	if (m_xCallback.Is())
	{
		vos::IMutex &rAppMutex = Application::GetSolarMutex();
		if (m_xCallback.Is() && rAppMutex.tryToAcquire())
		{
			m_xCallback->OnProgress (
				nNow, nEnd, eStat, m_aUrlObj.GetMainURL( INetURLObject::DECODE_TO_IURI ));
			rAppMutex.release();
		}
	}
}

/*
 * OnRedirect.
 */
void SvBinding::OnRedirect (const String &rUrl)
{
	SvBindingRef xThis (this);
	if (m_xCallback.Is())
	{
		vos::OGuard aAppGuard (Application::GetSolarMutex());

		INetURLHistory::GetOrCreate()->PutUrl (m_aUrlObj);
		m_aUrlObj.SetURL (rUrl);

		if (m_xCallback.Is())
			m_xCallback->OnProgress (0, 0, SVBINDSTATUS_REDIRECTING, rUrl);
	}
}

/*
 * ShouldUseFtpProxy.
 */
BOOL SvBinding::ShouldUseFtpProxy (const String &rUrl)
{
	return BAPP()->ShouldUseFtpProxy (rUrl);
}

/*
 * ~SvBindStatusCallback.
 */
SvBindStatusCallback::~SvBindStatusCallback (void)
{
}

/*
 * SetProgressCallback.
 */
Link SvBindStatusCallback::m_aProgressCallback;

void SvBindStatusCallback::SetProgressCallback (const Link &rLink)
{
	m_aProgressCallback = rLink;
}

/*
 * InitStartTime.
 */
void SvBindStatusCallback::InitStartTime (void)
{
	m_nStartTicks = Time::GetSystemTicks();
}

/*
 * OnProgress.
 */
void SvBindStatusCallback::OnProgress (
	ULONG nNow, ULONG nMax, SvBindStatus eStat, const String& rStatusText)
{
	ULONG nTicks = std::max((Time::GetSystemTicks() - m_nStartTicks), (ULONG) 1);

	SvProgressArg aArg (rStatusText);
	aArg.nProgress = nNow;
	aArg.nMax      = nMax;
	aArg.eStatus   = eStat;
	aArg.nRate     = (float)((nNow * 1000.0) / nTicks);

	m_aProgressCallback.Call (&aArg);
}

/*
 * OnDataAvailable (SvLockBytes).
 */
void SvBindStatusCallback::OnDataAvailable (
	SvStatusCallbackType eType, ULONG, SvLockBytes&)
{
	SvBindStatusCallbackRef xThis (this);

	if (!m_bInAvailableCall)
	{
		do
		{
			m_bInAvailableCall = TRUE;

			m_bReloadPending |=
				(eType == SVBSCF_RELOADAVAILABLENOTIFICATION);
			if (m_bReloadPending)
			{
				m_bReloadPending = FALSE;
				m_aReloadLink.Call (this);
			}

			m_bPartPending |=
				(eType == SVBSCF_NEWPARTAVAILABLENOTIFICATION);
			if (m_bPartPending)
			{
				m_bPartPending = FALSE;
				m_aPartLink.Call (this);
			}

			m_bDataPending |=
				((eType == SVBSCF_FIRSTDATANOTIFICATION) ||
				 (eType == SVBSCF_LASTDATANOTIFICATION ) ||
				 (eType == SVBSCF_INTERMEDIATEDATANOTIFICATION));
			if (m_bDataPending)
			{
				m_bDataPending = FALSE;
				m_aDataLink.Call (this);
			}

			m_bInAvailableCall = FALSE;
		} while (m_bDataPending || m_bPartPending || m_bReloadPending);
	}
	else
	{
		switch (eType)
		{
			case SVBSCF_RELOADAVAILABLENOTIFICATION:
				m_bReloadPending = TRUE;
				break;

			case SVBSCF_NEWPARTAVAILABLENOTIFICATION:
				m_bPartPending = TRUE;
				break;

			default:
				m_bDataPending = TRUE;
				break;
		}
	}

	if (m_bDonePending)
	{
		m_bDonePending = FALSE;
		m_aDoneLink.Call (this);
	}
}

/*
 * OnDataAvailable (SvStream).
 */
void SvBindStatusCallback::OnDataAvailable (
	SvStatusCallbackType eType, ULONG, SvStream&)
{
	SvLockBytes aLB(NULL);
	OnDataAvailable (eType, 0, aLB);
}

/*
 * OnStopBinding.
 */
void SvBindStatusCallback::OnStopBinding (
	ErrCode eErrCode, const String &rStatusText)
{
	(void)eErrCode;
	(void)rStatusText;

	if (!m_bInAvailableCall)
		m_aDoneLink.Call (this);
	else
		m_bDonePending = TRUE;
}

/*========================================================================
 *
 * SvKeyValueIterator implementation.
 *
 *======================================================================*/
SV_DECL_PTRARR_DEL(SvKeyValueList_Impl, SvKeyValue*, 0, 4)
SV_IMPL_PTRARR(SvKeyValueList_Impl, SvKeyValue*);

/*
 * SvKeyValueIterator.
 */
SvKeyValueIterator::SvKeyValueIterator (void)
	: m_pList (new SvKeyValueList_Impl),
	  m_nPos  (0)
{
}

/*
 * ~SvKeyValueIterator.
 */
SvKeyValueIterator::~SvKeyValueIterator (void)
{
	delete m_pList;
}

/*
 * GetFirst.
 */
BOOL SvKeyValueIterator::GetFirst (SvKeyValue &rKeyVal)
{
	m_nPos = m_pList->Count();
	return GetNext (rKeyVal);
}

/*
 * GetNext.
 */
BOOL SvKeyValueIterator::GetNext (SvKeyValue &rKeyVal)
{
	if (m_nPos > 0)
	{
		rKeyVal = *m_pList->GetObject(--m_nPos);
		return TRUE;
	}
	else
	{
		// Nothing to do.
		return FALSE;
	}
}

/*
 * Append.
 */
void SvKeyValueIterator::Append (const SvKeyValue &rKeyVal)
{
	SvKeyValue *pKeyVal = new SvKeyValue (rKeyVal);
	m_pList->C40_INSERT(SvKeyValue, pKeyVal, m_pList->Count());
}

}
