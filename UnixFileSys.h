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

    SuperBlock *sp = new SuperBlock();
    INode *root = new INode();
    Owners *os = new Owners();
    Groups *gs = new Groups();
    Owner *curOwner;
    INode *curINode = root;
    vector<Direct*> ds; // ��¼�������Ŀ¼
    Dir* d = new Dir();

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
    int mkdir(INode* parent, char name[MAX_NAME_SIZE]);
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
    int checkFileName(char name[MAX_NAME_SIZE]);
    // �л�����Ŀ¼
    int cd(char name[MAX_NAME_SIZE]);
    // �����û�Ŀ¼
    int superMkdir(INode* parent,char name[MAX_NAME_SIZE],unsigned short ownerId,unsigned short groupId);
    // �ı��ļ�Ȩ��
    int chmod(char name[MAX_NAME_SIZE],unsigned int mod);
    // �ı��ļ�ӵ����
    int chown(char name[MAX_NAME_SIZE],unsigned short ownerId);
    // �ı��ļ�������
    int chgrp(char name[MAX_NAME_SIZE],unsigned short groupId);
    // ���ĵ�ǰ�û�����
    int passwd();
    // д���û�����Ϣ
    bool writeOS();
    // �޸��ļ���
    int mv(char oldName[MAX_NAME_SIZE],char newName[MAX_NAME_SIZE]);
    // �������ļ�
    int touch(INode* parent,char name[MAX_NAME_SIZE]);
    // д���ļ�����
    int writeText(INode* temp,string text);
    // �ļ�����׷��
    int textAppend(char name[MAX_NAME_SIZE]);
    // ��ʾ�ļ�����
    int cat(char name[MAX_NAME_SIZE]);
    // ��ȡ�ļ�����
    int readText(INode *temp);
    // ɾ���ļ�
    int rm(char name[MAX_NAME_SIZE]);
    // ���տ����̿�
    int returnFreeBlock(unsigned int blockId);
    // ���տ��нڵ�Inode
    int returnFreeINode(unsigned int iNodeId);
    // ����Դ�ļ���Ŀ���ļ�
    int ln(char source[MAX_NAME_SIZE],char des[MAX_NAME_SIZE]);
    // ��������Ŀ¼����������
    void rmIter(unsigned short iNodeId);
    // ɾ��Ŀ¼����
    int rmdir(char name[MAX_NAME_SIZE]);
    // ����Դ�ļ���Ŀ���ļ�
    int cp(char source[MAX_NAME_SIZE],char des[MAX_NAME_SIZE]);
    // ��ȡ�ļ������ַ���
    string getText(INode* temp);
};

#endif //OSFINAL1_UNIXFILESYS_H
