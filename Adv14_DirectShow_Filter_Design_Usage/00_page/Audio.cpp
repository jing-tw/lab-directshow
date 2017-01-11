// 這個檔案包含自動選擇麥克風音源輸入 source code
//                                                                       by Jing
// Usage:
//				SelectAudioRecordDevice();

#include "stdafx.h"
#include "Audio.h"



void ReleaseAllDeviceData(CList<DeviceData*>& PinList){
// 因為 CList::RemoveAll 只會清除管理記憶體, 所以要自己清記憶體
	POSITION pos=PinList.GetHeadPosition();
	for(int i=0;i<(int)PinList.GetCount();i++){
		DeviceData* data=PinList.GetNext(pos);
		delete data;
	}
}

// 取得麥克風輸入接腳所在的位置
// return: The zero-based index of the Microphone in this List; -1 if there is no microphone
int GetMicPinIndex(CList<DeviceData*>& PinList){

	POSITION pos=PinList.GetHeadPosition();

	for(int i=0;i<(int)PinList.GetCount();i++){
		DeviceData* data=PinList.GetNext(pos);

		//CString strMessage;
		//strMessage.Format(_T("Audio Input source = %s\n"),data->m_FilterName);
		//OutputDebugString(strMessage); 
		
		int k=data->m_FilterName.Find(_T("Microphone"));
		if(data->m_FilterName.Find(_T("Microphone"))!=-1)
			return i;
	}
	
	return -1;
}

// HRESULT CAudioCapDlg::FillLists() 
// 列舉系統中, 所有的聲音輸入裝置到 List 中 
HRESULT EnumFiltersWithMonikerToList(ICreateDevEnum *pSysDevEnum,const GUID *clsid, CList<DeviceData*>& List)
{
    OutputDebugString(_T("列舉出目前系統聲音輸入裝置 \n"));

	HRESULT hr;
    IEnumMoniker *pEnumCat = NULL;

    // Step 1: 建立系統裝置列舉物件 (SystemDeviceEnum)
    if (pSysDevEnum == NULL)    {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
                              CLSCTX_INPROC, IID_ICreateDevEnum, 
                              (void **)&pSysDevEnum);
        if FAILED(hr)
            return hr;
    }

    // Step 2: 把相關 clsid 類別的 filters 一起放到 Moniker 袋子介面 (CLSID_AudioInputDeviceCategory 為錄音裝置)
    hr = pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);
    if (SUCCEEDED(hr)) {
    
    // Step 3: 如果成功, 則將所有相關的 Filter 全部放到 List 中
        hr = EnumFiltersAndMonikersToList(pEnumCat, List);
        SAFE_RELEASE(pEnumCat);
    }

    pSysDevEnum->Release();
    return hr;
}

// 列舉 Filters 與對應的 Moniker 到 ListBox 中
HRESULT EnumFiltersAndMonikersToList(IEnumMoniker *pEnumCat, CList<DeviceData*>& ListFilters){
    HRESULT hr=S_OK;
    IMoniker *pMoniker=0; // 對應該 Filter 的 標示介面 
    ULONG cFetched=0;
    VARIANT varName={0};
    int nFilters=0;

    // Step 1: 若沒有任何 Filter 符合, 則使用預設字串代替
    if (!pEnumCat){
        // ListFilters.AddString(TEXT("<< No entries >>\0"));
	    
        return S_FALSE;
    }

    // Step 2: 把 moniker 袋子中的 Filter 群, 一個一個拿出來檢視 
	
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)    {
        IPropertyBag *pPropBag;
        ASSERT(pMoniker);

        // a. 取得 目前 moniker 的儲存體位址 
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                                    (void **)&pPropBag);
        ASSERT(SUCCEEDED(hr));
        ASSERT(pPropBag);
        if (FAILED(hr))
            continue;

        // b. 讀取 filter name 
        varName.vt = VT_BSTR;
        hr = pPropBag->Read(_T("FriendlyName"), &varName, 0);		
        if (FAILED(hr))
            continue;

        // c. 將 BSTR 轉成 CString		
        CString str(varName.bstrVal);
        SysFreeString(varName.bstrVal);// 釋放 BSTR 佔用的記憶體
        nFilters++;

		CString strMessage;
		strMessage.Format(_T("Audio Input Device = %s\n"),str.GetBuffer());
		OutputDebugString(strMessage);

        // d. 將相關的資料存到 CListBox 中
        AddFilterToListWithMoniker(str, pMoniker, ListFilters);
        
        // 清除儲存體介面
        SAFE_RELEASE(pPropBag);

        // Intentionally DO NOT release the pMoniker, since it is
        // stored in a listbox for later use
		
    }

    return hr;
}




