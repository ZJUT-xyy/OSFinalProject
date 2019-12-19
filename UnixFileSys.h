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
    vector<Direct*> ds; // 记录所进入的目录
    Dir* d = new Dir();

public:
    // 全局初始化
    void initGlobal(FILE* f);
    // 初始化系统
    void initSystem();
    // 读内存INode节点
    bool readINode(INode* r);
    // 写内存INode节点
    bool writeINode(INode* w);
    // 写超级块
    bool writeSuperBlock();
    // 写目录
    bool writeDir(unsigned int blockId, Dir* d);
    // 读目录
    bool readDir(unsigned int blockId, Dir* d);
    // 读下一组盘块组
    bool readNextBG();
    // 创建目录
    int mkdir(INode* parent, char name[MAX_NAME_SIZE]);
    // 获取空闲盘块
    int getFreeBlock();
    // 获取空闲INode节点
    int getFreeINode();
    // 登录
    int login();
    // 指令分派器
    void commandDispatcher();
    // 显示所有命令
    void displayCommands();
    // 返回当前路径字符串
    string pwd();
    // 去除字符串两侧空格
    string trim(string s);
    // 读取当前目录
    int readCurDir();
    // 返回当前目录清单字符串
    string ls();
    // 校验文件名
    int checkFileName(char name[MAX_NAME_SIZE]);
    // 切换工作目录
    int cd(char name[MAX_NAME_SIZE]);
    // 创建用户目录
    int superMkdir(INode* parent,char name[MAX_NAME_SIZE],unsigned short ownerId,unsigned short groupId);
    // 改变文件权限
    int chmod(char name[MAX_NAME_SIZE],unsigned int mod);
    // 改变文件拥有者
    int chown(char name[MAX_NAME_SIZE],unsigned short ownerId);
    // 改变文件所属者
    int chgrp(char name[MAX_NAME_SIZE],unsigned short groupId);
    // 更改当前用户密码
    int passwd();
    // 写入用户组信息
    bool writeOS();
    // 修改文件名
    int mv(char oldName[MAX_NAME_SIZE],char newName[MAX_NAME_SIZE]);
    // 创建新文件
    int touch(INode* parent,char name[MAX_NAME_SIZE]);
    // 写入文件内容
    int writeText(INode* temp,string text);
    // 文件内容追加
    int textAppend(char name[MAX_NAME_SIZE]);
    // 显示文件内容
    int cat(char name[MAX_NAME_SIZE]);
    // 读取文件内容
    int readText(INode *temp);
    // 删除文件
    int rm(char name[MAX_NAME_SIZE]);
    // 回收空闲盘块
    int returnFreeBlock(unsigned int blockId);
    // 回收空闲节点Inode
    int returnFreeINode(unsigned int iNodeId);
    // 链接源文件至目标文件
    int ln(char source[MAX_NAME_SIZE],char des[MAX_NAME_SIZE]);
    // 迭代初除目录下所有内容
    void rmIter(unsigned short iNodeId);
    // 删除目录操作
    int rmdir(char name[MAX_NAME_SIZE]);
    // 复制源文件至目标文件
    int cp(char source[MAX_NAME_SIZE],char des[MAX_NAME_SIZE]);
    // 获取文件内容字符串
    string getText(INode* temp);
};

#endif //OSFINAL1_UNIXFILESYS_H
