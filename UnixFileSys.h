//
// Created by xyy on 2019/12/17.
//
#include <iostream>
#include <string>
#include <vector>
#include "Entity/SuperBlock.h"
#include "Entity/INode.h"
#include "Entity/Owners.h"
#include "Entity/Groups.h"
#include "Entity/Direct.h"
#include "Entity/Dir.h"

using namespace std;

#ifndef OSFINAL1_UNIXFILESYS_H
#define OSFINAL1_UNIXFILESYS_H

class UnixFIleSys {

    SuperBlock *sp=new SuperBlock();		//????????????
    INode *root=new INode();				//??????????
    Owners *os=new Owners();		//???????????
    Groups *gs=new Groups();		//?��????????
    Owner *curOwner;		//??????
    INode *curINode=root;			//???iNode
    vector<Direct*> ds;				//???��??????
    Dir* d=new Dir();

public:
    // ȫ�ֳ�ʼ��
    void initGlobal(FILE* f);
    // ��ʼ��ϵͳ
    void initSystem();
    // ���ڴ�INode�ڵ�
    bool readINode(INode* r);
    // д�ڴ�INode�ڵ�
    bool writeINode(INode* w);
    // д������
    bool writeSuperBlock();
    // дĿ¼
    bool writeDir(unsigned int blockId, Dir* d);
    // ��Ŀ¼
    bool readDir(unsigned int blockId, Dir* d);
    // ����һ���̿���
    bool readNextBG();
    // ����Ŀ¼
    int mkdir(INode* parent, const string& name);
    // ��ȡ�����̿�
    int getFreeBlock();
    // ��ȡ����INode�ڵ�
    int getFreeINode();
    // ��¼
    int login();
    // ָ�������
    void commandDispatcher();
    // ��ʾ��������
    void displayCommands();
    // ���ص�ǰ·���ַ���
    string pwd();
    // ȥ���ַ�������ո�
    string trim(string s);
    // ��ȡ��ǰĿ¼
    int readCurDir();
    // ���ص�ǰĿ¼�嵥�ַ���
    string ls();
    // У���ļ���
    bool checkFileName(string name);
    // �л�����Ŀ¼
    int cd(string name);
    // �����û�Ŀ¼
    int superMkdir(INode* parent, string name, unsigned short ownerId, unsigned short groupId);
    // �ı��ļ�Ȩ��
    int chmod(string name, unsigned int mod);
    // �ı��ļ�ӵ����
    int chown(string name, unsigned short ownerId);
    // �ı��ļ�������
    int chgrp(string name, unsigned short groupId);
    // ���ĵ�ǰ�û�����
    int passwd();
    // д���û�����Ϣ
    bool writeOS();
    // �޸��ļ���
    int mv(string oldName, string newName);
    // �������ļ�
    int touch(INode* parent, string name);
    // д���ļ�����
    int writeText(INode* temp, string text);
    // �ļ�����׷��
    int textAppend(string name);
    // ��ʾ�ļ�����
    int cat(string name);
    // ��ȡ�ļ�����
    int readText(INode *temp);
    // ɾ���ļ�
    int rm(string name);
    // ���տ����̿�
    int returnFreeBlock(unsigned int blockId);
    // ���տ��нڵ�Inode
    int returnFreeINode(unsigned int iNodeId);
    // ����Դ�ļ���Ŀ���ļ�
    int ln(string source, string des);
    // ��������Ŀ¼����������
    void rmIter(unsigned short iNodeId);
    // ɾ��Ŀ¼����
    int rmdir(string name);
    // ����Դ�ļ���Ŀ���ļ�
    int cp(string source, string des);
    // ��ȡ�ļ������ַ���
    string getText(INode* temp);
};

#endif //OSFINAL1_UNIXFILESYS_H
