// �o���ɮץ]�t�۰ʿ�ܳ��J��������J source code
//                                                                       by Jing
// Usage:
//				SelectAudioRecordDevice();

#include "stdafx.h"
#include "Audio.h"



void ReleaseAllDeviceData(CList<DeviceData*>& PinList){
// �]�� CList::RemoveAll �u�|�M���޲z�O����, �ҥH�n�ۤv�M�O����
	POSITION pos=PinList.GetHeadPosition();
	for(int i=0;i<(int)PinList.GetCount();i++){
		DeviceData* data=PinList.GetNext(pos);
		delete data;
	}
}

// ���o���J����J���}�Ҧb����m
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
// �C�|�t�Τ�, �Ҧ����n����J�˸m�� List �� 
HRESULT EnumFiltersWithMonikerToList(ICreateDevEnum *pSysDevEnum,const GUID *clsid, CList<DeviceData*>& List)
{
    OutputDebugString(_T("�C�|�X�ثe�t���n����J�˸m \n"));

	HRESULT hr;
    IEnumMoniker *pEnumCat = NULL;

    // Step 1: �إߨt�θ˸m�C�|���� (SystemDeviceEnum)
    if (pSysDevEnum == NULL)    {
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, 
                              CLSCTX_INPROC, IID_ICreateDevEnum, 
                              (void **)&pSysDevEnum);
        if FAILED(hr)
            return hr;
    }

    // Step 2: ����� clsid ���O�� filters �@�_��� Moniker �U�l���� (CLSID_AudioInputDeviceCategory �������˸m)
    hr = pSysDevEnum->CreateClassEnumerator(*clsid, &pEnumCat, 0);
    if (SUCCEEDED(hr)) {
    
    // Step 3: �p�G���\, �h�N�Ҧ������� Filter ������� List ��
        hr = EnumFiltersAndMonikersToList(pEnumCat, List);
        SAFE_RELEASE(pEnumCat);
    }

    pSysDevEnum->Release();
    return hr;
}

// �C�| Filters �P������ Moniker �� ListBox ��
HRESULT EnumFiltersAndMonikersToList(IEnumMoniker *pEnumCat, CList<DeviceData*>& ListFilters){
    HRESULT hr=S_OK;
    IMoniker *pMoniker=0; // ������ Filter �� �Хܤ��� 
    ULONG cFetched=0;
    VARIANT varName={0};
    int nFilters=0;

    // Step 1: �Y�S������ Filter �ŦX, �h�ϥιw�]�r��N��
    if (!pEnumCat){
        // ListFilters.AddString(TEXT("<< No entries >>\0"));
	    
        return S_FALSE;
    }

    // Step 2: �� moniker �U�l���� Filter �s, �@�Ӥ@�Ӯ��X���˵� 
	
    while(pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)    {
        IPropertyBag *pPropBag;
        ASSERT(pMoniker);

        // a. ���o �ثe moniker ���x�s���} 
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, 
                                    (void **)&pPropBag);
        ASSERT(SUCCEEDED(hr));
        ASSERT(pPropBag);
        if (FAILED(hr))
            continue;

        // b. Ū�� filter name 
        varName.vt = VT_BSTR;
        hr = pPropBag->Read(_T("FriendlyName"), &varName, 0);		
        if (FAILED(hr))
            continue;

        // c. �N BSTR �ন CString		
        CString str(varName.bstrVal);
        SysFreeString(varName.bstrVal);// ���� BSTR ���Ϊ��O����
        nFilters++;

		CString strMessage;
		strMessage.Format(_T("Audio Input Device = %s\n"),str.GetBuffer());
		OutputDebugString(strMessage);

        // d. �N��������Ʀs�� CListBox ��
        AddFilterToListWithMoniker(str, pMoniker, ListFilters);
        
        // �M���x�s�餶��
        SAFE_RELEASE(pPropBag);

        // Intentionally DO NOT release the pMoniker, since it is
        // stored in a listbox for later use
		
    }

    return hr;
}




