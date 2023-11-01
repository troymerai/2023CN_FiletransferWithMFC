// ChatAppLayer.cpp: implemtation of the CChatAppLayer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "pch.h"
#include "ChatAppLayer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// 생성자, 멤버 변수 초기화
CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName), Head(nullptr), totalLength(0)
{}

// 소멸자
CChatAppLayer::~CChatAppLayer()
{}

// 패킷을 만드는 함수
// 데이터, 길이, 타입과 시퀀스를 인자로 받음
void CChatAppLayer::make_frame(unsigned char* ppayload, unsigned short nlength, unsigned char type, int seq) {
	unsigned short length = nlength;
	if (type == 0x00 || type == 0x03)length++;
	m_sChatApp.capp_type = type;
	m_sChatApp.capp_totlen = length;
	m_sChatApp.capp_sequence = (unsigned char)seq;

	memcpy(m_sChatApp.capp_data, ppayload + (seq * CHAR_DATA_MAX_SIZE), length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length);
	((CEthernetLayer*)(this->GetUnderLayer()))->Send((unsigned char*)&m_sChatApp, CHAT_HEADER_SIZE + (length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length), 0x2080);
}

// 데이터 전송 함수
// 데이터와 길이를 인자로 받음
BOOL CChatAppLayer::Send(unsigned char* ppayload, int nlength)
{
	//message length가 1496bytes 보다 작으면, 바로 하위 레이어로 전달
	if (nlength < CHAR_DATA_MAX_SIZE) {
		make_frame(ppayload, nlength, 0x00, 0);
		return TRUE;
	}
	//message length가 1496bytes 보다 큰 경우 단편화 
	else {
		int length = nlength - CHAR_DATA_MAX_SIZE;	//최초 단편화 frame을 고려해 1496bytes 만큼 뺀다.
		int i = 0;	//몇 번 단편화 하는지 count 변수

		//최초 단편화 된 frame을 만들어 하위 레이어로 전달
		make_frame(ppayload, nlength, 0x01, i);
		i++;

		for (; length > CHAR_DATA_MAX_SIZE; i++, length -= CHAR_DATA_MAX_SIZE) {
			//처음과 끝 부분을 제외한 중간 부분 frame을 만들어 하위 레이어로 전달
			make_frame(ppayload, CHAR_DATA_MAX_SIZE, 0x02, i);
		}
		//마지막 단편화 된 frame을 만들어 하위 레이어로 전달
		make_frame(ppayload, length, 0x03, i);
		return TRUE;
	}
	return FALSE;
}


// 새로운 FrameSeq 객체를 생성하고 이전 객체 뒤에 추가
void CChatAppLayer::add_after(FrameSeq* prev, unsigned char* data, unsigned char seq) {
	FrameSeq* tmp = new FrameSeq;
	tmp->data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);
	memcpy(tmp->data, data, CHAR_DATA_MAX_SIZE);
	tmp->seq = seq;
	tmp->next = prev->next;
	prev->next = tmp;
}

// 시퀀스 비교 
bool CChatAppLayer::seq_compare(FrameSeq* p, unsigned char seq) {
	if (p->seq < seq)return true;
	else return false;
}

// 연결 리스트에 새 요소 추가
void CChatAppLayer::add(unsigned char* data, unsigned char seq) {
	FrameSeq* p = Head;
	FrameSeq* q = nullptr;
	while (p != nullptr && seq_compare(p, seq)) {
		q = p;
		p = p->next;
	}
	if (q == nullptr)add_first(data, seq);
	else add_after(q, data, seq);
}

// 연결 리스트의 첫 부분에 요소 추가
void CChatAppLayer::add_first(unsigned char* data, unsigned char seq) {
	FrameSeq* tmp = new FrameSeq;
	tmp->data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);
	memcpy(tmp->data, data, CHAR_DATA_MAX_SIZE);
	tmp->seq = seq;
	tmp->next = Head;
	Head = tmp;
}

// 데이터 수신 함수
// 0x00: 단편화 X, 0x01: 단편화 시작, 0x02: 단편화 중간, 0x03: 단편화 끝
BOOL CChatAppLayer::Receive(unsigned char* ppayload)
{
	LPCHAT_APP chat_data = (LPCHAT_APP)ppayload; // 받은 데이터를 LPCHAT_APP 타입으로 캐스팅

	// 만약 메시지 타입이 0x00(단편화되지 않은 메시지)이면
	if (chat_data->capp_type == 0x00)
	{
		// 데이터 총 길이를 가져와서 저장
		unsigned short size = chat_data->capp_totlen;
		// 그 길이만큼 메모리 공간을 할당 받아 GetBuff 포인터에 저장 (== 그 길이 만큼 버퍼를 할당)
		unsigned char* GetBuff = (unsigned char*)malloc(sizeof(unsigned char) * (size));
		// 원본 데이터를 GetBuff로 복사
		memcpy(GetBuff, chat_data->capp_data, size);
		// 상위 계층으로 GetBuff를 전달하여 수신 처리
		this->mp_aUpperLayer[0]->Receive(GetBuff);
		return true;
	}
	else {
		// 만약 메시지 타입이 0x01(단편화 시작 메시지)이면
		if (chat_data->capp_type == 0x01) {
			// 해당 데이터와 시퀀스 번호로 연결 리스트에 추가
			add(chat_data->capp_data, chat_data->capp_sequence);
			// 전체 필요한 길이 업데이트
			totalLength = chat_data->capp_totlen + 1;
			return true;
		}
		// 만약 메시지 타입이 0x02(단편화 중간 메시지)라면
		else if (chat_data->capp_type == 0x02) {
			// 해당 데이터와 시퀀스 번호로 연결리스트에 추가
			add(chat_data->capp_data, chat_data->capp_sequence);
			return true;
		}
		// 단편화가 끝나는 경우
		else {
			// 최종적으로 재조립할 버퍼 할당
			unsigned char* GetBuff = (unsigned char*)malloc(totalLength);

			int i = 0;
			// _head가 null이 될 때까지 순회하며
			for (FrameSeq* _head = Head; _head != nullptr; i++, _head = _head->next) {
				// _head의 데이터를 버퍼에 복사
				memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), _head->data, CHAR_DATA_MAX_SIZE);
			}

			// 전체 길이가 맞지 않으면 false 반환
			if (totalLength != (i * CHAR_DATA_MAX_SIZE) + (chat_data->capp_totlen))return false;		//모든 frame이 잘 들어왔나 확인
			// 마지막 조각 복사
			memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), chat_data->capp_data, chat_data->capp_totlen);
			// 마지막에 null 문자 삽입 (끝 표시)
			GetBuff[totalLength - 1] = '\0';
			CString Msg;
			Msg.Format(_T("%s"), GetBuff);
			// 메시지 박스로 메시지 출력
			AfxMessageBox(Msg);
			// 상위 계층으로 재조립된 데이터 전달
			mp_aUpperLayer[0]->Receive((unsigned char*)Msg.GetBuffer(0));
			// 연결 리스트 삭제
			deleteList();
			return true;
		}
	}
	return false;
}

// 연결 리스트 삭제 함수
void CChatAppLayer::deleteList() {
	FrameSeq* tmp = Head;
	while (tmp != nullptr) {
		Head = Head->next;
		delete tmp;
		tmp = Head;
	}
	Head = nullptr;
}
