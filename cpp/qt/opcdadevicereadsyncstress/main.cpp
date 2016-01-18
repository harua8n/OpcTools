#include <QAxObject>
#include <QDebug>
#include <QList>
#include <QVariant>
#include <QUuid>
#include "OPCDaAuto.h"
#include "windows.h"
#include <conio.h>

int main(int /* argc */, char ** /* argv[] */)
{
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

    CLSID classId;
    if(SUCCEEDED(CLSIDFromProgID(OLESTR("OPC.Automation"), &classId)))
    {
        IOPCAutoServer *pOpc = NULL;
        HRESULT hr = CoCreateInstance(classId,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IOPCAutoServer,
                                      (void**)&pOpc
                                      );

        if(!SUCCEEDED(hr)) {
            qDebug() << "Failed to get interface to OPCAutoServer";
            return 1;
        }

        VARIANT node;
        VariantInit(&node);
        node.vt = VT_BSTR;
        node.bstrVal = SysAllocString(L"localhost");
        BSTR server = SysAllocString(L"Matrikon.OPC.Simulation.1");
        hr = pOpc->Connect(server,node);
        VariantClear(&node);
        SysFreeString(server);

        if(!SUCCEEDED(hr)) {
            qDebug() << "Failed to connect to OPC Server";
            return 1;
        }

        //http://stackoverflow.com/questions/6284524/bstr-to-stdstring-stdwstring-and-vice-versa
        //http://www.qtcentre.org/threads/29633-BSTR-to-QString-Conversion
        BSTR bstrResult = NULL;
        pOpc->get_VendorInfo(&bstrResult);
        QString qs((QChar*)bstrResult, SysStringLen(bstrResult));
        qDebug() << "Vendor : " << qs;
        ::SysFreeString(bstrResult);

        pOpc->get_ServerName(&bstrResult);
        qs = QString((QChar*)bstrResult, SysStringLen(bstrResult));
        qDebug() << "Vendor : " << qs;
        ::SysFreeString(bstrResult);

        short sResult;
        pOpc->get_MajorVersion(&sResult);
        qDebug() << "Major : " << sResult;
        pOpc->get_MinorVersion(&sResult);
        qDebug() << "Minor : " << sResult;
        pOpc->get_BuildNumber(&sResult);
        qDebug() << "Build : " << sResult;

        IOPCGroups *grps = 0;
        hr = pOpc->get_OPCGroups((OPCGroups**)&grps);
        if(!SUCCEEDED(hr)) {
            qDebug() << "Failed to get OPC groups";
            return 1;
        }
        qDebug() << "grps ptr : " << grps;

        VARIANT bstr_group_name;
        VariantInit(&bstr_group_name);
        bstr_group_name.bstrVal = SysAllocString(L"G1");
        bstr_group_name.vt = VT_BSTR;
        IOPCGroup *grp;
        hr = grps->Add(bstr_group_name,(OPCGroup**)&grp);
        VariantClear(&bstr_group_name);
        if(!SUCCEEDED(hr)) {
            qDebug() << "Failed to add OPC group";
            return 1;
        }
        qDebug() << "grp ptr : " << grp;

        OPCItems *items;
        hr = grp->get_OPCItems((OPCItems**)&items);
        if(!SUCCEEDED(hr)) {
            qDebug() << "Failed to get OPC items";
            return 1;
        }
        qDebug() << "items ptr : " << items;

        qDebug() << "Any key to exit";
        while(TRUE)
        {
            VARIANT vr;
            VARIANT q;
            VARIANT t;
            long numItems = 1L;
            SAFEARRAYBOUND bound;
            bound.lLbound = 1;
            bound.cElements = numItems;
            long sh;
            long *psh;

            OPCItem *item;
            BSTR itemId = ::SysAllocString(L"Random.Real4");

            hr = items->AddItem(itemId, 0L, (OPCItem**)&item);
            if (hr != S_OK) {
                qDebug() << "Adding item failed";
                break;
            }
            ::SysFreeString(itemId);


            VariantInit(&vr);
            VariantInit(&q);
            VariantInit(&t);
            hr = item->Read(OPCDevice, &vr, &q, &t);
            if (hr != S_OK) {
                qDebug() << "Reading item failed";
                break;
            }

            if(vr.vt == VT_R4) {
                qDebug() << vr.fltVal;
            } else {
                qDebug() << "Unknown data type : " << vr.vt;
                break;
            }
            VariantClear(&vr);
            VariantClear(&q);
            VariantClear(&t);

            SAFEARRAY *pSH = SafeArrayCreate(VT_I4,1, &bound);
            item->get_ServerHandle(&sh);
            psh = &sh;
            SafeArrayPutElement(pSH, &numItems, psh);

            SAFEARRAY *pErrors = 0;
            hr = items->Remove(numItems, (SAFEARRAY**)&pSH, &pErrors);
            if (hr != S_OK) {
                qDebug() << "Removing item failed";
                break;
            }
            SafeArrayDestroy(pSH);
            SafeArrayDestroy(pErrors);
            item->Release();
            //        Sleep(1000);
            if(kbhit()) break;
        }



        items->Release();
        grp->Release();
        grps->RemoveAll();
        grps->Release();
        pOpc->Disconnect();
        pOpc->Release();
    }
    ::CoUninitialize();
}

