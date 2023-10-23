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


CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName), Head(nullptr), totalLength(0)
{}

CChatAppLayer::~CChatAppLayer()
{}

void CChatAppLayer::make_frame(unsigned char* ppayload, unsigned short nlength, unsigned char type,int seq){
	unsigned short length = nlength;
	if (type == 0x00 || type == 0x03)length++;
	m_sChatApp.capp_type = type;
	m_sChatApp.capp_totlen = length;
	m_sChatApp.capp_sequence = (unsigned char)seq;
	//memset(m_sChatApp.capp_data, 0, length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length);
	memcpy(m_sChatApp.capp_data, ppayload+(seq * CHAR_DATA_MAX_SIZE), length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length);
	((CEthernetLayer*)(this->GetUnderLayer()))->Send((unsigned char*)&m_sChatApp, CHAT_HEADER_SIZE + (length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length), 0x2080);
}


BOOL CChatAppLayer::Send(unsigned char* ppayload, int nlength)
{
	//message length�� 1496bytes ���� ������, �ٷ� ���� ���̾�� ����
	if (nlength < CHAR_DATA_MAX_SIZE) {
		make_frame(ppayload, nlength, 0x00, 0);
		return TRUE;
	}
	//message length�� 1496bytes ���� ū ��� ����ȭ �۾�...
	else {
		int length = nlength-CHAR_DATA_MAX_SIZE;	//���� ����ȭ frame�� ����� 1496bytes ��ŭ ����.
		int i = 0;	//�� �� ����ȭ �ϴ��� count ����

		//���� ����ȭ �� frame�� ����� ���� ���̾�� ����
		make_frame(ppayload, nlength, 0x01, i);
		i++;

		for (; length > CHAR_DATA_MAX_SIZE; i++, length -= CHAR_DATA_MAX_SIZE) {
			//ó���� �� �κ��� ������ �߰� �κ� frame�� ����� ���� ���̾�� ����
			make_frame(ppayload, CHAR_DATA_MAX_SIZE, 0x02, i);
		}
		//������ ����ȭ �� frame�� ����� ���� ���̾�� ����
		make_frame(ppayload, length, 0x03, i);
		return TRUE;
	}
	return FALSE;
}



void CChatAppLayer::add_after(FrameSeq* prev, unsigned char* data, unsigned char seq){
	FrameSeq* tmp = new FrameSeq;
	tmp->data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);
	memcpy(tmp->data, data, CHAR_DATA_MAX_SIZE);
	tmp->seq = seq;
	tmp->next = prev->next;
	prev->next = tmp;
}

bool CChatAppLayer::seq_compare(FrameSeq* p, unsigned char seq){
	if (p->seq < seq)return true;
	else return false;
}

void CChatAppLayer::add(unsigned char* data, unsigned char seq){
	FrameSeq* p = Head;
	FrameSeq* q = nullptr;
	while (p!=nullptr && seq_compare(p, seq)) {
		q = p;
		p = p->next;
	}
	if (q == nullptr)add_first(data, seq);
	else add_after(q, data, seq);
}

void CChatAppLayer::add_first(unsigned char* data, unsigned char seq) {
	FrameSeq* tmp = new FrameSeq;
	tmp->data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);
	memcpy(tmp->data, data, CHAR_DATA_MAX_SIZE);
	tmp->seq = seq;
	tmp->next = Head;
	Head = tmp;
}

BOOL CChatAppLayer::Receive(unsigned char* ppayload)
{
	LPCHAT_APP chat_data = (LPCHAT_APP)ppayload;

	if (chat_data->capp_type == 0x00)
	{
		unsigned short size = chat_data->capp_totlen;
		unsigned char *GetBuff=(unsigned char*)malloc(sizeof(unsigned char)*(size));
		//memset(GetBuff, '\0', size);  // GetBuff�� �ʱ�ȭ���ش�.
		memcpy(GetBuff, chat_data->capp_data, size);
		this->mp_aUpperLayer[0]->Receive(GetBuff);
		return true;
	}else {
		if (chat_data->capp_type == 0x01) {
			add(chat_data->capp_data, chat_data->capp_sequence);
			totalLength = chat_data->capp_totlen + 1;
			return true;
		}
		else if (chat_data->capp_type == 0x02) {
			add(chat_data->capp_data, chat_data->capp_sequence);
			return true;
		}
		else {
			unsigned char* GetBuff = (unsigned char*)malloc(totalLength);
			//memset(GetBuff, '\0', totalLength);  // GetBuff�� �ʱ�ȭ���ش�.
			int i = 0;
			for (FrameSeq *_head = Head; _head != nullptr; i++, _head = _head->next) {
				memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), _head->data, CHAR_DATA_MAX_SIZE);
			}
			if (totalLength != (i * CHAR_DATA_MAX_SIZE) + (chat_data->capp_totlen))return false;		//��� frame�� �� ���Գ� Ȯ��
			memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), chat_data->capp_data, chat_data->capp_totlen);
			GetBuff[totalLength-1] = '\0';
			CString Msg;
			Msg.Format(_T("%s"), GetBuff);
			AfxMessageBox(Msg);
			mp_aUpperLayer[0]->Receive((unsigned char*)Msg.GetBuffer(0));
			deleteList();
			return true;
		}
	}
	return false;
}

void CChatAppLayer::deleteList(){
	FrameSeq* tmp = Head;
	while (tmp != nullptr) {
		Head = Head->next;
		delete tmp;
		tmp = Head;
	}
	Head = nullptr;
}