// 將聲音裝置放到 CListBox 中進行管理
void AddFilterToListWithMoniker(const TCHAR *szFilterName,IMoniker *pMoniker, CList<DeviceData*> &ListFilters){
    if (!szFilterName)
        return;

	ListFilters.AddTail(new DeviceData(szFilterName,pMoniker));
    // Add the category name and a pointer to its CLSID to the list box
    //int nSuccess  = ListFilters.AddString(szFilterName);
    //int nIndexNew = ListFilters.FindStringExact(-1, szFilterName);

    // int nSuccess = ListFilters.SetItemDataPtr(nIndexNew, pMoniker);
	// ListFilters.SetItemDataPtr(i, pMoniker);
}

// 列舉出所有錄音設備, 放到 ListBox 中
HRESULT EnumPinsOnFilter (IBaseFilter *pFilter, PIN_DIRECTION PinDir,
                          CList<DeviceData*> &List)
{
    HRESULT hr;
    IEnumPins  *pEnum = NULL;
    IPin *pPin = NULL;

    // Clear the specified listbox (input or output)
    // Listbox.ResetContent();
	ReleaseAllDeviceData(List);
	List.RemoveAll();

    // Verify filter interface
    if (!pFilter)
        return E_NOINTERFACE;

    // Get pin enumerator
    hr = pFilter->EnumPins(&pEnum);
    if (FAILED(hr)){
        // Listbox.AddString(TEXT("<ERROR>\0"));
		List.AddTail(new DeviceData(_T("<Error>"),NULL));
        return hr;
    }

    pEnum->Reset();

    // Enumerate all pins on this filter
	CString strMessage;
	strMessage.Format(_T("目前裝置支援的輸入音源 pin\n"));
	OutputDebugString(strMessage);

    while((hr = pEnum->Next(1, &pPin, 0)) == S_OK){
        PIN_DIRECTION PinDirThis;

        hr = pPin->QueryDirection(&PinDirThis);
        if (FAILED(hr)){
            List.AddTail(new DeviceData(_T("<Error>"),NULL));// Listbox.AddString(_T("<ERROR>\0"));
            pPin->Release();
            continue;
        }

        // Does the pin's direction match the requested direction?
        if (PinDir == PinDirThis){
            PIN_INFO pininfo={0};

            // Direction matches, so add pin name to listbox
            hr = pPin->QueryPinInfo(&pininfo);
            if (SUCCEEDED(hr)){
                CString str(pininfo.achName);
                // Listbox.AddString(str);
				List.AddTail(new DeviceData(str,NULL));
				strMessage.Format(_T("%s\n"),str.GetBuffer());
				OutputDebugString(strMessage);
            }

            // The pininfo structure contains a reference to an IBaseFilter,
            // so you must release its reference to prevent resource a leak.
            pininfo.pFilter->Release();
        }
        pPin->Release();
    }
    pEnum->Release();

    return hr;
}

// 設定 audio 輸入 pin 的屬性 (讀取目前的設定值)
HRESULT SetInputPinProperties(IAMAudioInputMixer *pPinMixer) {
    HRESULT hr=0;
    BOOL bLoudness=0, bMono=0;
    double dblBass=0, dblBassRange=0, dblMixLevel=0, 
           dblPan=0, dblTreble=0, dblTrebleRange=0;

    // Read current settings for debugging purposes.  Many of these interfaces
    // will not be implemented by the input device's driver, so they will
    // return E_NOTIMPL.  In that case, just ignore the values.
    hr = pPinMixer->get_Bass(&dblBass);
    hr = pPinMixer->get_BassRange(&dblBassRange);
    hr = pPinMixer->get_Loudness(&bLoudness);
    hr = pPinMixer->get_MixLevel(&dblMixLevel);
    hr = pPinMixer->get_Mono(&bMono);
    hr = pPinMixer->get_Pan(&dblPan);
    hr = pPinMixer->get_Treble(&dblTreble);
    hr = pPinMixer->get_TrebleRange(&dblTrebleRange);

    // Manipulate desired values here (mono/stereo, pan, etc.)

    // Since some of these methods may fail, just return success.
    // This function is for demonstration purposes only.
    return S_OK;
}

