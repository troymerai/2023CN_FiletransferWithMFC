// EthernetLayer.cpp: implementation of the CEthernetLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch.h"
#include "EthernetLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// 생성자
CEthernetLayer::CEthernetLayer(char* pName)
	: CBaseLayer(pName)
{
	// 헤더 초기화
	ResetHeader();
}

CEthernetLayer::~CEthernetLayer()
{
}

void CEthernetLayer::ResetHeader()
{
	memset(m_sHeader.enet_dstaddr, 0, 6); // 목적지 주소 필드 초기화
	memset(m_sHeader.enet_srcaddr, 0, 6); // 출발지 주소 필드 초기화
	memset(m_sHeader.enet_data, ETHER_MAX_DATA_SIZE, 6); // 데이터 필드 초기화
	m_sHeader.enet_type = 0;
}

// Ethernet 출발지 주소 반환
unsigned char* CEthernetLayer::GetSourceAddress()
{
	return m_sHeader.enet_srcaddr;
}

// Ethernet 목적지 주소 반환
unsigned char* CEthernetLayer::GetDestinAddress()
{
	// Ethernet 목적지 주소 return
	//231004 Modify
	//return m_sHeader.??;
	return m_sHeader.enet_dstaddr;
}

void CEthernetLayer::SetSourceAddress(unsigned char* pAddress)
{
	// 넘겨받은 source 주소를 Ethernet source주소로 지정
	//231004 Modify
	//memcpy(??, ??, 6);
	memcpy(m_sHeader.enet_srcaddr, pAddress, 6);
}

// 넘겨 받은 목적지 주소를 Ethernet 목적지 주소로 지정
void CEthernetLayer::SetDestinAddress(unsigned char* pAddress)
{
	memcpy(m_sHeader.enet_dstaddr, pAddress, 6);
}

BOOL CEthernetLayer::Send(unsigned char* ppayload, int nlength, unsigned short type)
{
	// ChatApp 계층에서 받은 App 계층의 Frame 길이만큼을 Ethernet계층의 data로 넣는다.
	memcpy(m_sHeader.enet_data, ppayload, nlength);
	// 윗 계층에서 받은 type도 헤더에 포함시킨다.
	m_sHeader.enet_type = type;
	BOOL bSuccess = FALSE;

	// Ethernet Data + Ethernet Header의 사이즈를 합한 크기만큼의 Ethernet Frame을
	// File 계층으로 보낸다.
	//231004 Modify
	//bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, ??);
	//bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, ETHER_HEADER_SIZE + nlength);
	bSuccess = this->GetUnderLayer()->Send((unsigned char*)&m_sHeader, ETHER_HEADER_SIZE + nlength);
	return bSuccess;
}

BOOL CEthernetLayer::Receive(unsigned char* ppayload)
{
	// 받은 데이터를 Ethernet header 구조체로 캐스팅
	PETHERNET_HEADER pFrame = (PETHERNET_HEADER)ppayload;

	BOOL bSuccess = FALSE;

	// ChatApp 계층으로 Ethernet Frame의 data를 넘겨준다.
	//231004 Modify
	//bSuccess = mp_aUpperLayer[0]->Receive(??);
	//bSuccess = mp_aUpperLayer[0]->Receive(pFrame->enet_data);

	// 주소 확인
	if (memcmp(pFrame->enet_dstaddr, m_sHeader.enet_srcaddr, sizeof(m_sHeader.enet_srcaddr)) == 0) {
		// enet_type을 기준으로 Ethernet Frame의 data를 넘겨줄 레이어를 지정한다.
		if (pFrame->enet_type == 0x2080)
			//mp_aUpperLayer[0] == ChatApp
			bSuccess = mp_aUpperLayer[0]->Receive(pFrame->enet_data);
		else if (pFrame->enet_type == 0x2090)
			//mp_aUpperLayer[1] == FileApp
			bSuccess = mp_aUpperLayer[1]->Receive(pFrame->enet_data);
	}

	// 성공 여부 반환
	return bSuccess;
}
