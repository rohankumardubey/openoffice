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

// UserRegistrar.cpp: Implementierung der Klasse UserRegistrar.
//
//////////////////////////////////////////////////////////////////////

#include "userregistrar.hxx"
#include "registryvalueimpl.hxx"
#include "windowsregistry.hxx"
#include "registryexception.hxx"

#ifdef _MSC_VER
#pragma warning(disable : 4350)
#endif

//--------------------------------------
/**
*/
UserRegistrar::UserRegistrar(const RegistrationContextInformation& RegContext) :
	Registrar(RegContext)
{
	RegistryKey RegKey = WindowsRegistry().GetCurrentUserKey();
	m_RootKey = RegKey->OpenSubKey(L"Software\\Classes");
}

//###################################
// Command 
//###################################

//--------------------------------------
/**
*/
void UserRegistrar::UnregisterAsHtmlEditorForInternetExplorer() const
{
    Registrar::UnregisterAsHtmlEditorForInternetExplorer();
    
    DeleteHtmFileAssociationKeys();
    
	try
	{
		RegistryKey RegKey = m_RootKey->OpenSubKey(L"Applications");
		if ((0 == RegKey->GetSubValueCount()) && (0 == RegKey->GetSubKeyCount()))
		{
			RegKey->Close();
			m_RootKey->DeleteSubKey(L"Applications");
		}
	}
	catch(RegistryKeyNotFoundException&)
	{
	}
}

//--------------------------------------
/**
*/
void UserRegistrar::RegisterAsDefaultShellHtmlEditor() const
{
    RegistryKey LocalHtmKey = m_RootKey->CreateSubKey(L".htm");
    
    if (!LocalHtmKey->HasValue(DEFAULT_VALUE_NAME))
    {
        RegistryKey HKCRKey = WindowsRegistry().GetClassesRootKey();
        
        if (HKCRKey->HasSubKey(L".htm"))
        {
            RegistryKey RootHtmKey = HKCRKey->OpenSubKey(L".htm", false);
            
            if (RootHtmKey->HasValue(DEFAULT_VALUE_NAME))
            {
                RegistryValue RegVal = RootHtmKey->GetValue(DEFAULT_VALUE_NAME);
            
                std::wstring RootHtmFwdKey = RegVal->GetDataAsUniString();
                
                if (HKCRKey->HasSubKey(RootHtmFwdKey))
                {            
                    m_RootKey->CreateSubKey(RootHtmFwdKey);
                    LocalHtmKey->CopyValue(RootHtmKey, DEFAULT_VALUE_NAME);
                }
            }
        }
    }
    
    // calling base class method
    Registrar::RegisterAsDefaultShellHtmlEditor();
}

//--------------------------------------
/**
*/
void UserRegistrar::UnregisterAsDefaultShellHtmlEditor() const
{
    // calling base class method
    Registrar::UnregisterAsDefaultShellHtmlEditor();
    DeleteHtmFileAssociationKeys();
}
    
//--------------------------------------
/**
*/
void UserRegistrar::UnregisterForMsOfficeApplication(
        const std::wstring& FileExtension) const
{
    /// calling base class method
    Registrar::UnregisterForMsOfficeApplication(FileExtension);
        
    if (m_RootKey->HasSubKey(FileExtension))
    {
        RegistryKey RegKey = m_RootKey->OpenSubKey(FileExtension);
        
        if ((0 == RegKey->GetSubKeyCount()) && (0 == RegKey->GetSubValueCount()))
        {
            RegKey->Close();
            m_RootKey->DeleteSubKey(FileExtension);
        }
    }
}

//--------------------------------------
/**
*/
RegistryKey UserRegistrar::GetRootKeyForDefHtmlEditorForIERegistration() const
{
    return WindowsRegistry().GetCurrentUserKey();
}

//--------------------------------------
/** 
*/
void UserRegistrar::DeleteHtmFileAssociationKeys() const
{
    // Later delete the created keys if they are empty and have not changed meanwhile.
    // Remeber: if we create a new registry key in the user part of the
    // registry, changes to that key via the merged key HKEY_CLASSES_ROOT
    // go into the user branch HKEY_CURRENT_USER and are not visible for other users. 
    // so we must carefully detect if the keys have not changed in order to prevent accidentally
    // deleting a key and so destroying existing associations
    // See MSDN: "Merged View of HKEY_CLASSES_ROOT"
}
