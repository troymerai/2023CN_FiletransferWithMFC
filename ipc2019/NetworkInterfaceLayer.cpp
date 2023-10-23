// NetworkInterfaceLayer.cpp: implementation of the NetworkInterfaceLayer class.

#include "stdafx.h"
#include "pch.h"
#include "NetworkInterfaceLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNILayer::CNILayer(char *pName) 
	: CBaseLayer(pName), device(NULL) {
	if (pcap_findalldevs(&allDevices, errbuf) == -1)
	{
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		allDevices = NULL;
	}

	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	OidData->Oid = 0x01010101;
	OidData->Length = 6;
}

CNILayer::~CNILayer() {
	pcap_if_t* temp = allDevices;

	while (temp) {
		temp = allDevices;
		allDevices = allDevices->next;
		delete(temp);
		delete(OidData);
	}
}

BOOL CNILayer::Receive(unsigned char* pkt) {
	//endian ��ȯ �ʿ�?
	if (pkt == nullptr) {
		return FALSE;
	}
	if (!(mp_aUpperLayer[0]->Receive(pkt))) { // �Ѱ����� ���ߴٸ� FALSE
		return FALSE;
	}
	
	return TRUE;
}

BOOL CNILayer::Send(unsigned char* ppayload, int nlength) {
	if (pcap_sendpacket(m_AdapterObject, ppayload, nlength))
	{
		AfxMessageBox(_T("��Ŷ ���� ����"));
		return FALSE;
	}
	return TRUE;
}

UCHAR* CNILayer::SetAdapter(const int index) { //MAC �ּҸ� ����!
	device = allDevices;
	for (int i = 0; i < index && device; i++) {
		device = device->next;
	}

	m_AdapterObject = pcap_open_live(device->name, 65536, 0, 1000, errbuf);
	if (m_AdapterObject == nullptr)
	{
		AfxMessageBox(_T("��� ���� ����!"));
	}

	adapter = PacketOpenAdapter(device->name);
	PacketRequest(adapter, FALSE, OidData);

	memcpy(macAddress, (OidData->Data), 6);
	PacketCloseAdapter(adapter);
	//AfxBeginThread(ThreadFunction_RECEIVE, this);

	return macAddress;
}

void CNILayer::GetMacAddressList(CStringArray &adapterlist) {
	for (pcap_if_t* d = allDevices; d; d = d->next) {
		adapterlist.Add(d->description);
	}
}

UINT CNILayer::ThreadFunction_RECEIVE(LPVOID pParam) {
	struct pcap_pkthdr* header;
	const u_char* pkt_data;
	CNILayer* pNI = (CNILayer*)pParam;

	int result=1;
	
	while (pNI->canRead)
	{
		result = pcap_next_ex(pNI->m_AdapterObject, &header, &pkt_data);
		if (result == 0) {
			//AfxMessageBox("��Ŷ ���� ����");
		}
		else if (result == 1) {
			memcpy(pNI->data, pkt_data, ETHER_MAX_SIZE);
			//AfxMessageBox("��Ŷ �޾��� ����");
			pNI->Receive(pNI->data);
		}
	}
	return 0;
}

void CNILayer::flip() {
	canRead = !canRead;
}