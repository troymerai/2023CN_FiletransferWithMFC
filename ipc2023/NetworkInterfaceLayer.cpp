// NetworkInterfaceLayer.cpp: implementation of the NetworkInterfaceLayer class.

#include "stdafx.h"
#include "pch.h"
#include "NetworkInterfaceLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// 생성자 >> 이름을 부모 클래스에 전달 && pcap_findalldevs로 어댑터 정보 저장
CNILayer::CNILayer(char* pName)
	: CBaseLayer(pName), device(NULL) {
	// pcap_findalldevs 함수 호출 실패 시
	if (pcap_findalldevs(&allDevices, errbuf) == -1)
	{
		// 에러 메시지 출력
		fprintf(stderr, "Error in pcap_findalldevs: %s\n", errbuf);
		allDevices = NULL;
	}

	// OID 데이터 구조체 메모리 할당
	OidData = (PPACKET_OID_DATA)malloc(sizeof(PACKET_OID_DATA));
	// OID 데이터 구조체의 Oid 멤버 초기화
	OidData->Oid = 0x01010101;
	// OID 데이터 구조체의 length 멤버 초기화
	OidData->Length = 6;
}

// 소멸자 >> 동적으로 할당한 메모리를 해제 && 모든 장치 정보 삭제
CNILayer::~CNILayer() {
	pcap_if_t* temp = allDevices;

	while (temp) {
		temp = allDevices;
		allDevices = allDevices->next;
		delete(temp);
		delete(OidData);
	}
}

// 패킷을 받아 상위 계층(여기서는 Ethernet Layer)로 전달하는 함수
BOOL CNILayer::Receive(unsigned char* pkt) {
	//endian 변환 필요?
	if (pkt == nullptr) {
		return FALSE;
	}
	if (!(mp_aUpperLayer[0]->Receive(pkt))) { // 넘겨주지 못했다면 FALSE
		return FALSE;
	}

	return TRUE;
}

// 패킷을 보내는 함수
BOOL CNILayer::Send(unsigned char* ppayload, int nlength) {
	if (pcap_sendpacket(m_AdapterObject, ppayload, nlength))
	{
		AfxMessageBox(_T("ㅠㅠ 패킷 전송 실패"));
		return FALSE;
	}
	return TRUE;
}

// index를 기준으로 선택한 adapter 설정 && MAC 주소 반환
UCHAR* CNILayer::SetAdapter(const int index) {

	// device 포인터를 allDevices로 초기화
	device = allDevices;
	// 사용자가 선택한 인덱스의 장치를 찾기 위해 리스트 순회
	for (int i = 0; i < index && device; i++) {
		device = device->next;
	}

	// 선택된 장치로  pcap 세션 열기 (장치 이름, 패킷 캡처 버퍼 크기, promiscuous 모드, timeout) 설정
	m_AdapterObject = pcap_open_live(device->name, 65536, 0, 1000, errbuf);

	// pcap_open_live 함수 호출 실패 시
	if (m_AdapterObject == nullptr)
	{
		AfxMessageBox(_T("어뎁터 연결 실패!"));
	}

	// PacketOpenAdapter와 PacketRequest 함수로 adapter 정보 및 OID 데이터 구조체 요청
	adapter = PacketOpenAdapter(device->name);
	PacketRequest(adapter, FALSE, OidData);

	// OID 데이터 구조체에서 MAC 주소 정보를 복사
	memcpy(macAddress, (OidData->Data), 6);
	PacketCloseAdapter(adapter);

	// MAC 주소 반환
	return macAddress;
}

// 모든 네트워크 장치(이더넷 카드 ..) 정보를 adapterlist에 추가하는 함수
void CNILayer::GetMacAddressList(CStringArray& adapterlist) {
	for (pcap_if_t* d = allDevices; d; d = d->next) {
		adapterlist.Add(d->description);
	}
}

// 패킷 수신 스레드 함수
// pcap_next_ex 함수로 패킷을 받아서 Receive 함수로 전달
UINT CNILayer::ThreadFunction_RECEIVE(LPVOID pParam) {
	struct pcap_pkthdr* header;
	const u_char* pkt_data;
	CNILayer* pNI = (CNILayer*)pParam;

	int result = 1;
	while (pNI->canRead)
	{
		// pcap_next_ex 함수를 사용하여 패킷을 읽음
		// 성공 == 1 타임아웃 == 0 에러 발생 == -1 반환
		result = pcap_next_ex(pNI->m_AdapterObject, &header, &pkt_data);
		if (result == 0) {
			//AfxMessageBox("패킷 없음 ㅋㅋ");
		}
		else if (result == 1) {
			// 패킷이 있는 경우 해당 데이터를 data 배열에 복사
			memcpy(pNI->data, pkt_data, ETHER_MAX_SIZE);
			//AfxMessageBox("패킷 받았음 ㅋㅋ");
			// 복사한 데이터를 Receive 함수로 전달
			pNI->Receive(pNI->data);
		}

		Sleep(1000);
	}
	return 0;
}

// canRead 변수의 값을 반전시키는 함수
// 패킷 수신 불/가능 상태 전환
void CNILayer::flip() {
	canRead = !canRead;
}