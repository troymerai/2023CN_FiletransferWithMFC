#pragma once
//NetworkInterfaceLayer.h: interface for the CNetworkInterfaceLayer class.

#if !defined(AFX_NETWORKINTERFACELAYER_H)
#define AFX_NETWORKINTERFACELAYER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include "pch.h"
#include <pcap.h> // pcap ���̺귯�� ��� ���� ����
#include <tchar.h>
#include <Packet32.h>
#include <WinSock2.h>
#include <thread> // thread ��� ���� ����
#pragma comment(lib, "packet.lib")
#pragma comment(lib, "ws2_32.lib")

class CNILayer : public CBaseLayer {
    pcap_if_t* allDevices; // ��� ��ġ ���� ���� ����
    pcap_if_t* device; // ������ ��ġ ���� ���� ����
    pcap_t* m_AdapterObject; // pcap ���̺귯���� ���� ����(?) >> ��Ŷ ĸó �� ���� �ÿ� �ʿ�
    UCHAR data[1500]; // ��Ŷ ������ ���� ���� (packet maxsize 1500byte)
    char errbuf[PCAP_ERRBUF_SIZE]; // pcap���̺귯������ �߻��ϴ� ���� �޽��� ���� ����
    UCHAR macAddress[6]; // MAC �ּ� ���� ����
    LPADAPTER adapter = NULL; // ���õ� ����� ���� ���� ���� >> ��Ŷ ĸó�� ���� �ÿ� �ʿ�
    PPACKET_OID_DATA OidData; // Object identifier ������ ����ü ������ >> ��Ʈ��ũ ��ġ ���� �۾� ���� �� �ʿ�
    bool canRead = false; // ��Ŷ ���� ���� ����
public:
    CNILayer(char* pName); // ���� �ÿ� pcap_findalldevs�� adapter ���� ����
    ~CNILayer(); // �Ҹ���
    // Packet�� �޾Ƽ� ���� layer(���⿡���� ethernet layer)�� ���� >> little endian, big endian ��ȯ�� �ʿ�?
    BOOL Receive(unsigned char* pkt);
    // packet ���� �Լ� >> little endian, big endian ��ȯ�� �ʿ�?
    BOOL Send(unsigned char* ppayload, int nlength);
    // ��� ��ġ ���� ��ȯ �Լ�
    void GetMacAddressList(CStringArray& adapterlist);
    // index�� �������� ������ ����� ����, MAC �ּ� ��ȯ
    UCHAR* SetAdapter(const int index);
    // ��Ŷ ���� ������ �Լ�
    static UINT ThreadFunction_RECEIVE(LPVOID pParam);
    // ��Ŷ �۽� ������ �Լ�
    static UINT ThreadFunction_SEND(LPVOID pParam);
    // big endian <-> little endian ��ȯ �Լ�
    void flip();
};

#endif // !defined(AFX_NETWORKINTERFACELAYER_H)