#ifndef _DSAUDIO
#define _DSAUDIO
#include <Dshow.h>
#include "Utility.h"
class DeviceData{
public:
	CString m_FilterName;
	IMoniker *m_pMoniker;

	DeviceData(CString Name,IMoniker *pMoniker){
		m_FilterName=Name;
		m_pMoniker=pMoniker;
	}
	~DeviceData(){
		if(m_pMoniker!=NULL)
			m_pMoniker->Release();
	}
};

void ReleaseAllDeviceData(CList<DeviceData*>& PinList);
HRESULT SelectAudioRecordDevice();
int GetMicPinIndex(CList<DeviceData*>& PinList); // 取得麥克風輸入接腳所在的位置
HRESULT EnumFiltersAndMonikersToList(IEnumMoniker *pEnumCat, CList<DeviceData*>& List); // 列舉系統中, 所有的聲音輸入裝置到 ListBox 中 
void AddFilterToListWithMoniker(const TCHAR *szFilterName,IMoniker *pMoniker, CList<DeviceData*>& List); 
HRESULT SetInputPinProperties(IAMAudioInputMixer *pPinMixer);// 設定 audio 輸入 pin 的屬性 (讀取目前的設定值)
HRESULT EnumPinsOnFilter (IBaseFilter *pFilter, PIN_DIRECTION PinDir,CList<DeviceData*> &List);// 列舉出所有錄音設備, 放到 ListBox 中
HRESULT ActivateSelectedInputPin(IBaseFilter *m_pInputDevice,CList<DeviceData*> &m_ListInputPins,int nActivePin);// 啟動選擇的 輸入裝置

#endif