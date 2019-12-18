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
    Groups *gs=new Groups();		//?鼯????????
    Owner *curOwner;		//??????
    INode *curINode=root;			//???iNode
    vector<Direct*> ds;				//???·??????
    Dir* d=new Dir();

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
    int mkdir(INode* parent, const string& name);
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
    bool checkFileName(string name);
    // 切换工作目录
    int cd(string name);
    // 创建用户目录
    int superMkdir(INode* parent, string name, unsigned short ownerId, unsigned short groupId);
    // 改变文件权限
    int chmod(string name, unsigned int mod);
    // 改变文件拥有者
    int chown(string name, unsigned short ownerId);
    // 改变文件所属者
    int chgrp(string name, unsigned short groupId);
    // 更改当前用户密码
    int passwd();
    // 写入用户组信息
    bool writeOS();
    // 修改文件名
    int mv(string oldName, string newName);
    // 创建新文件
    int touch(INode* parent, string name);
    // 写入文件内容
    int writeText(INode* temp, string text);
    // 文件内容追加
    int textAppend(string name);
    // 显示文件内容
    int cat(string name);
    // 读取文件内容
    int readText(INode *temp);
    // 删除文件
    int rm(string name);
    // 回收空闲盘块
    int returnFreeBlock(unsigned int blockId);
    // 回收空闲节点Inode
    int returnFreeINode(unsigned int iNodeId);
    // 链接源文件至目标文件
    int ln(string source, string des);
    // 迭代初除目录下所有内容
    void rmIter(unsigned short iNodeId);
    // 删除目录操作
    int rmdir(string name);
    // 复制源文件至目标文件
    int cp(string source, string des);
    // 获取文件内容字符串
    string getText(INode* temp);
};

#endif //OSFINAL1_UNIXFILESYS_H
