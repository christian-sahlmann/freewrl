// freeWRLAx.cpp : Implementation of CfreeWRLAxApp and DLL registration.

#include "stdafx.h"
#include "freeWRLAx.h"
#include "cathelp.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CfreeWRLAxApp NEAR theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x656FC6F3, 0x9CF2, 0x4674, { 0xB3, 0x32, 0xB2, 0x55, 0x8, 0x52, 0xE6, 0x44 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;

      const CATID CATID_SafeForScripting     =
      {0x7dd95801,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}};
      const CATID CATID_SafeForInitializing  =
      {0x7dd95802,0x9882,0x11cf,{0x9f,0xa9,0x00,0xaa,0x00,0x6c,0x42,0xc4}}; 


// CfreeWRLAxApp::InitInstance - DLL initialization

BOOL CfreeWRLAxApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO: Add your own module initialization code here.
	}

	return bInit;
}



// CfreeWRLAxApp::ExitInstance - DLL termination

int CfreeWRLAxApp::ExitInstance()
{
	// TODO: Add your own module termination code here.

	return COleControlModule::ExitInstance();
}



// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    BOOL        fErr = TRUE;
    char        szSubKey[513];

    // file extension for new mime type
    const char* pszMTExt0 = ".wrl";
    const char* pszMTExt1 = ".wrl";
    const char* pszMTExt2 = ".x3dv";
    const char* pszMTExt3 = ".x3d";
    // text for new mime content type
    const char* pszMTContent0 = "x-world/x-vrml";
    const char* pszMTContent1 = "model/vrml";
    const char* pszMTContent2 = "model/x3d+vrml";
    const char* pszMTContent3 = "model/x3d+xml";
    // text for mimetype subkey
    const char* pszMTSubKey0 = "MIME\\DataBase\\Content Type\\x-world/x-vrml";
    const char* pszMTSubKey1 = "MIME\\DataBase\\Content Type\\model/vrml";
    const char* pszMTSubKey2 = "MIME\\DataBase\\Content Type\\model/x3d+vrml";
    const char* pszMTSubKey3 = "MIME\\DataBase\\Content Type\\model/x3d+xml";
    // extension named value
    const char* pszMTExtVal = "Extension";
    // clsid
	const char* pszMTCLSID = "{582C9301-A2C8-45FC-831B-654DE7F3AF11}";
    const GUID CDECL BASED_CODE _ctlid =
	{ 0x582c9301, 0xa2c8, 0x45fc, {0x83, 0x1b, 0x65, 0x4d, 0xe7, 0xf3, 0xaf, 0x11}};

    // clsid named value name
    const char* pszMTCLSIDVal = "CLSID";
    // content type named value name
    const char* pszMTContentVal = "Content Type";
    // EnableFullPage key
    const char* pszMTFullPage = "EnableFullPage";

	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);


    do 
    {
	//HKCR/MIME/DataBase/Content Type/x-world/x-vrml
        // create new mime type key for our new mimetype.  Only necessary for new mime types
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTSubKey0, &hkey) )
            break;

        // add extension value to that mime type key to associate .xxx files with the 
        //  mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTExtVal, 0, REG_SZ, 
            (const BYTE *)pszMTExt0, strlen(pszMTExt0)) )
            break;

        // Add class id to associate this object with the mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTCLSIDVal, 0, REG_SZ,
            (const BYTE *)pszMTCLSID, strlen(pszMTCLSID)) )
            break;

        RegCloseKey(hkey);

	//HKCR/MIME/DataBase/Content Type/model/vrml
        // create new mime type key for our new mimetype.  Only necessary for new mime types
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTSubKey1, &hkey) )
            break;

        // add extension value to that mime type key to associate .xxx files with the 
        //  mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTExtVal, 0, REG_SZ, 
            (const BYTE *)pszMTExt1, strlen(pszMTExt1)) )
            break;

        // Add class id to associate this object with the mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTCLSIDVal, 0, REG_SZ,
            (const BYTE *)pszMTCLSID, strlen(pszMTCLSID)) )
            break;

        RegCloseKey(hkey);
	//HKCR/MIME/DataBase/Content Type/model/x3d+vrml
        // create new mime type key for our new mimetype.  Only necessary for new mime types
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTSubKey2, &hkey) )
            break;

        // add extension value to that mime type key to associate .xxx files with the 
        //  mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTExtVal, 0, REG_SZ, 
            (const BYTE *)pszMTExt2, strlen(pszMTExt2)) )
            break;

        // Add class id to associate this object with the mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTCLSIDVal, 0, REG_SZ,
            (const BYTE *)pszMTCLSID, strlen(pszMTCLSID)) )
            break;

        RegCloseKey(hkey);
	//HKCR/MIME/DataBase/Content Type/model/x3d+xml
        // create new mime type key for our new mimetype.  Only necessary for new mime types
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTSubKey3, &hkey) )
            break;

        // add extension value to that mime type key to associate .xxx files with the 
        //  mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTExtVal, 0, REG_SZ, 
            (const BYTE *)pszMTExt3, strlen(pszMTExt3)) )
            break;

        // Add class id to associate this object with the mime type
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTCLSIDVal, 0, REG_SZ,
            (const BYTE *)pszMTCLSID, strlen(pszMTCLSID)) )
            break;

        RegCloseKey(hkey);

