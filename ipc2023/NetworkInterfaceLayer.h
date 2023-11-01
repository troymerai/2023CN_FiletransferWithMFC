#pragma once
//NetworkInterfaceLayer.h: interface for the CNetworkInterfaceLayer class.

#if !defined(AFX_NETWORKINTERFACELAYER_H)
#define AFX_NETWORKINTERFACELAYER_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseLayer.h"
#include "pch.h"
#include <pcap.h> // pcap 라이브러리 헤더 파일 포함
#include <tchar.h>
#include <Packet32.h>
#include <WinSock2.h>
#include <thread> // thread 헤더 파일 포함
#pragma comment(lib, "packet.lib")
#pragma comment(lib, "ws2_32.lib")

class CNILayer : public CBaseLayer {
    pcap_if_t* allDevices; // 모든 장치 정보 저장 변수
    pcap_if_t* device; // 선택한 장치 정보 저장 변수
    pcap_t* m_AdapterObject; // pcap 라이브러리에 조작 변수(?) >> 패킷 캡처 및 전송 시에 필요
    UCHAR data[1500]; // 패킷 데이터 저장 변수 (packet maxsize 1500byte)
    char errbuf[PCAP_ERRBUF_SIZE]; // pcap라이브러리에서 발생하는 에러 메시지 저장 버퍼
    UCHAR macAddress[6]; // MAC 주소 저장 변수
    LPADAPTER adapter = NULL; // 선택된 어댑터 정보 저장 변수 >> 패킷 캡처와 전송 시에 필요
    PPACKET_OID_DATA OidData; // Object identifier 데이터 구조체 포인터 >> 네트워크 장치 관련 작업 수행 시 필요
    bool canRead = false; // 패킷 수신 가능 여부
public:
    CNILayer(char* pName); // 생성 시에 pcap_findalldevs로 adapter 정보 저장
    ~CNILayer(); // 소멸자
    // Packet을 받아서 상위 layer(여기에서는 ethernet layer)로 전달 >> little endian, big endian 변환이 필요?
    BOOL Receive(unsigned char* pkt);
    // packet 전송 함수 >> little endian, big endian 변환이 필요?
    BOOL Send(unsigned char* ppayload, int nlength);
    // 모든 장치 정보 반환 함수
    void GetMacAddressList(CStringArray& adapterlist);
    // index를 기준으로 선택한 어댑터 설정, MAC 주소 반환
    UCHAR* SetAdapter(const int index);
    // 패킷 수신 스레드 함수
    static UINT ThreadFunction_RECEIVE(LPVOID pParam);
    // 패킷 송신 스레드 함수
    static UINT ThreadFunction_SEND(LPVOID pParam);
    // big endian <-> little endian 변환 함수
    void flip();
};

#endif // !defined(AFX_NETWORKINTERFACELAYER_H)