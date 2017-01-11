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
int GetMicPinIndex(CList<DeviceData*>& PinList); // ���o���J����J���}�Ҧb����m
HRESULT EnumFiltersAndMonikersToList(IEnumMoniker *pEnumCat, CList<DeviceData*>& List); // �C�|�t�Τ�, �Ҧ����n����J�˸m�� ListBox �� 
void AddFilterToListWithMoniker(const TCHAR *szFilterName,IMoniker *pMoniker, CList<DeviceData*>& List); 
HRESULT SetInputPinProperties(IAMAudioInputMixer *pPinMixer);// �]�w audio ��J pin ���ݩ� (Ū���ثe���]�w��)
HRESULT EnumPinsOnFilter (IBaseFilter *pFilter, PIN_DIRECTION PinDir,CList<DeviceData*> &List);// �C�|�X�Ҧ������]��, ��� ListBox ��
HRESULT ActivateSelectedInputPin(IBaseFilter *m_pInputDevice,CList<DeviceData*> &m_ListInputPins,int nActivePin);// �Ұʿ�ܪ� ��J�˸m

#endif