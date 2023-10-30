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

// ������
CEthernetLayer::CEthernetLayer(char* pName)
	: CBaseLayer(pName)
{
	// ��� �ʱ�ȭ
	ResetHeader();
}

CEthernetLayer::~CEthernetLayer()
{
}

void CEthernetLayer::ResetHeader()
{
	memset(m_sHeader.enet_dstaddr, 0, 6); // ������ �ּ� �ʵ� �ʱ�ȭ
	memset(m_sHeader.enet_srcaddr, 0, 6); // ����� �ּ� �ʵ� �ʱ�ȭ
	memset(m_sHeader.enet_data, ETHER_MAX_DATA_SIZE, 6); // ������ �ʵ� �ʱ�ȭ
	m_sHeader.enet_type = 0;
}

// Ethernet ����� �ּ� ��ȯ
unsigned char* CEthernetLayer::GetSourceAddress()
{
	return m_sHeader.enet_srcaddr;
}

// Ethernet ������ �ּ� ��ȯ
unsigned char* CEthernetLayer::GetDestinAddress()
{
	// Ethernet ������ �ּ� return
	//231004 Modify
	//return m_sHeader.??;
	return m_sHeader.enet_dstaddr;
}

void CEthernetLayer::SetSourceAddress(unsigned char* pAddress)
{
	// �Ѱܹ��� source �ּҸ� Ethernet source�ּҷ� ����
	//231004 Modify
	//memcpy(??, ??, 6);
	memcpy(m_sHeader.enet_srcaddr, pAddress, 6);
}

// �Ѱ� ���� ������ �ּҸ� Ethernet ������ �ּҷ� ����
void CEthernetLayer::SetDestinAddress(unsigned char* pAddress)
{
	memcpy(m_sHeader.enet_dstaddr, pAddress, 6);
}

BOOL CEthernetLayer::Send(unsigned char* ppayload, int nlength, unsigned short type)
{
	// ChatApp �������� ���� App ������ Frame ���̸�ŭ�� Ethernet������ data�� �ִ´�.
	memcpy(m_sHeader.enet_data, ppayload, nlength);
	// �� �������� ���� type�� ����� ���Խ�Ų��.
	m_sHeader.enet_type = type;
	BOOL bSuccess = FALSE;

	// Ethernet Data + Ethernet Header�� ����� ���� ũ�⸸ŭ�� Ethernet Frame��
	// File �������� ������.
	//231004 Modify
	//bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, ??);
	//bSuccess = mp_UnderLayer->Send((unsigned char*)&m_sHeader, ETHER_HEADER_SIZE + nlength);
	bSuccess = this->GetUnderLayer()->Send((unsigned char*)&m_sHeader, ETHER_HEADER_SIZE + nlength);
	return bSuccess;
}

BOOL CEthernetLayer::Receive(unsigned char* ppayload)
{
	// ���� �����͸� Ethernet header ����ü�� ĳ����
	PETHERNET_HEADER pFrame = (PETHERNET_HEADER)ppayload;

	BOOL bSuccess = FALSE;

	// ChatApp �������� Ethernet Frame�� data�� �Ѱ��ش�.
	//231004 Modify
	//bSuccess = mp_aUpperLayer[0]->Receive(??);
	//bSuccess = mp_aUpperLayer[0]->Receive(pFrame->enet_data);

	// �ּ� Ȯ��
	if (memcmp(pFrame->enet_dstaddr, m_sHeader.enet_srcaddr, sizeof(m_sHeader.enet_srcaddr)) == 0) {
		// enet_type�� �������� Ethernet Frame�� data�� �Ѱ��� ���̾ �����Ѵ�.
		if (pFrame->enet_type == 0x2080)
			//mp_aUpperLayer[0] == ChatApp
			bSuccess = mp_aUpperLayer[0]->Receive(pFrame->enet_data);
		else if (pFrame->enet_type == 0x2090)
			//mp_aUpperLayer[1] == FileApp
			bSuccess = mp_aUpperLayer[1]->Receive(pFrame->enet_data);
	}

	// ���� ���� ��ȯ
	return bSuccess;
}
