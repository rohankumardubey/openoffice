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



#ifndef _DBG_LAY_HXX
#define _DBG_LAY_HXX

#include <bf_svtools/bf_solar.h>


#define PROT_FILE_INIT  0x00000000
#define PROT_INIT		0x00000001
#define PROT_MAKEALL	0x00000002
#define PROT_MOVE_FWD	0x00000004
#define PROT_MOVE_BWD	0x00000008
#define PROT_GROW		0x00000010
#define PROT_SHRINK		0x00000020
#define PROT_GROW_TST	0x00000040
#define PROT_SHRINK_TST	0x00000080
#define PROT_SIZE		0x00000100
#define PROT_PRTAREA	0x00000200
#define PROT_POS		0x00000400
#define PROT_ADJUSTN	0x00000800
#define PROT_SECTION	0x00001000
#define PROT_CUT		0x00002000
#define PROT_PASTE		0x00004000
#define PROT_LEAF		0x00008000
#define PROT_TESTFORMAT	0x00010000
#define PROT_FRMCHANGES	0x00020000
#define PROT_SNAPSHOT   0x00040000

#define ACT_START			1
#define ACT_END         	2
#define ACT_CREATE_MASTER   3
#define ACT_CREATE_FOLLOW   4
#define ACT_DEL_MASTER   	5
#define ACT_DEL_FOLLOW   	6
#define ACT_MERGE			7
#define ACT_NEXT_SECT		8
#define ACT_PREV_SECT		9

#define SNAP_LOWER       0x00000001
#define SNAP_FLYFRAMES   0x00000002
#define SNAP_TABLECONT   0x00000004

#ifdef DBG_UTIL

#include <tools/debug.hxx>
namespace binfilter {
class SwImplProtocol;
class SwFrm;
class SwImplEnterLeave;

class SwProtocol
{
	static ULONG nRecord;
	static SwImplProtocol* pImpl;
	static BOOL Start() { return 0 != ( PROT_INIT & nRecord ); }
public:
	static ULONG Record() { return nRecord; }
	static void SetRecord( ULONG nNew ) { nRecord = nNew; }
	static BOOL Record( ULONG nFunc ) { return 0 != (( nFunc | PROT_INIT ) & nRecord); }
	static void Record( const SwFrm* pFrm, ULONG nFunction, ULONG nAction, void* pParam ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 static void Record( const SwFrm* pFrm, ULONG nFunction, ULONG nAction, void* pParam );
	static void Init();
	static void Stop();
};

class SwEnterLeave
{
	SwImplEnterLeave* pImpl;
	void Ctor( const SwFrm* pFrm, ULONG nFunc, ULONG nAct, void* pPar ){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void Ctor( const SwFrm* pFrm, ULONG nFunc, ULONG nAct, void* pPar );
	void Dtor(){DBG_BF_ASSERT(0, "STRIP");} //STRIP001 void Dtor();
public:
 	SwEnterLeave( const SwFrm* pFrm, ULONG nFunc, ULONG nAct, void* pPar )
	{ if( SwProtocol::Record( nFunc ) ) Ctor( pFrm, nFunc, nAct, pPar ); else pImpl = NULL; }
	~SwEnterLeave() { if( pImpl ) Dtor(); }
};

#define PROTOCOL( pFrm, nFunc, nAct, pPar ) { 	if( SwProtocol::Record( nFunc ) )\
													SwProtocol::Record( pFrm, nFunc, nAct, pPar ); }
#define PROTOCOL_INIT SwProtocol::Init();
#define PROTOCOL_STOP SwProtocol::Stop();
#define PROTOCOL_ENTER( pFrm, nFunc, nAct, pPar ) SwEnterLeave aEnter( pFrm, nFunc, nAct, pPar );
#define PROTOCOL_SNAPSHOT( pFrm, nFlags ) SwProtocol::SnapShot( pFrm, nFlags );
#define GET_VARIABLE( nNo, nVar ) SwProtocol::GetVar( nNo, nVar );
} //STRIP008 end of namespace binfilter
#else
namespace binfilter {
#define PROTOCOL( pFrm, nFunc, nAct, pPar )
#define PROTOCOL_INIT
#define PROTOCOL_STOP
#define PROTOCOL_ENTER( pFrm, nFunc, nAct, pPar )
#define PROTOCOL_SNAPSHOT( pFrm, nFlags )
#define GET_VARIABLE( nNo, nVar )
} //namespace binfilter
#endif

#endif