// �N�n���˸m��� CListBox ���i��޲z
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

// �C�|�X�Ҧ������]��, ��� ListBox ��
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
	strMessage.Format(_T("�ثe�˸m�䴩����J���� pin\n"));
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

// �]�w audio ��J pin ���ݩ� (Ū���ثe���]�w��)
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

// �Ұʿ�ܪ� ��J�˸m
HRESULT ActivateSelectedInputPin(IBaseFilter *m_pInputDevice,CList<DeviceData*> &m_ListInputPins,int nActivePin) {
    HRESULT hr=S_OK;
    IPin *pPin=0;
    IAMAudioInputMixer *pPinMixer;

	// Step 1:
    // �q�ƥ��C�|�X�� �˸m Moniker ��C��, 
	// ���o�쩳�����ǭ�����J pin ���}
    int nPins = (int)m_ListInputPins.GetCount();
   
    
    for (int i=0; i<nPins; i++)    {
        // Step 2: �q�A�����ĸ˸m���o IAMAudioInputMixer ����
		//         �A�i�H�γo�Ӹ˸m�]�w ���ǭ�����J pin �n disable �� �P��
        hr = GetPin(m_pInputDevice, PINDIR_INPUT, i, &pPin);
        if (SUCCEEDED(hr)){
            hr = pPin->QueryInterface(IID_IAMAudioInputMixer, (void **)&pPinMixer);
            if (SUCCEEDED(hr)){
				// ------------- ������q -----------------
                if (i == nActivePin){
                    // �A�]�i�H�Q�� IAMAudioInputMixer �]�w�p���q���ݩ�
                    hr = SetInputPinProperties(pPinMixer);

                    // �Y�A���t�Υu���@�� input, 
					// ����]�w Enable �i��|�Ǧ^ E_NOTIMPL
                    hr = pPinMixer->put_Enable(TRUE);
                }else {
                    hr = pPinMixer->put_Enable(FALSE);
                }
                pPinMixer->Release();
            }
            // Release pin interfaces
            pPin->Release();
        }
    }// end of �Ҧ����� pin �B�z

    return hr;
}

HRESULT SelectAudioRecordDevice(){
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	// �C�|�t�Τ��Ҧ��������w��˸m
	CList<DeviceData*> m_ListAudioInputDevices;
    HRESULT hr = EnumFiltersWithMonikerToList(NULL, &CLSID_AudioInputDeviceCategory, m_ListAudioInputDevices);
    if (FAILED(hr))
        return hr;

	// ���o�����˸m�� moniker
	DeviceData* selDevice=(DeviceData*)m_ListAudioInputDevices.GetHead();
	IMoniker *pMoniker = selDevice->m_pMoniker;
    if (!pMoniker)
       return -1;

	// �Q�� moniker �إ� audio capture device ����
	IBaseFilter *m_pInputDevice;
    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&m_pInputDevice);   
    if (FAILED(hr))
        return hr;

	// �C�|�����˸m����J���} {CD ��J, Micophone, ... etc}
	CList<DeviceData*>	m_ListInputPins;
	hr = EnumPinsOnFilter(m_pInputDevice, PINDIR_INPUT, m_ListInputPins);
    if (FAILED(hr))
        return hr;

	// ��� microphone �Ҧb����m
	int microphoneIndex=GetMicPinIndex(m_ListInputPins);
	if(microphoneIndex!=-1)
		ActivateSelectedInputPin(m_pInputDevice,m_ListInputPins,microphoneIndex);
	else
		OutputDebugString(_T("There is no microphone input source in your computer"));
	
	// �M���Ҧ��ϥΪ��O����
	ReleaseAllDeviceData(m_ListInputPins);
	ReleaseAllDeviceData(m_ListAudioInputDevices);
	m_ListInputPins.RemoveAll();
	m_ListAudioInputDevices.RemoveAll();
	CoUninitialize();// Closes the COM library on the current thread
	return S_OK;
}