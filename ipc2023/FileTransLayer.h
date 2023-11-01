#pragma once
#include "BaseLayer.h"
#include "pch.h"

#define MAX_APP_DATA 1488 // �������� �ִ� ũ�⸦ ����
#define SIZE_FILE_HEADER 12 // ���� ����� ũ�⸦ ����
#define DONT_FRAG 0x0000 // ����ȭ���� ������ ��Ÿ��
#define FILE_INFO 0x0100 // ���� ������ ��Ÿ��
#define FILE_MORE 0x0200 // �� ���� ���� �����Ͱ� ������ ��Ÿ��
#define FILE_LAST 0x0300 // ������ ���� ���������� ��Ÿ��


class FileTransLayer :
    public CBaseLayer
{
    CString FilePath; // ���� �Ǵ� �۽� ������ ��θ� ����
    CProgressCtrl* mPro; // progress bar controller�� ���� ������
    CFile WriteFile; // ���� �۾��� ���� CFile ��ü
public:
    FileTransLayer(char* name); // ������. ���� �̸��� ���ڷ� �޾� �ʱ�ȭ
    ~FileTransLayer(); // �Ҹ���
    BOOL Receive(unsigned char* frame); // ������ ���� �Լ�. frame�� ����Ű�� �����͸� ����
    BOOL Send(unsigned char* ppayload, int nlength); // ������ ���� �Լ�. ppayload�� ����Ű�� �����͸� nlength��ŭ ����
    // FilePath ���� �Լ�
    void SetFilePath(CString Path) {
        FilePath = Path;
    }
    // FilePath ��ȯ �Լ�
    CString GetFilePath() {
        return FilePath;
    }
    // Header ������ ���� �Լ�
    void SetHeader(
        unsigned long len,
        unsigned short type,
        unsigned long seq_num,
        unsigned char* data
    );
    // progress bar�� �����ϱ� ���� �Լ�
    void SetProgressBar(CProgressCtrl* p);

    // ���� ������ ���� ������ ���� ���� ����ü
    typedef struct _FILE_APP {
        unsigned long   fapp_totlen; // ������ �� ����
        unsigned short  fapp_type; // ������ Ÿ��
        unsigned char   faa_msg_type; // �޽��� Ÿ�� >> �޽����� � �������� �ĺ��ϴ� �� ����
        unsigned char   unused; // Ȯ�强�� ���� ���ܵδ� ������ ��.. (������ �Ⱦ�)
        unsigned long   fapp_seq_num; // ������ ��ȣ >> ��Ŷ���� �ùٸ� ������ �������ǰ� �ϱ� ���� ����
        unsigned char   fapp_data[MAX_APP_DATA]; // ���� ���� �����͸� �����ϴ� �迭 >> MAX_APP_DATA ũ�⸸ŭ�� ������ �Ҵ����
    }FILE_APP, * PFILE_APP;

    FILE_APP mHeader; // _File_APP Ÿ���� ��� ���� mHeader ����
    static UINT FILE_SEND(LPVOID pParam); // ���� ������ ���� �Լ� 
};