//For object/embed IE uses the type="content type" field of the tag to get the mime type directly.
//For href links (which show FullPage), IE uses the .xxx extension to get the Content Type, 
//and uses that mime type. So for the hrefs we need to register our prefered mime type with the .xxx 
	if(TRUE)
	{
	//HKCR/.wrl
        // Register .xxx as a file extension this is only necessary for new file extensions, addimg
        // a new player for .avi files for instance would not require this
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTExt1, &hkey) )
            break;

        // Add content type to associate this extension with the content type.  This is required
        // and is used when the mime type is unknown and IE looks up associations by extension
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTContentVal, 0, REG_SZ,
            (const BYTE *)pszMTContent1, strlen(pszMTContent1)) )
            break;

        RegCloseKey(hkey);
	//HKCR/.x3d
        // Register .xxx as a file extension this is only necessary for new file extensions, addimg
        // a new player for .avi files for instance would not require this
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTExt2, &hkey) )
            break;

        // Add content type to associate this extension with the content type.  This is required
        // and is used when the mime type is unknown and IE looks up associations by extension
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTContentVal, 0, REG_SZ,
            (const BYTE *)pszMTContent2, strlen(pszMTContent2)) )
            break;

        RegCloseKey(hkey);
	//HKCR/.wrl
        // Register .xxx as a file extension this is only necessary for new file extensions, addimg
        // a new player for .avi files for instance would not require this
        if ( ERROR_SUCCESS != RegCreateKey(HKEY_CLASSES_ROOT, pszMTExt3, &hkey) )
            break;

        // Add content type to associate this extension with the content type.  This is required
        // and is used when the mime type is unknown and IE looks up associations by extension
        if ( ERROR_SUCCESS != RegSetValueEx(hkey, pszMTContentVal, 0, REG_SZ,
            (const BYTE *)pszMTContent3, strlen(pszMTContent3)) )
            break;

        RegCloseKey(hkey);
	}
//<<<<<<<<<<<<<<<<<
//I can start here for my first tests
        // Open the key under the control's clsid HKEY_CLASSES_ROOT\CLSID\<CLSID>
        wsprintf(szSubKey, "%s\\%s", pszMTCLSIDVal, pszMTCLSID);
        if ( ERROR_SUCCESS != RegOpenKey(HKEY_CLASSES_ROOT, szSubKey, &hkey) )
            break;

        // Create the EnableFullPage and extension key under this so that we can display files
        // with the extension full frame in the browser
        wsprintf(szSubKey, "%s\\%s", pszMTFullPage, pszMTExt1);
        if ( ERROR_SUCCESS != RegCreateKey(hkey, szSubKey, &hkey1) )
            break;
        RegCloseKey(hkey1);
        wsprintf(szSubKey, "%s\\%s", pszMTFullPage, pszMTExt2);
        if ( ERROR_SUCCESS != RegCreateKey(hkey, szSubKey, &hkey1) )
            break;
        RegCloseKey(hkey1);
        wsprintf(szSubKey, "%s\\%s", pszMTFullPage, pszMTExt3);
        if ( ERROR_SUCCESS != RegCreateKey(hkey, szSubKey, &hkey1) )
            break;
        RegCloseKey(hkey1);


        // Register with Internet Explorer, as per:
        // http://msdn.microsoft.com/en-us/library/bb250471.aspx  ActiveX opt-In / signing / registering 
		// "To put your control on the pre-approved list, you need to write the CLSID of the control to the following registry location.
		//  HKEY_LOCAL_MACHINE/SOFTWARE/Microsoft/Windows/CurrentVersion/Ext/PreApproved"
		if ( ERROR_SUCCESS != RegCreateKey(HKEY_LOCAL_MACHINE,
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Ext\\PreApproved\\{582C9301-A2C8-45FC-831B-654DE7F3AF11}",&hkey) )
            break;

         RegCloseKey(hkey);


		 //from http://www.codeproject.com/KB/COM/CompleteActiveX.aspx vs2005
          if (FAILED( CreateComponentCategory(
                  CATID_SafeForScripting,
                  L"Controls that are safely scriptable") ))
                return ResultFromScode(SELFREG_E_CLASS);

          if (FAILED( CreateComponentCategory(
                  CATID_SafeForInitializing,
                  L"Controls safely initializable from persistent data") ))
                return ResultFromScode(SELFREG_E_CLASS);

          if (FAILED( RegisterCLSIDInCategory(
                  _ctlid, CATID_SafeForScripting) ))
                return ResultFromScode(SELFREG_E_CLASS);

          if (FAILED( RegisterCLSIDInCategory(
                  _ctlid, CATID_SafeForInitializing) ))
                return ResultFromScode(SELFREG_E_CLASS);




        fErr = FALSE;
    } while (FALSE);

    if ( hkey )
        RegCloseKey(hkey);

    if ( hkey1 )
        RegCloseKey(hkey1);

    if ( fErr )
        MessageBox(0, "Cannot register player for mime type", "Registration Error", MB_OK);


	return NOERROR;
}



// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}
