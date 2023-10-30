// FileTransLayer.cpp : ������ ��Ŷ���� �����Ͽ� ��/�����ϴ� ����� ����

#include "pch.h"
#include "FileTransLayer.h"

// ������, �ʵ� �ʱ�ȭ
FileTransLayer::FileTransLayer(char* name) :CBaseLayer(name) {
	FilePath = "";
	mHeader.fapp_totlen = 0;
	mHeader.fapp_type = 0;
	mHeader.fapp_seq_num = 0;
	mHeader.faa_msg_type = 0;
	mHeader.unused = 0;
	memset(mHeader.fapp_data, 0, MAX_APP_DATA);
	return;
}

// �Ҹ���
FileTransLayer::~FileTransLayer() {
	return;
}

// Ethernet Layer�� ������, ����, ������ Ÿ��(== '0x2090')�� �����Ͽ� ���� ��Ŷ ������ ��û
BOOL FileTransLayer::Send(unsigned char* ppayload, int nlength) {
	return ((CEthernetLayer*)(this->GetUnderLayer()))->Send((unsigned char*)&mHeader, nlength, 0x2090);
}

// Ethernet Layer���� ���� frame�� ó���ϴ� ����
BOOL FileTransLayer::Receive(unsigned char* frame) {
	PFILE_APP data = (PFILE_APP)frame;

	// ���� ���� ��Ŷ�� ���
	if (data->fapp_type == FILE_INFO) {
		// �ش� ���� �̸����� �� ���� ��ü ���� �� ũ�� ����
		CString file_name;
		file_name.Format(_T("%s"), data->fapp_data);
		WriteFile.Open(file_name, CFile::modeCreate | CFile::modeWrite);
		WriteFile.SetLength(data->fapp_totlen);
	}
	// ������ �Ϻ� ������ ��Ŷ�� ���
	else if (data->fapp_type == FILE_MORE) {
		// �ش� ������ ��ȣ ��ġ�� ������ �ۼ�
		WriteFile.Seek(data->fapp_seq_num * MAX_APP_DATA, CFile::begin);
		WriteFile.Write(data->fapp_data, data->fapp_totlen);
	}
	// �� ���� ���� ���� ������ �����ٰ� �����ϰ� ������ �ݰ� ����ڿ��� �˸�
	else {
		WriteFile.Close();
		AfxMessageBox(_T("file transfered successfully!"));
	}

	return true;
}

// �־��� parameter�� ����� �����ϴ� ����
void FileTransLayer::SetHeader(
	unsigned long len,
	unsigned short type,
	unsigned long seq_num,
	unsigned char* data
) {
	mHeader.fapp_totlen = len; // ��ü ���� ����
	mHeader.fapp_type = type; // Ÿ�� ����
	mHeader.fapp_seq_num = seq_num; // ������ ��ȣ ����
	// ��ü ���̰� MAX_APP_DATA���� ũ�� �ִ�ġ�� MAX_APP_DATA��ŭ�� ����
	memcpy(mHeader.fapp_data, data, len > MAX_APP_DATA ? MAX_APP_DATA : len);
}

// progressbar ��ü�� ������ �޾� ����
void FileTransLayer::SetProgressBar(CProgressCtrl* p) {
	mPro = p;
}

// ���� ���� ���� �۾��� �����ϴ� ������
UINT FileTransLayer::FILE_SEND(LPVOID pParam) {
	// �Ű������� �Ѿ�� ��ü ������ ĳ�����Ͽ� ���
	FileTransLayer* pFL = (FileTransLayer*)pParam;
	CFile SendFile;
	unsigned long totallength;
	unsigned char buffer[MAX_APP_DATA];
	unsigned int seq = -1;

	// ���� ���� ����
	SendFile.Open(pFL->GetFilePath(), CFile::modeRead);
	// ���� ������ �� ���� ���
	totallength = SendFile.GetLength();
	// CArchive ��ü�� ����, ���� ���� ���� >> �� ��ü�� ���� ������ �о��
	CArchive read_file(&SendFile, CArchive::load);
	// progress bar�� ������ ���� >> �ִ밪�� ��ü ���̸� ��Ŷ ũ��(MAX_APP_DATA)�� ���� ��
	pFL->mPro->SetRange(0, totallength / MAX_APP_DATA);

	// ù ��° ��Ŷ�� FILE_INFO Ÿ���� ��� ������ �����ϰ� ����
	// ��ü ���̿� ���� �̸� ����
	pFL->SetHeader(totallength, FILE_INFO, seq, (unsigned char*)(SendFile.GetFileName().GetBuffer(0)));
	pFL->Send((unsigned char*)&(pFL->mHeader), SIZE_FILE_HEADER + SendFile.GetFileName().GetLength() + 1);

	// read_file���� MAX_APP_DATA ũ�� ��ŭ �о buffer�� ����,
	// �� ũ�⸸ŭ FILE_MORE Ÿ���� ��Ŷ���� ����
	// �� ������ �� �̻� �о�� �����Ͱ� ���� ������ �ݺ�
	while (read_file.Read(buffer, MAX_APP_DATA)) {
		// ��� ����
		pFL->SetHeader(totallength > MAX_APP_DATA ? MAX_APP_DATA : totallength, FILE_MORE, ++seq, buffer);
		// ��Ŷ ����
		pFL->Send((unsigned char*)&(pFL->mHeader), SIZE_FILE_HEADER + (totallength > MAX_APP_DATA ? MAX_APP_DATA : totallength));
		// ���� ������ ���� ����
		totallength -= MAX_APP_DATA;
		// progress bar ��ġ ����
		pFL->mPro->SetPos(seq);
	}

	// ��� �����͸� ������ �� FILE_LAST Ÿ���� ��Ŷ�� �����Ͽ� ���� ������ �������� �˸�
	pFL->SetHeader(0, FILE_LAST, ++seq, (unsigned char*)"aaaaaaaa");
	pFL->Send((unsigned char*)&(pFL->mHeader), SIZE_FILE_HEADER + 8);

	SendFile.Close(); // ���� �ݱ�
	read_file.Close(); // CArchive ��ü �ݱ� 


	return 0;
}
