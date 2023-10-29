// FileTransLayer.cpp : 파일을 패킷으로 분할하여 송/수신하는 기능을 구현

#include "pch.h"
#include "FileTransLayer.h"

// 생성자, 필드 초기화
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

// 소멸자
FileTransLayer::~FileTransLayer() {
	return;
}

// Ethernet Layer에 데이터, 길이, 프레임 타입(== '0x2090')을 전달하여 실제 패킷 전송을 요청
BOOL FileTransLayer::Send(unsigned char* ppayload, int nlength) {
	return ((CEthernetLayer*)(this->GetUnderLayer()))->Send((unsigned char*)&mHeader, nlength, 0x2090);
}

// Ethernet Layer에서 받은 frame을 처리하는 역할
BOOL FileTransLayer::Receive(unsigned char* frame) {
	PFILE_APP data = (PFILE_APP)frame;

	// 파일 정보 패킷일 경우
	if (data->fapp_type == FILE_INFO) {
		// 해당 파일 이름으로 새 파일 객체 생성 및 크기 설정
		CString file_name;
		file_name.Format(_T("%s"), data->fapp_data);
		WriteFile.Open(file_name, CFile::modeCreate | CFile::modeWrite);
		WriteFile.SetLength(data->fapp_totlen);
	}
	// 파일의 일부 데이터 패킷일 경우
	else if (data->fapp_type == FILE_MORE) {
		// 해당 시퀀스 번호 위치에 데이터 작성
		WriteFile.Seek(data->fapp_seq_num * MAX_APP_DATA, CFile::begin);
		WriteFile.Write(data->fapp_data, data->fapp_totlen);
	}
	// 그 외의 경우는 파일 전송이 끝났다고 가정하고 파일을 닫고 사용자에게 알림
	else {
		WriteFile.Close();
		AfxMessageBox(_T("file transfered successfully!"));
	}

	return true;
}

// 주어진 parameter로 헤더를 설정하는 역할
void FileTransLayer::SetHeader(
	unsigned long len,
	unsigned short type,
	unsigned long seq_num,
	unsigned char* data
) {
	mHeader.fapp_totlen = len; // 전체 길이 설정
	mHeader.fapp_type = type; // 타입 설정
	mHeader.fapp_seq_num = seq_num; // 시퀀스 번호 설정
	// 전체 길이가 MAX_APP_DATA보다 크면 최대치인 MAX_APP_DATA만큼만 복사
	memcpy(mHeader.fapp_data, data, len > MAX_APP_DATA ? MAX_APP_DATA : len);
}

// progressbar 객체의 참조를 받아 저장
void FileTransLayer::SetProgressBar(CProgressCtrl* p) {
	mPro = p;
}

// 실제 파일 전송 작업을 수행하는 스레드
UINT FileTransLayer::FILE_SEND(LPVOID pParam) {
	// 매개변수로 넘어온 객체 참조를 캐스팅하여 사용
	FileTransLayer* pFL = (FileTransLayer*)pParam;
	CFile SendFile;
	unsigned long totallength;
	unsigned char buffer[MAX_APP_DATA];
	unsigned int seq = -1;

	// 보낼 파일 열기
	SendFile.Open(pFL->GetFilePath(), CFile::modeRead);
	// 보낼 파일의 총 길이 계산
	totallength = SendFile.GetLength();
	// CArchive 객체를 생성, 보낼 파일 연결 >> 이 객체로 파일 내용을 읽어옴
	CArchive read_file(&SendFile, CArchive::load);
	// progress bar의 범위를 설정 >> 최대값은 전체 길이를 패킷 크기(MAX_APP_DATA)로 나눈 값
	pFL->mPro->SetRange(0, totallength / MAX_APP_DATA);

	// 첫 번째 패킷인 FILE_INFO 타입의 헤더 정보를 설정하고 전송
	// 전체 길이와 파일 이름 포함
	pFL->SetHeader(totallength, FILE_INFO, seq, (unsigned char*)(SendFile.GetFileName().GetBuffer(0)));
	pFL->Send((unsigned char*)&(pFL->mHeader), SIZE_FILE_HEADER + SendFile.GetFileName().GetLength() + 1);

	// read_file에서 MAX_APP_DATA 크기 만큼 읽어서 buffer에 저장,
	// 그 크기만큼 FILE_MORE 타입의 패킷으로 전송
	// 이 과정을 더 이상 읽어올 데이터가 없을 때까지 반복
	while (read_file.Read(buffer, MAX_APP_DATA)) {
		// 헤더 설정
		pFL->SetHeader(totallength > MAX_APP_DATA ? MAX_APP_DATA : totallength, FILE_MORE, ++seq, buffer);
		// 패킷 전송
		pFL->Send((unsigned char*)&(pFL->mHeader), SIZE_FILE_HEADER + (totallength > MAX_APP_DATA ? MAX_APP_DATA : totallength));
		// 남은 데이터 길이 갱신
		totallength -= MAX_APP_DATA;
		// progress bar 위치 갱신
		pFL->mPro->SetPos(seq);
	}

	// 모든 데이터를 전송한 후 FILE_LAST 타입의 패킷을 전송하여 파일 전송이 끝났음을 알림
	pFL->SetHeader(0, FILE_LAST, ++seq, (unsigned char*)"aaaaaaaa");
	pFL->Send((unsigned char*)&(pFL->mHeader), SIZE_FILE_HEADER + 8);

	SendFile.Close(); // 파일 닫기
	read_file.Close(); // CArchive 객체 닫기 


	return 0;
}
