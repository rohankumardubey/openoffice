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




#ifndef TX3_WTRATREE_HXX
#define TX3_WTRATREE_HXX

// USED
	// Base Classes
	// Components
	// Parameters
#include <tools/string.hxx>

const INT16		C_NR_OF_WTT_RESULTS = 5;
const INT16    	C_NR_OF_POSSIBLE_CHARS = 256;


typedef unsigned char u_char;
typedef const char * constr;


class WTT_Node;


/**	@task
	This class implements the functionality, that class WordTransformer
	offers.
	WordTransformer is dependant of this class, but NOT the other way!
**/
class WordTransTree
{
  public:
	enum E_Result
	{
		OK = 0,
		HOTKEY_LOST,
		OUTPUT_OVERFLOW
	};


	//	LIFECYCLE
						WordTransTree(
							CharSet				i_nWorkingCharSet = RTL_TEXTENCODING_MS_1252);
	void				SetCharSet(
							CharSet				i_nWorkingCharSet);
						~WordTransTree();

	void				AddWordPair(
							const ByteString &		i_sOldString,
							const ByteString &		i_sReplaceString );

	// OPERATIONS
	void				InitTransformation(
							const char *		i_sInput,               /// [!=0], a range of i_nInputLength must be valid memory for read.
							UINT32				i_nInputLength,
							UINT32				i_nOutputMaxLength = STRING_MAXLEN - 12 );
	E_Result			TransformNextToken();

	// INQUIRY
	sal_Bool				TextEndReached() const;
	const char *		Output() const;

		// These 3 functions are valid between two calls of
		//   TransformNextToken():
	E_Result			CurResult() const;
	ByteString 			CurReplacedString() const;
	ByteString	 		CurReplacingString() const;
	char 				CurHotkey() const;

  private:
	// SERVICE FUNCTONS
	UINT8	  			CalculateBranch(
							u_char 				i_cInputChar ) const;

	void				Handle_Hotkey();
	void				Handle_TokenToKeep();
	void				Handle_TokenToTransform();

	// DATA
		// Fixed data
	const u_char *		sInput;
	UINT32              nInputLength;
	const u_char *		pInputEnd;

	u_char *			sOutput;				// DYN
	UINT32				nOutputMaxLength;

	WTT_Node *			dpParsingTreeTop;		// DYN
	WTT_Node *			pUnknownAlpha;
	u_char		 		cChar2Branch[C_NR_OF_POSSIBLE_CHARS];
	u_char		 		c_AE, c_OE, c_UE, c_ae, c_oe, c_ue;

		// Working data
	const u_char *		pInputCurTokenStart;
	const u_char *		pInputPosition;
	u_char *			pOutputPosition;
	WTT_Node *			pCurParseNode;

		// Data which are valid only after a completed call to TransformNextToken()
	E_Result			eCurResult;
	u_char				cCurHotkey;            	// Letter wich is used as hotkey
	u_char				cCurHotkeySign;       	// Letter which is used to assign hotkey ('~'or '&') .
};







inline sal_Bool
WordTransTree::TextEndReached() const
	{ return pInputPosition == pInputEnd; }
inline const char *
WordTransTree::Output() const
	{ return TextEndReached() ? (constr) sOutput : ""; }
inline WordTransTree::E_Result
WordTransTree::CurResult() const
	{ return eCurResult; }
inline ByteString
WordTransTree::CurReplacedString() const
	{ return ByteString((constr) pInputCurTokenStart,pInputPosition-pInputCurTokenStart); }
inline char
WordTransTree::CurHotkey() const
	{ return cCurHotkey; }
inline UINT8
WordTransTree::CalculateBranch(u_char i_cInputChar) const
	{ return cChar2Branch[i_cInputChar]; }



#endif