// 啟動選擇的 輸入裝置
HRESULT ActivateSelectedInputPin(IBaseFilter *m_pInputDevice,CList<DeviceData*> &m_ListInputPins,int nActivePin) {
    HRESULT hr=S_OK;
    IPin *pPin=0;
    IAMAudioInputMixer *pPinMixer;

	// Step 1:
    // 從事先列舉出的 裝置 Moniker 串列中, 
	// 取得到底有哪些音源輸入 pin 接腳
    int nPins = (int)m_ListInputPins.GetCount();
   
    
    for (int i=0; i<nPins; i++)    {
        // Step 2: 從你的音效裝置取得 IAMAudioInputMixer 介面
		//         你可以用這個裝置設定 哪些音源輸入 pin 要 disable 或 致能
        hr = GetPin(m_pInputDevice, PINDIR_INPUT, i, &pPin);
        if (SUCCEEDED(hr)){
            hr = pPin->QueryInterface(IID_IAMAudioInputMixer, (void **)&pPinMixer);
            if (SUCCEEDED(hr)){
				// ------------- 關鍵片段 -----------------
                if (i == nActivePin){
                    // 你也可以利用 IAMAudioInputMixer 設定如音量等屬性
                    hr = SetInputPinProperties(pPinMixer);

                    // 若你的系統只有一個 input, 
					// 那麼設定 Enable 可能會傳回 E_NOTIMPL
                    hr = pPinMixer->put_Enable(TRUE);
                }else {
                    hr = pPinMixer->put_Enable(FALSE);
                }
                pPinMixer->Release();
            }
            // Release pin interfaces
            pPin->Release();
        }
    }// end of 所有音源 pin 處理

    return hr;
}

HRESULT SelectAudioRecordDevice(){
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// 列舉系統中所有的錄音硬體裝置
	CList<DeviceData*> m_ListAudioInputDevices;
    HRESULT hr = EnumFiltersWithMonikerToList(NULL, &CLSID_AudioInputDeviceCategory, m_ListAudioInputDevices);
    if (FAILED(hr))
        return hr;

	// 取得錄音裝置的 moniker
	DeviceData* selDevice=(DeviceData*)m_ListAudioInputDevices.GetHead();
	IMoniker *pMoniker = selDevice->m_pMoniker;
    if (!pMoniker)
       return -1;

	// 利用 moniker 建立 audio capture device 物件
	IBaseFilter *m_pInputDevice;
    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pInputDevice);   
    if (FAILED(hr))
        return hr;

	// 列舉錄音裝置的輸入接腳 {CD 輸入, Micophone, ... etc}
	CList<DeviceData*>	m_ListInputPins;
	hr = EnumPinsOnFilter(m_pInputDevice, PINDIR_INPUT, m_ListInputPins);
    if (FAILED(hr))
        return hr;

	// 選擇 microphone 所在的位置
	int microphoneIndex=GetMicPinIndex(m_ListInputPins);
	if(microphoneIndex!=-1)
		ActivateSelectedInputPin(m_pInputDevice,m_ListInputPins,microphoneIndex);
	else
		OutputDebugString(_T("There is no microphone input source in your computer"));
	
	// 清除所有使用的記憶體
	ReleaseAllDeviceData(m_ListInputPins);
	ReleaseAllDeviceData(m_ListAudioInputDevices);
	m_ListInputPins.RemoveAll();
	m_ListAudioInputDevices.RemoveAll();
	CoUninitialize();// Closes the COM library on the current thread
	return S_OK;
}