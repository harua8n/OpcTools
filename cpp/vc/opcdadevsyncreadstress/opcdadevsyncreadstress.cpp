#include "stdafx.h"
#include <iostream>
#include <conio.h>

#import "C:\\Windows\\SysWOW64\\OPCDAAuto.dll"

using namespace std;

int main()
{
	CoInitializeEx(NULL, COINITBASE_MULTITHREADED);

	OPCAutomation::IOPCAutoServer *pOPC = NULL;
	HRESULT hr = CoCreateInstance(__uuidof(OPCAutomation::OPCServer),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(OPCAutomation::IOPCAutoServer),
		(void**)&pOPC
		);

	if (SUCCEEDED(hr)) {
		hr = pOPC->Connect("Matrikon.OPC.Simulation.1");
		if (SUCCEEDED(hr)) {
			cout << "Vendor : " << pOPC->VendorInfo << endl;
			cout << "Server : " << pOPC->ServerName << endl;
			cout << "Major version: " << pOPC->MajorVersion << endl;
			cout << "Minor version: " << pOPC->MinorVersion << endl;
			cout << "Build : " << pOPC->BuildNumber << endl;
			DATE curdate;
			pOPC->get_CurrentTime(&curdate);
			SYSTEMTIME st;
			VariantTimeToSystemTime(curdate, &st); 
			cout << "Date : " << st.wYear << "/" << st.wMonth << "/" << st.wDay << " " << st.wHour << ":" << st.wMinute << ":" << st.wSecond << endl;
		
			OPCAutomation::IOPCGroupsPtr pGrps = pOPC->OPCGroups;
			if (pGrps) {
				OPCAutomation::IOPCGroupPtr pGrp = pGrps->Add();
				OPCAutomation::OPCItemsPtr pOpcItems = pGrp->OPCItems;

				while (true)
				{
					cout << "Items : " << pOpcItems->Count << endl;
					OPCAutomation::OPCItemPtr pOpcItem = pOpcItems->AddItem("Random.Real4", 0);

					VARIANT v, q, t;
					VariantInit(&v);
					VariantInit(&q);
					VariantInit(&t);
					pOpcItem->Read(OPCAutomation::OPCDataSource::OPCDevice, &v, &q, &t);
					cout << "Value : " << v.fltVal << endl;

					long lNumItems = 1L;
					SAFEARRAYBOUND bound;
					bound.lLbound = 1;
					bound.cElements = lNumItems;
					SAFEARRAY * pSH = SafeArrayCreate(VT_I4, 1, &bound);
					long handle = pOpcItem->ServerHandle;
					SafeArrayPutElement(pSH, &lNumItems, &handle);

					SAFEARRAY * pErrors = 0;
					hr = pOpcItems->Remove(lNumItems, &pSH, &pErrors);
					if (!SUCCEEDED(hr)) {
						cout << "Error removing item" << endl;
						break;
					}
					SafeArrayDestroy(pSH);
					SafeArrayDestroy(pErrors);
					//Sleep(10000);
					if (_kbhit()) break;
				}
			}
		}
	}
	
	if (NULL != pOPC) {
		pOPC->Release();
		pOPC = NULL;
	}

	CoUninitialize();
    return 0;
}

