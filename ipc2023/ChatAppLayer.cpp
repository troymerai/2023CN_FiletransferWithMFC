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

// ������, ��� ���� �ʱ�ȭ
CChatAppLayer::CChatAppLayer(char* pName)
	: CBaseLayer(pName), Head(nullptr), totalLength(0)
{}

// �Ҹ���
CChatAppLayer::~CChatAppLayer()
{}

// ��Ŷ�� ����� �Լ�
// ������, ����, Ÿ�԰� �������� ���ڷ� ����
void CChatAppLayer::make_frame(unsigned char* ppayload, unsigned short nlength, unsigned char type, int seq) {
	unsigned short length = nlength;
	if (type == 0x00 || type == 0x03)length++;
	m_sChatApp.capp_type = type;
	m_sChatApp.capp_totlen = length;
	m_sChatApp.capp_sequence = (unsigned char)seq;

	memcpy(m_sChatApp.capp_data, ppayload + (seq * CHAR_DATA_MAX_SIZE), length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length);
	((CEthernetLayer*)(this->GetUnderLayer()))->Send((unsigned char*)&m_sChatApp, CHAT_HEADER_SIZE + (length > CHAR_DATA_MAX_SIZE ? CHAR_DATA_MAX_SIZE : length), 0x2080);
}

// ������ ���� �Լ�
// �����Ϳ� ���̸� ���ڷ� ����
BOOL CChatAppLayer::Send(unsigned char* ppayload, int nlength)
{
	//message length�� 1496bytes ���� ������, �ٷ� ���� ���̾�� ����
	if (nlength < CHAR_DATA_MAX_SIZE) {
		make_frame(ppayload, nlength, 0x00, 0);
		return TRUE;
	}
	//message length�� 1496bytes ���� ū ��� ����ȭ 
	else {
		int length = nlength - CHAR_DATA_MAX_SIZE;	//���� ����ȭ frame�� ����� 1496bytes ��ŭ ����.
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


// ���ο� FrameSeq ��ü�� �����ϰ� ���� ��ü �ڿ� �߰�
void CChatAppLayer::add_after(FrameSeq* prev, unsigned char* data, unsigned char seq) {
	FrameSeq* tmp = new FrameSeq;
	tmp->data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);
	memcpy(tmp->data, data, CHAR_DATA_MAX_SIZE);
	tmp->seq = seq;
	tmp->next = prev->next;
	prev->next = tmp;
}

// ������ �� 
bool CChatAppLayer::seq_compare(FrameSeq* p, unsigned char seq) {
	if (p->seq < seq)return true;
	else return false;
}

// ���� ����Ʈ�� �� ��� �߰�
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

// ���� ����Ʈ�� ù �κп� ��� �߰�
void CChatAppLayer::add_first(unsigned char* data, unsigned char seq) {
	FrameSeq* tmp = new FrameSeq;
	tmp->data = (UCHAR*)malloc(CHAR_DATA_MAX_SIZE);
	memcpy(tmp->data, data, CHAR_DATA_MAX_SIZE);
	tmp->seq = seq;
	tmp->next = Head;
	Head = tmp;
}

// ������ ���� �Լ�
// 0x00: ����ȭ X, 0x01: ����ȭ ����, 0x02: ����ȭ �߰�, 0x03: ����ȭ ��
BOOL CChatAppLayer::Receive(unsigned char* ppayload)
{
	LPCHAT_APP chat_data = (LPCHAT_APP)ppayload; // ���� �����͸� LPCHAT_APP Ÿ������ ĳ����

	// ���� �޽��� Ÿ���� 0x00(����ȭ���� ���� �޽���)�̸�
	if (chat_data->capp_type == 0x00)
	{
		// ������ �� ���̸� �����ͼ� ����
		unsigned short size = chat_data->capp_totlen;
		// �� ���̸�ŭ �޸� ������ �Ҵ� �޾� GetBuff �����Ϳ� ���� (== �� ���� ��ŭ ���۸� �Ҵ�)
		unsigned char* GetBuff = (unsigned char*)malloc(sizeof(unsigned char) * (size));
		// ���� �����͸� GetBuff�� ����
		memcpy(GetBuff, chat_data->capp_data, size);
		// ���� �������� GetBuff�� �����Ͽ� ���� ó��
		this->mp_aUpperLayer[0]->Receive(GetBuff);
		return true;
	}
	else {
		// ���� �޽��� Ÿ���� 0x01(����ȭ ���� �޽���)�̸�
		if (chat_data->capp_type == 0x01) {
			// �ش� �����Ϳ� ������ ��ȣ�� ���� ����Ʈ�� �߰�
			add(chat_data->capp_data, chat_data->capp_sequence);
			// ��ü �ʿ��� ���� ������Ʈ
			totalLength = chat_data->capp_totlen + 1;
			return true;
		}
		// ���� �޽��� Ÿ���� 0x02(����ȭ �߰� �޽���)���
		else if (chat_data->capp_type == 0x02) {
			// �ش� �����Ϳ� ������ ��ȣ�� ���Ḯ��Ʈ�� �߰�
			add(chat_data->capp_data, chat_data->capp_sequence);
			return true;
		}
		// ����ȭ�� ������ ���
		else {
			// ���������� �������� ���� �Ҵ�
			unsigned char* GetBuff = (unsigned char*)malloc(totalLength);

			int i = 0;
			// _head�� null�� �� ������ ��ȸ�ϸ�
			for (FrameSeq* _head = Head; _head != nullptr; i++, _head = _head->next) {
				// _head�� �����͸� ���ۿ� ����
				memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), _head->data, CHAR_DATA_MAX_SIZE);
			}

			// ��ü ���̰� ���� ������ false ��ȯ
			if (totalLength != (i * CHAR_DATA_MAX_SIZE) + (chat_data->capp_totlen))return false;		//��� frame�� �� ���Գ� Ȯ��
			// ������ ���� ����
			memcpy(GetBuff + (i * CHAR_DATA_MAX_SIZE), chat_data->capp_data, chat_data->capp_totlen);
			// �������� null ���� ���� (�� ǥ��)
			GetBuff[totalLength - 1] = '\0';
			CString Msg;
			Msg.Format(_T("%s"), GetBuff);
			// �޽��� �ڽ��� �޽��� ���
			AfxMessageBox(Msg);
			// ���� �������� �������� ������ ����
			mp_aUpperLayer[0]->Receive((unsigned char*)Msg.GetBuffer(0));
			// ���� ����Ʈ ����
			deleteList();
			return true;
		}
	}
	return false;
}

// ���� ����Ʈ ���� �Լ�
void CChatAppLayer::deleteList() {
	FrameSeq* tmp = Head;
	while (tmp != nullptr) {
		Head = Head->next;
		delete tmp;
		tmp = Head;
	}
	Head = nullptr;
}
