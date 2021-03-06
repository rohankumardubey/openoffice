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

#include <precomp.h>
#include <s2_luidl/pe_singl.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_singleton.hxx>
#include <ary/idl/i_sisingleton.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/pe_type2.hxx>
#include <s2_luidl/tk_keyw.hxx>
#include <s2_luidl/tk_ident.hxx>
#include <s2_luidl/tk_punct.hxx>



namespace csi
{
namespace uidl
{



#if 0
#ifdef DF
#undef DF
#endif
#define DF 	&PE_Singleton::On_Default


PE_Singleton::F_TOK
PE_Singleton::aDispatcher[PE_Singleton::e_STATES_MAX][PE_Singleton::tt_MAX] =
		{ 	{ DF, DF, DF },  // e_none
			{ DF, &PE_Singleton::On_need_name_Identifer,
					  DF },  // need_name
			{ DF, DF, &PE_Singleton::On_need_curlbr_open_Punctuation,
						 },  // need_curlbr_open
			{ &PE_Singleton::On_std_GotoService,
				  DF, &PE_Singleton::On_std_Punctuation,
						 },  // e_std
			{ DF, DF, DF },  // in_service
			{ DF, DF, &PE_Interface::On_need_finish_Punctuation,
						 }   // need_finish
		};
#endif // 0


PE_Singleton::PE_Singleton()
	:	eState(e_none),
		sData_Name(),
		bIsPreDeclaration(false),
		pCurSingleton(0),
        pCurSiSingleton(0),
		pPE_Type(0),
		nCurParsed_Type(0)
{
	pPE_Type 		= new PE_Type(nCurParsed_Type);
}

void
PE_Singleton::EstablishContacts( UnoIDL_PE *				io_pParentPE,
							   ary::Repository &		io_rRepository,
							   TokenProcessing_Result & o_rResult )
{
	UnoIDL_PE::EstablishContacts(io_pParentPE,io_rRepository,o_rResult);
	pPE_Type->EstablishContacts(this,io_rRepository,o_rResult);
}

PE_Singleton::~PE_Singleton()
{
}

void
PE_Singleton::ProcessToken( const Token & i_rToken )
{
	i_rToken.Trigger(*this);
}


void
PE_Singleton::Process_MetaType( const TokMetaType &	i_rToken )
{
	switch ( i_rToken.Id() )
	{
		case TokMetaType::mt_service:
					if (eState == e_std)
					{
						SetResult(done, push_sure, pPE_Type.Ptr());
						eState = in_service;
					}
					else
						On_Default();
					break;
		case TokMetaType::mt_singleton:
					if (eState == need_name)
						SetResult(done, stay);
					else
						On_Default();
					break;
        default:
            // KORR_FUTURE
            // Should throw syntax error warning
                    ;

	}	// end switch
}

void
PE_Singleton::Process_Identifier( const TokIdentifier & i_rToken )
{
	if (eState == need_name)
	{
		sData_Name = i_rToken.Text();
		SetResult(done, stay);
		eState = need_curlbr_open;
	}
	else
		On_Default();
}

void
PE_Singleton::Process_Punctuation( const TokPunctuation & i_rToken )
{
	switch (i_rToken.Id())
	{
		case TokPunctuation::CurledBracketOpen:
					if (eState == need_curlbr_open)
					{
                        pCurSingleton = &Gate().Ces().Store_Singleton(
                                                        CurNamespace().CeId(),
                                                        sData_Name );
						PassDocuAt(*pCurSingleton);
						SetResult(done, stay);
						eState = e_std;
					}
					else
						On_Default();
					break;
		case TokPunctuation::CurledBracketClose:
					if (eState == e_std)
					{
						SetResult(done, stay);
						eState = need_finish;
					}
					else
						On_Default();
					break;
		case TokPunctuation::Semicolon:
					switch (eState)
					{
					   case e_std:  SetResult(done, stay);
									break;
					   case need_finish:
									SetResult(done, pop_success);
									eState = e_none;
									break;
					   default:
									On_Default();
					}	// end switch
					break;
		case TokPunctuation::Colon:
					switch (eState)
					{
					   case need_curlbr_open:
					                SetResult(done, push_sure, pPE_Type.Ptr());
                                    eState = in_base_interface;
									break;
					   default:
									On_Default();
					}	// end switch
		            break;
		default:
					On_Default();
	}	// end switch
}

void
PE_Singleton::Process_Default()
{
	On_Default();
}


void
PE_Singleton::On_Default()
{
    SetResult(not_done, pop_failure);
}

void
PE_Singleton::InitData()
{
	eState = need_name;
	sData_Name.clear();
	bIsPreDeclaration = false;
	pCurSingleton = 0;
    pCurSiSingleton = 0;
	nCurParsed_Type = 0;
}

void
PE_Singleton::TransferData()
{
	if (NOT bIsPreDeclaration)
	{
		csv_assert(sData_Name.size() > 0);
		csv_assert( (pCurSingleton != 0) != (pCurSiSingleton != 0) );
	}

	eState = e_none;
}

void
PE_Singleton::ReceiveData()
{
    switch (eState)
    {
        case in_service:
                    pCurSingleton->Set_Service(nCurParsed_Type);
					nCurParsed_Type = 0;
					eState = e_std;
                    break;
        case in_base_interface:
                    pCurSiSingleton = &Gate().Ces().Store_SglIfcSingleton(
                                                    CurNamespace().CeId(),
                                                    sData_Name,
                                                    nCurParsed_Type );
					PassDocuAt(*pCurSiSingleton);
					nCurParsed_Type = 0;
					eState = need_finish;
                    break;
        default:
            csv_assert(false);
    }   // end switch
}

UnoIDL_PE &
PE_Singleton::MyPE()
{
	return *this;
}

}   // namespace uidl
}   // namespace csi
