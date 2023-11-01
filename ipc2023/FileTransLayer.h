#pragma once
#include "BaseLayer.h"
#include "pch.h"

#define MAX_APP_DATA 1488 // 데이터의 최대 크기를 정의
#define SIZE_FILE_HEADER 12 // 파일 헤더의 크기를 정의
#define DONT_FRAG 0x0000 // 단편화하지 않음을 나타냄
#define FILE_INFO 0x0100 // 파일 정보를 나타냄
#define FILE_MORE 0x0200 // 더 많은 파일 데이터가 있음을 나타냄
#define FILE_LAST 0x0300 // 마지막 파일 데이터임을 나타냄


class FileTransLayer :
    public CBaseLayer
{
    CString FilePath; // 수신 또는 송신 파일의 경로를 저장
    CProgressCtrl* mPro; // progress bar controller에 대한 포인터
    CFile WriteFile; // 쓰기 작업을 위한 CFile 객체
public:
    FileTransLayer(char* name); // 생성자. 계층 이름을 인자로 받아 초기화
    ~FileTransLayer(); // 소멸자
    BOOL Receive(unsigned char* frame); // 데이터 수신 함수. frame이 가리키는 데이터를 받음
    BOOL Send(unsigned char* ppayload, int nlength); // 데이터 전송 함수. ppayload가 가리키는 데이터를 nlength만큼 전송
    // FilePath 설정 함수
    void SetFilePath(CString Path) {
        FilePath = Path;
    }
    // FilePath 반환 함수
    CString GetFilePath() {
        return FilePath;
    }
    // Header 설정을 위한 함수
    void SetHeader(
        unsigned long len,
        unsigned short type,
        unsigned long seq_num,
        unsigned char* data
    );
    // progress bar를 설정하기 위한 함수
    void SetProgressBar(CProgressCtrl* p);

    // 파일 전송을 위한 데이터 구조 정의 구조체
    typedef struct _FILE_APP {
        unsigned long   fapp_totlen; // 파일의 총 길이
        unsigned short  fapp_type; // 파일의 타입
        unsigned char   faa_msg_type; // 메시지 타입 >> 메시지가 어떤 종류인지 식별하는 데 사용됨
        unsigned char   unused; // 확장성을 위해 남겨두는 변수라 함.. (지금은 안씀)
        unsigned long   fapp_seq_num; // 시퀀스 번호 >> 패킷들이 올바른 순서로 재조립되게 하기 위해 사용됨
        unsigned char   fapp_data[MAX_APP_DATA]; // 실제 파일 데이터를 저장하는 배열 >> MAX_APP_DATA 크기만큼의 공간을 할당받음
    }FILE_APP, * PFILE_APP;

    FILE_APP mHeader; // _File_APP 타입의 멤버 변수 mHeader 선언
    static UINT FILE_SEND(LPVOID pParam); // 파일 전송을 위한 함수 
};

