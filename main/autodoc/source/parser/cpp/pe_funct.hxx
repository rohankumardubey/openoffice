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




#ifndef ADC_CPP_PE_FUNCT_HXX
#define ADC_CPP_PE_FUNCT_HXX



// USED SERVICES
	// BASE CLASSES
#include "cpp_pe.hxx"
	// COMPONENTS
#include <semantic/callf.hxx>
#include <semantic/sub_peu.hxx>
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/cpp/c_vfflag.hxx>
	// PARAMETERS


namespace ary
{
namespace cpp
{
class Function;
struct S_VariableInfo;
}
}

namespace cpp
{

class PE_Type;
class PE_Parameter;

class PE_Function : public Cpp_PE
{
  public:
    enum E_State
	{
        afterStdOperator,           // if initializes as operator
        afterStdOperatorLeftBracket,
                                    // if initializes as operator with ( or [
        afterCastOperator,          // if initializes as operator
		afterName,	  				// undecided
		expectParameterSeparator,	//
		afterParameters,            // before const, volatile throw or = 0.
		afterThrow,			        // expect (
		expectExceptionSeparator,   //
		afterExceptions,	        // = 0 oder ; oder ,
		expectZero,	                // after '='
        inImplementation,           // after {
		size_of_states
	};
	typedef ary::cpp::E_Protection 				E_Protection;
	typedef ary::cpp::E_Virtuality 				E_Virtuality;
	typedef ary::cpp::E_ConVol 				    E_ConVol;

						PE_Function(
							Cpp_PE *			i_pParent );
						~PE_Function();

	void		        Init_Std(
                            const String  &     i_sName,
                            ary::cpp::Type_id   i_nReturnType,
	                        bool                i_bVirtual,
                            ary::cpp::FunctionFlags
                                                i_aFlags );
	void		        Init_Ctor(
                            const String  &     i_sName,
                            ary::cpp::FunctionFlags
                                                i_aFlags );
	void		        Init_Dtor(
                            const String  &     i_sName,
	                        bool                i_bVirtual,
                            ary::cpp::FunctionFlags
                                                i_aFlags );
	void		        Init_CastOperator(
	                        bool                i_bVirtual,
                            ary::cpp::FunctionFlags
                                                i_aFlags );
	void		        Init_NormalOperator(
                            ary::cpp::Type_id   i_nReturnType,
	                        bool                i_bVirtual,
                            ary::cpp::FunctionFlags
                                                i_aFlags );

	ary::cpp::Ce_id     Result_Id() const;
	bool	            Result_WithImplementation() const;

	virtual void		Call_Handler(
							const cpp::Token &	i_rTok );
  private:
	typedef SubPe< PE_Function, PE_Type >		    SP_Type;
	typedef SubPeUse< PE_Function, PE_Type >	    SPU_Type;
	typedef SubPe< PE_Function, PE_Parameter>	    SP_Parameter;
	typedef SubPeUse<PE_Function, PE_Parameter>	    SPU_Parameter;

    typedef std::vector<ary::cpp::S_Parameter>	    ParameterList;
    typedef std::vector<ary::cpp::Type_id>	        ExceptionTypeList;

	void				Setup_StatusFunctions();
	virtual void		InitData();
	virtual void		TransferData();
	void				Hdl_SyntaxError(const char * i_sText);

	void				SpInit_CastOperatorType();

	void				SpReturn_Parameter();
	void				SpReturn_Exception();
	void				SpReturn_CastOperatorType();

	void				On_afterOperator_Std_Operator(const char * i_sText);             // Operator+() etc.
	void				On_afterOperator_Std_LeftBracket(const char * i_sText); // operator [] or ()
    void                On_afterStdOperatorLeftBracket_RightBracket(const char * i_sText);
	void				On_afterOperator_Cast_Type(const char * i_sText);    // Type

	void				On_afterName_Bracket_Left(const char * i_sText);

	void				On_expectParameterSeparator_BracketRight(const char * i_sText);
	void				On_expectParameterSeparator_Comma(const char * i_sText);

	void				On_afterParameters_const(const char * i_sText);
	void				On_afterParameters_volatile(const char * i_sText);
	void				On_afterParameters_throw(const char * i_sText);
	void				On_afterParameters_SwBracket_Left(const char * i_sText);
	void				On_afterParameters_Semicolon(const char * i_sText);
	void				On_afterParameters_Comma(const char * i_sText);
	void				On_afterParameters_Colon(const char * i_sText);
	void				On_afterParameters_Assign(const char * i_sText);

	void				On_afterThrow_Bracket_Left(const char * i_sText);

    void				On_expectExceptionSeparator_BracketRight(const char * i_sText);
	void				On_expectExceptionSeparator_Comma(const char * i_sText);

	void				On_afterExceptions_SwBracket_Left(const char * i_sText);
	void				On_afterExceptions_Semicolon(const char * i_sText);
	void				On_afterExceptions_Comma(const char * i_sText);
	void				On_afterExceptions_Colon(const char * i_sText);
	void				On_afterExceptions_Assign(const char * i_sText);

	void				On_expectZero_Constant(const char * i_sText);

	void				On_inImplementation_SwBracket_Left(const char * i_sText);
	void				On_inImplementation_SwBracket_Right(const char * i_sText);
	void				On_inImplementation_Default(const char * i_sText);

    void                PerformFinishingPunctuation();
    void                EnterImplementation(
                            intt                i_nBracketCountStart ); /// 1 normally, 0 in initialisation section of c'tors.

	// DATA
	Dyn< PeStatusArray<PE_Function> >
						pStati;

	Dyn< SP_Parameter > pSpParameter;
	Dyn< SPU_Parameter> pSpuParameter;
	Dyn< SP_Type >      pSpType;
	Dyn< SPU_Type >     pSpuException;
	Dyn< SPU_Type >     pSpuCastOperatorType;       // in "operator int()" or "operator ThatClass *()"

	ary::cpp::Ce_id     nResult;
    bool                bResult_WithImplementation; // Necessary for the parent ParseEnvironment
                                                    //   to know, there is no semicolon or comma following.
        // Pre results
    StreamStr           aName;
    E_Virtuality        eVirtuality;
    E_ConVol            eConVol;
    ary::cpp::FunctionFlags
                        aFlags;
    ary::cpp::Type_id   nReturnType;
    ParameterList       aParameters;
    ExceptionTypeList   aExceptions;
    bool                bThrow;                     // Indicates, if there is a throw - important, if there are 0 exceptions listed.
    intt                nBracketCounterInImplementation;
};




// IMPLEMENTATION
inline bool
PE_Function::Result_WithImplementation() const
    { return bResult_WithImplementation; }




}   // namespace cpp
#endif





/*	// Overview of Stati

Undecided
---------

start			// vor und w?hrend storage class specifiern

->Typ

expectName		// Typ ist da

afterName




Variable
--------

start			// vor und w?hrend storage class specifiern

->Typ

expectName		// Typ ist da  -> im Falle von '(': notyetimplemented
afterName

expectSize  	// after [
expectFinish
				// vor ; oder ,
expectNextVarName  // anders als bei expectName kann hier auch * oder & kommen





Function
--------

start				// vor und w?hrend storage class specifiern

->Typ

expectName			// Typ ist da
expectBracket		// Nach Name
expectParameter		// nach ( oder ,
-> Parameter
after Parameters    // before const, volatile throw or = 0.
after throw			// expect (
expectException		// after (
after exceptions	// = 0 oder ; oder ,


expectNextVarName  // anders als bei expectName kann hier auch * oder & kommen







*/
