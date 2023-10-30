// NetworkInterfaceLayer.cpp: implementation of the NetworkInterfaceLayer class.

#include "stdafx.h"
#include "pch.h"
#include "NetworkInterfaceLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// ������ >> �̸��� �θ� Ŭ������ ���� && pcap_findalldevs�� ����� ���� ����
CNILayer::CNILayer(char* pName)
	: CBaseLayer(pName), device(NULL) {
	// pcap_findalldevs �Լ� ȣ�� ���� ��
	if (pcap_findalldevs(&allDevices, errbuf) == -1)
	{
		// ���� �޽��� ���
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		allDevices = NULL;
	}

	// OID ������ ����ü �޸� �Ҵ�
	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	// OID ������ ����ü�� Oid ��� �ʱ�ȭ
	OidData->Oid = 0x01010101;
	// OID ������ ����ü�� length ��� �ʱ�ȭ
	OidData->Length = 6;
}

// �Ҹ��� >> �������� �Ҵ��� �޸𸮸� ���� && ��� ��ġ ���� ����
CNILayer::~CNILayer() {
	pcap_if_t* temp = allDevices;

	while (temp) {
		temp = allDevices;
		allDevices = allDevices->next;
		delete(temp);
		delete(OidData);
	}
}

// ��Ŷ�� �޾� ���� ����(���⼭�� Ethernet Layer)�� �����ϴ� �Լ�
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

// ��Ŷ�� ������ �Լ�
BOOL CNILayer::Send(unsigned char* ppayload, int nlength) {
	if (pcap_sendpacket(m_AdapterObject, ppayload, nlength))
	{
		AfxMessageBox(_T("�Ф� ��Ŷ ���� ����"));
		return FALSE;
	}
	return TRUE;
}

// index�� �������� ������ adapter ���� && MAC �ּ� ��ȯ
UCHAR* CNILayer::SetAdapter(const int index) {

	// device �����͸� allDevices�� �ʱ�ȭ
	device = allDevices;
	// ����ڰ� ������ �ε����� ��ġ�� ã�� ���� ����Ʈ ��ȸ
	for (int i = 0; i < index && device; i++) {
		device = device->next;
	}

	// ���õ� ��ġ��  pcap ���� ���� (��ġ �̸�, ��Ŷ ĸó ���� ũ��, promiscuous ���, timeout) ����
	m_AdapterObject = pcap_open_live(device->name, 65536, 0, 1000, errbuf);

	// pcap_open_live �Լ� ȣ�� ���� ��
	if (m_AdapterObject == nullptr)
	{
		AfxMessageBox(_T("��� ���� ����!"));
	}

	// PacketOpenAdapter�� PacketRequest �Լ��� adapter ���� �� OID ������ ����ü ��û
	adapter = PacketOpenAdapter(device->name);
	PacketRequest(adapter, FALSE, OidData);

	// OID ������ ����ü���� MAC �ּ� ������ ����
	memcpy(macAddress, (OidData->Data), 6);
	PacketCloseAdapter(adapter);

	// MAC �ּ� ��ȯ
	return macAddress;
}

// ��� ��Ʈ��ũ ��ġ(�̴��� ī�� ..) ������ adapterlist�� �߰��ϴ� �Լ�
void CNILayer::GetMacAddressList(CStringArray& adapterlist) {
	for (pcap_if_t* d = allDevices; d; d = d->next) {
		adapterlist.Add(d->description);
	}
}

// ��Ŷ ���� ������ �Լ�
// pcap_next_ex �Լ��� ��Ŷ�� �޾Ƽ� Receive �Լ��� ����
UINT CNILayer::ThreadFunction_RECEIVE(LPVOID pParam) {
	struct pcap_pkthdr* header;
	const u_char* pkt_data;
	CNILayer* pNI = (CNILayer*)pParam;

	int result = 1;
	while (pNI->canRead)
	{
		// pcap_next_ex �Լ��� ����Ͽ� ��Ŷ�� ����
		// ���� == 1 Ÿ�Ӿƿ� == 0 ���� �߻� == -1 ��ȯ
		result = pcap_next_ex(pNI->m_AdapterObject, &header, &pkt_data);
		if (result == 0) {
			//AfxMessageBox("��Ŷ ���� ����");
		}
		else if (result == 1) {
			// ��Ŷ�� �ִ� ��� �ش� �����͸� data �迭�� ����
			memcpy(pNI->data, pkt_data, ETHER_MAX_SIZE);
			//AfxMessageBox("��Ŷ �޾��� ����");
			// ������ �����͸� Receive �Լ��� ����
			pNI->Receive(pNI->data);
		}

		Sleep(1000);
	}
	return 0;
}

// canRead ������ ���� ������Ű�� �Լ�
// ��Ŷ ���� ��/���� ���� ��ȯ
void CNILayer::flip() {
	canRead = !canRead;
}