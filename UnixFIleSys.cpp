//
// Created by xyy on 2019/12/17.
//
#include <cstring>
#include <cstdio>
#include <ctime>
#include "UnixFileSys.h"
#include "status.h"

// 初始化系列操作 ///////////////////////////////////////////////////////////////////////////////////////////////////////
// 初始化数据
void UnixFIleSys :: initGlobal(FILE* f) {
    cout << "现在开始初始化数据" << endl;
    // 初始化超级块
    sp -> size = DISK_SIZE;
    sp -> freeBlockNum = BLOCK_NUM; // 初始所有的盘块都为空闲盘块
    sp -> freeINodeNum = INODE_NUM - 1;
    for (int j = 0; j < INODE_NUM; j ++)
        sp -> freeINode[j] = INODE_NUM + 1 - j; // 为每一个INode初始化一个INodeId
    sp -> nextFreeINode = INODE_NUM - 2;

    // 初始化根INode
    root -> parent = nullptr; // 根目录的父目录为空
    root -> nodeId = 2;
    root -> users = 1;
    time_t now;
    now = time(nullptr);
    root -> dINode.createTime = now;
    root -> dINode.fileSize = 0;
    root -> dINode.groupId = 0;
    root -> dINode.linkNum = 0;
    root -> dINode.mod = 14;
    root -> dINode.modifyTime = now;
    root -> dINode.ownerId = 1;
    root -> dINode.readTime = now;

    // 初始化用户
    os -> ownerNum = 2;
    os -> os[0].ownerId = 1;
    os -> os[0].groupId = 1;
    strcpy(os -> os[0].ownerName, "osfinal");
    strcpy(os -> os[0].ownerPassword, "osfinal");
    os -> os[1].ownerId = 2;
    os -> os[1].groupId = 2;
    strcpy(os -> os[1].ownerName, "osfinal2");
    strcpy(os -> os[1].ownerPassword, "osfinal2");

    // 初始化用户组
    gs -> groupNum = 2;
    gs -> gs[0].groupId = 1;
    strcpy(gs -> gs[0].groupName, "zjut");
    gs -> gs[1].groupId = 2;
    strcpy(gs -> gs[1].groupName, "zjut2");

    f = fopen(FILE_PATH,"wb"); // wb - 只写打开或新建一个二进制文件

    int bGs = BLOCK_NUM / BLOCK_GROUP_SIZE; // 有多少组
    int left = BLOCK_NUM - bGs * BLOCK_GROUP_SIZE; // 整组外剩下的块数

    fseek(f, DISK_SIZE, SEEK_SET);
    fwrite("?", 1, 1, f);

    // 如果为整组，没有零碎块
    if (left == 0) {
        for (int i = 0; i < BLOCK_GROUP_SIZE; i ++)
            sp -> freeBlock[i] = BLOCK_START + BLOCK_GROUP_SIZE - 1 - i; // 初始化当前组的块号
        sp -> nextFreeBlock = BLOCK_GROUP_SIZE - 1; // 下一个空闲块在组内的偏移数
        // 按组初始化全部块号，并按组写入磁盘
        for (int j = 0; j < bGs; j ++) {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            unsigned int blocks[BLOCK_GROUP_SIZE];
            for (int k = 0; k < BLOCK_GROUP_SIZE; k ++)
                blocks[k] = left + BLOCK_START + BLOCK_GROUP_SIZE * (j + 1) - k - 1; // 初始化全部块号
            fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * j - 1), SEEK_SET);
            fwrite(&blocksNum, sizeof(blocksNum), 1, f);
            fwrite(&blocks, sizeof(blocks), 1, f);
        }
        unsigned int blocksNum = 0;
        unsigned int blocks[BLOCK_GROUP_SIZE];
        fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * bGs - 1), SEEK_SET);
        fwrite(&blocksNum, sizeof(blocksNum), 1, f);
        fwrite(&blocks, sizeof(blocks), 1, f);
    } else {
    // 如果有零碎块，零碎块自成一组，该组放在空闲组栈的栈顶
        for (int i = 0; i < left; i ++)
            sp -> freeBlock[i] = BLOCK_START + left - 1 - i;
        sp -> nextFreeBlock = left - 1; // 下一个空闲块在组内的偏移数
        // 按组初始化全部块号，并按组写入磁盘
        for(int j = 0; j < bGs; j ++) {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            unsigned int blocks[BLOCK_GROUP_SIZE];
            for (int k = 0; k < BLOCK_GROUP_SIZE; k ++)
                blocks[k] = left + BLOCK_START + BLOCK_GROUP_SIZE * (j + 1) - k - 1; // 计算块号
            fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * j - 1), SEEK_SET);
            fwrite(&blocksNum,sizeof(blocksNum), 1, f);
            fwrite(&blocks, sizeof(blocks), 1, f);
        }
        // 不足一个盘块组的部分
        unsigned int blocksNum = 0;
        unsigned int blocks[BLOCK_GROUP_SIZE];
        fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * bGs - 1), SEEK_SET);
        fwrite(&blocksNum, sizeof(blocksNum), 1, f);
        fwrite(&blocks, sizeof(blocks), 1, f);
    }

    // 将初始化后的全局变量写入磁盘
    fseek(f, BLOCK_SIZE, SEEK_SET); // 定位到超级块的位置（一个BLOCK_SIZE之后）
    fwrite(sp, sizeof(SuperBlock), 1, f);
    fwrite(os, sizeof(Owners), 1, f);
    fwrite(gs, sizeof(Groups), 1, f);
    fseek(f, BLOCK_SIZE * root -> nodeId, SEEK_SET);
    fwrite(&root -> dINode, sizeof(DINode), 1, f);
    fclose(f);

    // 初始化目录
    auto* dt = new Direct();
    dt -> iNodeId = 2;
    strcpy(dt -> name, "root");
    ds.push_back(dt);
    superMkdir(root, "test1", 1, 1);
    superMkdir(root, "test2", 2, 2);

    cout << "初始化数据成功！" << endl;

    cout << "总共有" << os -> ownerNum << "个用户" << endl;
    for (int j = 0; j < os -> ownerNum; j ++)
        cout << os -> os[j].ownerName << endl;
}

void UnixFIleSys :: initSystem() {
    FILE *f = fopen(FILE_PATH,"rb");
    // 如果磁盘文件不存在就进行全局初始化
    if(f == nullptr) {
        cout << "系统暂无数据" << endl;
        initGlobal(f);
    } else {
    // 如果磁盘文件存在就读出全局变量、根目录信息，且将根目录压入路径栈
        fseek(f, BLOCK_SIZE, SEEK_SET);
        fread(sp, sizeof(SuperBlock), 1, f);
        fread(os, sizeof(Owners), 1, f);
        fread(gs, sizeof(Groups), 1, f);
        root -> nodeId = 2;
        root -> parent = nullptr;
        root -> users = 2;
        fseek(f, BLOCK_SIZE * root -> nodeId, SEEK_SET);
        fread(&root -> dINode, sizeof(DINode), 1, f);
        auto* dt = new Direct();
        dt -> iNodeId = 2;
        strcpy(dt -> name, "root");
        ds.push_back(dt); // 将根目录压入路径栈
        fclose(f);
        readCurDir();
    }
}


// 工具方法//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 读内存节点
// 通过INodeId找到INode的起始位置，读出
bool UnixFIleSys :: readINode(INode* r)	{
    FILE *f=fopen(FILE_PATH,"rb");
    if(f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * r -> nodeId, SEEK_SET);
        fread(&r -> dINode, sizeof(DINode), 1, f);
        fclose(f);
        return true;
    }
}

// 写内存节点
// 通过INodeId找到INode的起始位置，写入
bool UnixFIleSys :: writeINode(INode* w) {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * w -> nodeId, SEEK_SET);
        fwrite(&w -> dINode, sizeof(DINode), 1, f);
        fclose(f);
        return true;
    }
}

// 写超级块
// 定位到磁盘头开始，一个BLOCK_SIZE之后（SuperBlock的起始位置），写入
bool UnixFIleSys :: writeSuperBlock() {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE, SEEK_SET);
        fwrite(sp, sizeof(SuperBlock), 1, f);
        fclose(f);
        return true;
    }
}

// 写目录
// 通过blockId找到目录起始位置，写入
bool UnixFIleSys :: writeDir(unsigned int blockId, Dir* d) {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == NULL)
        return false;
    else {
        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
        fwrite(d, sizeof(Dir), 1, f);
        fclose(f);
        return true;
    }
}

// 读目录
// 通过blockId找到目录起始位置，读出
bool UnixFIleSys :: readDir(unsigned int blockId, Dir* d) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
        fread(d, sizeof(Dir), 1, f);
        fclose(f);
        return true;
    }
}

// 读下一组
// 通过当前组的栈顶块的块号找到下一组的起始位置（栈底），读出下一组的空闲栈信息和下一个空闲块信息
bool UnixFIleSys :: readNextBG() {
    FILE *f = fopen(FILE_PATH, "rb");
    if(f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE * sp -> freeBlock[0], SEEK_SET);
        fread(&sp -> nextFreeBlock, sizeof(sp -> nextFreeBlock), 1, f);
        fread(&sp -> freeBlock, sizeof(sp -> freeBlock), 1, f);
        sp -> freeBlockNum --;
        sp -> nextFreeBlock --;
        fclose(f);
        writeSuperBlock();
        return true;
    }
}

// 获取一个空闲块
// 当前组的信息存储在超级块中，操作完以后，写超级块
int UnixFIleSys :: getFreeBlock() {
    if(sp -> freeBlockNum > 0) {
        // 如果当前盘有空闲块就拿一个
        if (sp -> nextFreeBlock > 0) {
            sp -> freeBlockNum --;
            sp -> nextFreeBlock --;
            writeSuperBlock();
            return sp -> freeBlock[sp -> nextFreeBlock + 1];
        } else {
            // 如果当前盘已经没有空闲块了，就读下一个盘，再执行一次获取空闲块的方法
            readNextBG();
            return getFreeBlock();
        }
    } else
        return STATUS_NO_BLOCK;
}

// 获取一个空闲节点
// 当前关于节点的信息存储在超级块中，操作完以后，写超级块
int UnixFIleSys :: getFreeINode() {
    if (sp -> freeINodeNum > 0) {
        sp -> freeINodeNum --;
        sp -> nextFreeINode --;
        writeSuperBlock();
        return sp -> freeINode[sp -> nextFreeINode + 1];
    }
    else
        return STATUS_NO_INODE;
}

// 读当前目录
// 如果全局变量中的当前节点的文件大小信息为0则直接修改全局变量d，不然通过当前节点的地址数组的头位置，读出目录信息
int UnixFIleSys ::  readCurDir() {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == nullptr)
        return STATUS_FILE_OPEN_ERROR;
    else {
        if (curINode -> dINode.fileSize == 0)
            d -> dirNum = 0;
        else {
            fseek(f, BLOCK_SIZE * curINode -> dINode.addr[0], SEEK_SET);
            fread(d, sizeof(Dir), 1, f);
            fclose(f);
        }
        return STATUS_OK;
    }
}

// 写Owners
// 定位到一个块大小加上一个超级块大小之后（Owners的起始位置），写入
bool UnixFIleSys ::  writeOS() {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == NULL)
        return false;
    else {
        fseek(f, BLOCK_SIZE + sizeof(SuperBlock), SEEK_SET);
        fwrite(os, sizeof(Owners), 1, f);
        fclose(f);
        return true;
    }
}

// 写text
// 主要考虑原文件有没有块空空隙，追加字符串分块后有没有块空隙，有空隙先填空隙
int UnixFIleSys :: writeText(INode* temp, const string& text) {
    FILE *f = fopen(FILE_PATH,"rb+");
    if (f == nullptr)
        return STATUS_FILE_OPEN_ERROR;
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE; // 原来文件大小所占的整块数
        int ps = temp -> dINode.fileSize % BLOCK_SIZE; // 原来文件大小所占的多余字节数
        int bs = text.size() / BLOCK_SIZE; // 追加的字符串大小所占的整块数
        int ls = text.size() % BLOCK_SIZE; // 追加的字符串大小所占的多余字节数
        int pos = 0;

        // 计算如果写入后文件的总块数和最终多余字节数
        int totalBlockNum = 0;
        int totalLeft = 0;
        if (ls <= BLOCK_SIZE - ps) { // 要写入的部分的多余字节可以填满原空隙
            totalBlockNum = as + bs;
            totalLeft = ps + ls;
        } else { // 要写入的部分的多余字节填满原空隙还有多
            totalBlockNum = as + bs + 1;
            totalLeft = ps + ls - BLOCK_SIZE;
        }
        // 计算间址使用情况
        int status = 0; // status为0代表未使用间址，status为1代表使用了以一级间址，status为2代表使用了以二级间址
        if (totalBlockNum + (totalLeft > 0? 1: 0) <= 4) {
            status = 0;
        } else if (totalBlockNum + (totalLeft > 0? 1: 0) <= 4 + 128) {
            status = 1;
        } else if (totalBlockNum + (totalLeft > 0? 1: 0) <= 4 + 128 + 128 * 128) {
            status = 2;
        } else {
            return STATUS_BEYOND_SIZE;
        }
        // 计算原文件间址使用情况
        int status0 = 0;
        if (as + (ps > 0? 1: 0) <= 4) {
            status0 = 0;
        } else if (as + (ps > 0? 1: 0) <= 4 + 128) {
            status0 = 1;
        } else if (as + (ps > 0? 1: 0) <= 4 + 128 + 128 * 128) {
            status0 = 2;
        }

        // 如果原文件的块数不需要使用间址，就直接读每一个整块，追加到content
        if (status0 == 0 && status == 0) {
            // 原来文件恰好填满了一整块，就按照追加的字符串大小所占的整块数拿空闲块，并将每一个blockId加入原文件的地址数组
            if (ps == 0) {
                for (int i = 0; i < bs; i++) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        temp->dINode.addr[as++] = blockId;
                        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        temp->dINode.fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // 如果追加的字符串有多余整块的字节，就再拿一个空闲块，将blockId加入原文件的地址数组
                if (ls > 0) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        temp->dINode.addr[as++] = blockId;
                        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                        fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                        temp->dINode.fileSize += ls;
                        pos += ls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                fclose(f);
                return STATUS_OK;
            } else {
                // 原来文件不是恰好填满了一整块
                int lps = BLOCK_SIZE - ps; // 原文件的非满块的剩余空间
                // 如果追加字符串的大小小于原文件的非满块的剩余空间，直接写入，并修改文件大小属性
                if (text.size() <= lps) {
                    fseek(f, BLOCK_SIZE * temp->dINode.addr[as] + ps, SEEK_SET);
                    fwrite(text.c_str(), text.size(), 1, f);
                    temp->dINode.fileSize += text.size();
                    fclose(f);
                    return STATUS_OK;
                } else {
                    // 如果追加字符串的大小大于原文件的非满块的剩余空间，先写入追加字符串恰好填满非满块的剩余空间的部分
                    fseek(f, BLOCK_SIZE * temp->dINode.addr[as++] + ps, SEEK_SET);
                    fwrite(text.c_str(), lps, 1, f);
                    temp->dINode.fileSize += lps;
                    int lts = text.size() - lps;  // 追加字符剩余未写大小
                    int lbs = lts / BLOCK_SIZE;   // 追加字符剩余未写所占的整块数
                    int lls = lts % BLOCK_SIZE;   // 追加字符剩余未写所占的多余字节数
                    int tpos = lps;
                    // 按照追加字符剩余未写大小所占的整块数拿空闲块，并将每一个blockId加入原文件的地址数组
                    for (int i = 0; i < lbs; i++) {
                        int blockId = getFreeBlock();
                        if (blockId < 0) {
                            fclose(f);
                            return blockId;
                        } else {
                            temp->dINode.addr[as++] = blockId;
                            fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                            fwrite(text.substr(tpos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                            temp->dINode.fileSize += BLOCK_SIZE;
                            tpos += BLOCK_SIZE;
                        }
                    }
                    // 如果追加字符剩余未写大小有多余整块的字节，就再拿一个空闲块，将blockId加入原文件的地址数组
                    if (lls > 0) {
                        int blockId = getFreeBlock();
                        if (blockId < 0) {
                            fclose(f);
                            return blockId;
                        } else {
                            temp->dINode.addr[as++] = blockId;
                            fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                            fwrite(text.substr(tpos, lls).c_str(), BLOCK_SIZE, 1, f);
                            temp->dINode.fileSize += lls;
                            tpos += lls;
                        }
                        fclose(f);
                        return STATUS_OK;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
            }
        } else if (status0 == 0 && status == 1) {
        // 如果原文件没有使用间址，追加后需要使用一级间址
            // 原来文件恰好填满了一整块，就按照追加的字符串大小所占的整块数拿空闲块，并将每一个blockId加入原文件的地址数组
            if (ps == 0) {
                // 先使用完直接块号的部分
                int remainDirectBlockNum = 4 - as;
                for (int i = 0; i < remainDirectBlockNum; i++) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        (temp -> dINode).addr[as++] = blockId;
                        fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        temp->dINode.fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // 申请一个空闲block用作存一级栈
                int blockId = getFreeBlock();
                int remainBlockNum = totalBlockNum - 4;
                (temp -> dINode).addr[4] = blockId; // 写入地址数组的第5项（一级间址）
                vector<unsigned int> firstStackBlocks[remainBlockNum];
                // 为剩余每一个需要存进一级栈的（块），申请一个空闲块，并写入，同时调整pos
                for (int p = 0; p < remainBlockNum; p ++) {
                    int blockIdFir = getFreeBlock();
                    if (blockIdFir < 0) {
                        fclose(f);
                        return blockIdFir;
                    } else {
                        // 申请到空闲块后写入一级栈数组
                        firstStackBlocks -> push_back(blockIdFir);
                        // 写数据
                        fseek(f, BLOCK_SIZE * blockIdFir, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // 如果追加的字符串有多余整块的字节，就再拿一个空闲块，将blockId加入原文件的地址数组
                if (ls > 0) {
                    int blockIdLeft = getFreeBlock();
                    if (blockIdLeft < 0) {
                        fclose(f);
                        return blockIdLeft;
                    } else {
                        // 申请到空闲块后写入一级栈数组
                        firstStackBlocks -> push_back(blockIdLeft);
                        // 写数据
                        fseek(f, BLOCK_SIZE * blockIdLeft, SEEK_SET);
                        fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += ls;
                        pos += ls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                fclose(f);
                return STATUS_OK;
            } else {
                // 原来文件不是恰好填满了一整块
                int lps = BLOCK_SIZE - ps; // 原文件的非满块的剩余空间
                // 因为原文件未使用间址，写入后将使用间址，所以不存在追加字符串的大小小于原文件的非满块的剩余空间的情况
                // 先写入追加字符串恰好填满非满块的剩余空间的部分
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[as ++] + ps, SEEK_SET);
                fwrite(text.substr(pos, ls).c_str(), lps, 1, f);
                temp -> dINode.fileSize += lps;
                pos += lps;

                int lts = text.size() - lps;  // 追加字符剩余未写大小
                int lbs = lts / BLOCK_SIZE;   // 追加字符剩余未写所占的整块数
                int lls = lts % BLOCK_SIZE;   // 追加字符剩余未写所占的多余字节数
                int tpos = lps;
                // 申请一个空闲块存储一级栈
                int blockId = getFreeBlock();
                int remainBlockNum = totalBlockNum - 4;
                (temp -> dINode).addr[4] = blockId; // 写入地址数组的第5项（一级间址）
                vector<unsigned int> firstStackBlocks;
                // 为剩余每一个需要存进一级栈的（块），申请一个空闲块，并写入，同时调整pos
                for (int p = 0; p < remainBlockNum; p ++) {
                    int blockIdFir = getFreeBlock();
                    if (blockIdFir < 0) {
                        fclose(f);
                        return blockIdFir;
                    } else {
                        // 申请到空闲块后写入一级栈数组
                        firstStackBlocks.push_back(blockIdFir);
                        // 写数据
                        fseek(f, BLOCK_SIZE * blockIdFir, SEEK_SET);
                        fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += BLOCK_SIZE;
                        pos += BLOCK_SIZE;
                    }
                }
                // 如果追加的字符串有多余整块的字节，就再拿一个空闲块，将blockId加入原文件的地址数组
                if (totalLeft > 0) {
                    int blockIdLeft = getFreeBlock();
                    if (blockIdLeft < 0) {
                        fclose(f);
                        return blockIdLeft;
                    } else {
                        // 申请到空闲块后写入一级栈数组
                        firstStackBlocks.push_back(blockIdLeft);
                        // 写数据
                        fseek(f, BLOCK_SIZE * blockIdLeft, SEEK_SET);
                        fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                        (temp -> dINode).fileSize += ls;
                        pos += ls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                // 一级栈写回磁盘
                fseek(f, BLOCK_SIZE * blockId, SEEK_SET);
                fwrite(firstStackBlocks.data(),sizeof(decltype(firstStackBlocks)::value_type)* firstStackBlocks.size(), 1, f);
                // fwrite(firstStackBlocks, sizeof(firstStackBlocks), 1, f);

                fclose(f);
                return STATUS_OK;
            }
        } else if (status0 == 0 && status == 2) {

        } else if (status0 == 1 && status == 0) {

        } else if (status0 == 1 && status == 1) {

        } else if (status0 == 1 && status == 2) {

        } else if (status0 == 2 && status == 0) {

        } else if (status0 == 2 && status == 1) {

        } else if (status0 == 2 && status == 2) {

        }
    }
}

// 读text
// 主要考虑是否有块外字节，根据传入的INode信息中的地址数组找到存储块，读出每一块的内容
int UnixFIleSys :: readText(INode *temp) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return STATUS_FILE_OPEN_ERROR;
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;  // 文件大小所占的整块数
        int ls = temp -> dINode.fileSize % BLOCK_SIZE;  // 文件大小所占的多余字节数
        char content[BLOCK_SIZE];
        // 如果原文件的块数不需要使用间址，就直接读每一个整块，输出
        if (as <= 4) {
            int i = 0;
            for (; i < as; i ++) {
                memset(content, '\0', sizeof(content));
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // 如果文件有块外字节
            if (ls > 0) {
                memset(content, '\0', sizeof(content));
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    cout << content[p];
            }
        } else if(as >= 5 && as <= 132) {
            // 如果只用到了一级间址,先读直接地址的每一个整块，追加到content,再去一级栈读每一块的内容，输出
            // 先读直接地址的每一个整块，输出
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // 读一级栈所在的块号，并定位到一级栈，读出其中的直接块号数组
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            int blockNumInFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4; // 通过文件大小计算在一级间址中使用了几个block
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            int i = 0;
            for (; i < blockNumInFirstStack; i ++) {
                // 读出一级栈中的直接块号并读出其中内容，输出
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // 如果文件有块外字节
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    cout << content[p];
            }
        } else if (as > 132) {
            // 如果用到了二级间址
            // 先读直接地址的每一个整块，输出
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // 读一级栈所在的块号，并定位到一级栈，读出其中的二级栈所在的块的块号数组
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            // 对于一级栈元素位置非最尾（其对应的二级栈不可能出现空隙）
            for (int i = 0; i < 128 - 1; i ++) {
                // 读出一级栈中的二级栈所在的块的块号，并定位到对应二级栈，读出二级栈中的直接块号数组
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                unsigned int blocksInSecStack[128];
                fread(blocksInSecStack, BLOCK_SIZE, 1, f);
                for (int j = 0; j < 128; j ++) {
                    // 读出二级栈中的直接块号并读出其中内容，输出
                    fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                    fread(content, BLOCK_SIZE, 1, f);
                    cout << content;
                }
            }
            // 对于一级栈元素位置最尾（其对应的二级栈可能出现空隙）
            fseek(f, BLOCK_SIZE * blocksInFirstStack[128], SEEK_SET);
            unsigned int blocksInSecStack[128];
            fread(blocksInSecStack, BLOCK_SIZE, 1, f);
            int blockNumInLastFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4 - 127 * 128; // 计算一级栈元素位置最尾对应的二级栈使用的blockNum
            int j = 0;
            for (; j < blockNumInLastFirstStack; j ++) {
                // 读出二级栈中的直接块号并读出其中内容，输出
                fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                cout << content;
            }
            // 如果文件有块外字节
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[j], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    cout << content[p];
            }
        }
        fclose(f);
    }
    return STATUS_OK;

}

// 归还空闲块
// 根据blockId做归还操作，修改超级块的信息，修改后写超级块
int UnixFIleSys :: returnFreeBlock(unsigned int blockId) {
    // 如果当前组还有空，就归还在当前组
    if (sp -> nextFreeBlock <= 18) {
        sp -> freeBlockNum ++;
        sp -> nextFreeBlock ++;
        sp -> freeBlock[sp -> nextFreeBlock] = blockId;
        writeSuperBlock();
        return STATUS_OK;
    } else {
    // 如果当前组无空，取出下一组，归还到下一组
        FILE *f = fopen(FILE_PATH, "rb+");
        if (f == NULL)
            return STATUS_FILE_OPEN_ERROR;
        else {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            fseek(f, BLOCK_SIZE * (sp -> freeBlock[0] - BLOCK_GROUP_SIZE * 2), SEEK_SET);
            fwrite(&blocksNum, sizeof(unsigned int), 1, f);
            fwrite(&sp -> freeBlock, sizeof(sp -> freeBlock), 1, f);
            fclose(f);
            sp -> freeBlockNum += 2;
            sp -> freeBlock[0] = sp -> freeBlock[0] - BLOCK_GROUP_SIZE * 2; // 存储上一个空闲盘的首位盘块号
            sp -> freeBlock[1] = blockId;
            sp -> nextFreeBlock = 1;
            writeSuperBlock();
            return STATUS_OK;
        }
    }
}

// 归还INode
// 修改超级块信息，然后写超级块
int UnixFIleSys :: returnFreeINode(unsigned int iNodeId) {
    sp -> freeINodeNum ++;
    sp -> nextFreeINode ++;
    sp -> freeINode[sp -> nextFreeINode] = iNodeId; // 写入这个归还了的INodeId
    writeSuperBlock();
    return STATUS_OK;
}

// 读text
// 通过INodeId读取文件大小信息和地址信息，先读每一个整块，追加到content，如果文件有块外字节也读出追加
string UnixFIleSys :: getText(INode* temp) {
    string text;
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return "";
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;  // 文件大小所占的整块数
        int ls = temp -> dINode.fileSize % BLOCK_SIZE;  // 文件大小所占的多余字节数
        char content[BLOCK_SIZE];
        // 如果原文件的块数不需要使用间址，就直接读每一个整块，追加到content
        if (as <= 4) {
            int i = 0;
            for (; i < as; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // 如果文件有块外字节
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    text += content[p];
            }
        } else if(as == 5) {
        // 如果只用到了一级间址,先读直接地址的每一个整块，追加到content,再去一级栈读每一块的内容，追加到content
            // 先读直接地址的每一个整块，追加到content
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // 读一级栈所在的块号，并定位到一级栈，读出其中的直接块号数组
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            int blockNumInFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4; // 通过文件大小计算在一级间址中使用了几个block
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            int i = 0;
            for (; i < blockNumInFirstStack; i ++) {
                // 读出一级栈中的直接块号并读出其中内容，追加到content
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // 如果文件有块外字节
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    text += content[p];
            }
        } else if (as == 6) {
        // 如果用到了二级间址
            // 先读直接地址的每一个整块，追加到content
            for (int i = 0; i < 4; i ++) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[i], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // 读一级栈所在的块号，并定位到一级栈，读出其中的二级栈所在的块的块号数组
            int mid1BlockId = temp -> dINode.addr[4];
            fseek(f, BLOCK_SIZE * mid1BlockId, SEEK_SET);
            unsigned int blocksInFirstStack[128];
            fread(blocksInFirstStack, BLOCK_SIZE, 1, f);
            // 对于一级栈元素位置非最尾（其对应的二级栈不可能出现空隙）
            for (int i = 0; i < 128 - 1; i ++) {
                // 读出一级栈中的二级栈所在的块的块号，并定位到对应二级栈，读出二级栈中的直接块号数组
                fseek(f, BLOCK_SIZE * blocksInFirstStack[i], SEEK_SET);
                unsigned int blocksInSecStack[128];
                fread(blocksInSecStack, BLOCK_SIZE, 1, f);
                for (int j = 0; j < 128; j ++) {
                    // 读出二级栈中的直接块号并读出其中内容，追加到content
                    fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                    fread(content, BLOCK_SIZE, 1, f);
                    text += content;
                }
            }
            // 对于一级栈元素位置最尾（其对应的二级栈可能出现空隙）
            fseek(f, BLOCK_SIZE * blocksInFirstStack[128], SEEK_SET);
            unsigned int blocksInSecStack[128];
            fread(blocksInSecStack, BLOCK_SIZE, 1, f);
            int blockNumInLastFirstStack = (temp -> dINode.fileSize) / BLOCK_SIZE - 4 - 127 * 128; // 计算一级栈元素位置最尾对应的二级栈使用的blockNum
            int j = 0;
            for (; j < blockNumInLastFirstStack; j ++) {
                // 读出二级栈中的直接块号并读出其中内容，追加到content
                fseek(f, BLOCK_SIZE * blocksInSecStack[j], SEEK_SET);
                fread(content, BLOCK_SIZE, 1, f);
                text += content;
            }
            // 如果文件有块外字节
            if (ls > 0) {
                fseek(f, BLOCK_SIZE * temp -> dINode.addr[j], SEEK_SET);
                fread(content, ls, 1, f);
                for (int p = 0; p < ls; p ++)
                    text += content[p];
            }
        }
        fclose(f);
        return text;
    }
}

// 整型
// 去掉头尾空格
string UnixFIleSys :: trim(string s) {
    if (s.empty())
        return s;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}


// 业务方法//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 展示当前目录
// 依次读路径栈的内容，组合成字符串
string UnixFIleSys :: pwd() {
    string path;
    for (int i = 0; i < ds.size(); i ++) {
        path += ds[i] -> name;
        path += "/";
    }
    return path;
}

// 检验文件名
// 在当前目录下，检查是否有同名目录
int UnixFIleSys :: checkFileName(char name[MAX_NAME_SIZE]) {
    int flag = 0;
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL) {
        cout << "f不存在" << endl;
        flag = 1;
        return flag;
    } else {
        //cout << "f存在" << endl;
        Dir d;
        // d.dirNum = 0;
        fseek(f, BLOCK_SIZE * curINode -> dINode.addr[0], SEEK_SET);
        fread(&d, sizeof(Dir), 1, f);
        fclose(f);
        //cout << "现在所在的目录的目录项数为" << d.dirNum << endl;
        for (int i = 0; i < d.dirNum; i ++)
            if(strcmp(d.direct[i].name, name) == 0)
                flag = 1;
        return flag;
    }
}

// 创建文件夹
// 本质是创建节点（大小是Dir的大小）和目录项
int UnixFIleSys :: mkdir(INode* parent, char name[MAX_NAME_SIZE]) {
    // cout << "您要创建的目录名是" + name << endl;
    bool exist = checkFileName(name);
    // 原父节点为空
    if (parent -> dINode.fileSize == 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int blockId = getFreeBlock();
        if(blockId < 0)
            return blockId;
        else {
            int iNodeId = getFreeINode();
            if (iNodeId < 0)
                return iNodeId;
            else {
                parent -> dINode.fileSize = sizeof(Dir);
                time_t now;
                now = time(nullptr);
                parent -> dINode.modifyTime = now;
                parent -> dINode.addr[0] = blockId;
                writeINode(parent);
                Direct *dt = new Direct();
                strcpy(dt -> name, name);
                dt -> iNodeId = iNodeId;
                DINode *di = new DINode();
                di -> createTime = now;
                di -> fileSize = 0;
                di -> groupId = parent -> dINode.groupId;
                di -> linkNum = 0;
                di -> mod = 14;
                di -> modifyTime = now;
                di -> ownerId = parent -> dINode.ownerId;
                di -> readTime = now;
                INode i;
                i.dINode = *di;
                i.nodeId = iNodeId;
                i.parent = parent;
                i.users = parent -> users;
                writeINode(&i);
                Dir	dd;
                dd.dirNum = 1;
                dd.direct[0] = *dt;
                writeDir(blockId, &dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    // 原父节点不为空
    } else if (parent -> dINode.fileSize != 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(NULL);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            strcpy(dt -> name, name);
            dt -> iNodeId = iNodeId;
            DINode *di = new DINode();
            di -> createTime = now;
            di -> fileSize = 0;
            di -> groupId = parent -> dINode.groupId;
            di -> linkNum = 0;
            di -> mod = 14;
            di -> modifyTime = now;
            di -> ownerId = parent -> dINode.ownerId;
            di -> readTime = now;
            INode i;
            i.dINode = *di;
            i.nodeId = iNodeId;
            i.parent = parent;
            i.users = parent -> users;
            writeINode(&i);
            Dir dd;
            readDir(parent -> dINode.addr[0], &dd); // 读出父节点的目录结构
            if (dd.dirNum < MAX_DIRECT_NUM) { // 如果目录还能追加就追加
                dd.direct[dd.dirNum] = *dt;
                dd.dirNum ++;
                writeDir(parent -> dINode.addr[0], &dd);
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            } else
                return STATUS_BEYOND_DIRECT_NUM;
        }
    } else if (parent -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
        return STATUS_BEYOND_RIGHT;
    else if(parent -> dINode.mod == 12)
        return STATUS_READ_ONLY;
    else if(exist)
        return STATUS_FILENAME_EXIST;
    return STATUS_ERROR;
}

// 用super权限创建目录
// 本质是创建节点（大小是Dir的大小）和目录项
int UnixFIleSys :: superMkdir(INode* parent, char name[MAX_NAME_SIZE], unsigned short ownerId, unsigned short groupId) {
    int exist = checkFileName(name);
    if (parent -> dINode.fileSize == 0) {
        int blockId = getFreeBlock();
        if (blockId < 0)
            return blockId;
        else {
            int iNodeId = getFreeINode();
            if (iNodeId < 0)
                return iNodeId;
            else {
                parent -> dINode.fileSize = sizeof(Dir);
                time_t now;
                now = time(NULL);
                parent -> dINode.modifyTime = now;
                parent -> dINode.addr[0] = blockId;
                writeINode(parent);
                Direct *dt = new Direct();
                strcpy(dt -> name, name);
                dt -> iNodeId = iNodeId;
                DINode *di = new DINode();
                di -> createTime = now;
                di -> fileSize = 0;
                di -> groupId = groupId;
                di -> linkNum = 0;
                di -> mod = 14;
                di -> modifyTime = now;
                di -> ownerId = ownerId;
                di -> readTime = now;
                INode i;
                i.dINode = *di;
                i.nodeId = iNodeId;
                i.parent = parent;
                i.users = 0;
                writeINode(&i);
                Dir	dd;
                dd.dirNum = 1;
                dd.direct[0] = *dt;
                writeDir(blockId, &dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    }
    else if (parent -> dINode.fileSize != 0) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(NULL);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            strcpy(dt -> name, name);
            dt -> iNodeId = iNodeId;
            DINode *di = new DINode();
            di -> createTime = now;
            di -> fileSize = 0;
            di -> groupId = groupId;
            di -> linkNum = 0;
            di -> mod = 14;
            di -> modifyTime = now;
            di -> ownerId = ownerId;
            di -> readTime = now;
            INode i;
            i.dINode = *di;
            i.nodeId = iNodeId;
            i.parent = parent;
            i.users = 0;
            writeINode(&i);
            Dir dd;
            readDir(parent -> dINode.addr[0], &dd);
            if (dd.dirNum < MAX_DIRECT_NUM) {
                dd.direct[dd.dirNum] = *dt;
                dd.dirNum ++;
                writeDir(parent -> dINode.addr[0], &dd);
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
            else
                return STATUS_BEYOND_DIRECT_NUM;
        }
    }
    return STATUS_ERROR;
}

// 登录
// 在全局变量Owners中查找对应用户名和密码的用户
int UnixFIleSys :: login() {
    cout << "please input your userName:";
    string userName;
    cin >> userName;
    cout << "please input your password:";
    char password[MAX_NAME_SIZE]={0};
    int i=0;
    while (i < MAX_NAME_SIZE) {
        password[i] = getchar();
        if (password[i] == '\n' && i == 0) i = -1;
        if (i != -1 && password[i] == '\n') {
            cout << "检测到换行" << endl;
            for (int j = i; j <= 13; j ++) {
                password[j]='\0';
            }
            break;
        }
        if (password[i] == 13) {
            password[i]='\0';
            break;
        }
        i++;
    }
    cout<<endl;
    char un[MAX_NAME_SIZE];
    strcpy(un, userName.c_str());
    for (int j = 0; j < os -> ownerNum; j ++) {
        int flag = strcmp(un, os->os[j].ownerName);
        int flag2 = strcmp(password, os->os[j].ownerPassword);
        if (strcmp(un, os->os[j].ownerName) == 0 && strcmp(password, os->os[j].ownerPassword) == 0) {
            curOwner = &os->os[j];
            return STATUS_OK;
        }
    }
    cout << "UserName or Password Wrong!!!" << endl;
    return login();
}

// 指令分发
void UnixFIleSys :: commandDispatcher() {
    cout << endl;
    cout << curOwner -> ownerName << "@UnixFileSystem:~" << pwd() << "$";
    char c[100];
    cin.getline(c,100,'\n');
    while(c[0] == NULL)
        cin.getline(c,100,'\n');
    string command = c;
    command = trim(command);
    int flag = -1;
    string target = " -h";
    int result = command.find(target);
    if (result != -1) {
        flag = 18;
        goto aa;
    }
    int subPos;
    subPos = command.find_first_of(" ");
    if (subPos == -1) {
        if (command == "ls") {
            flag = 1;
        } else if (command == "pwd")
            flag=5;
        else if (command == "passwd")
            flag = 14;
        else if (command == "sp")
            flag = 17;
        else if (command == "help")
            flag = 19;
    } else {
        string c = command.substr(0, subPos);
        if (c == "chmod")
            flag = 2;
        else if (c == "chown")
            flag = 3;
        else if (c == "chgrp")
            flag = 4;
        else if (c == "cd")
            flag = 6;
        else if (c == "mkdir")
            flag = 7;
        else if (c == "rmdir")
            flag = 8;
        else if (c == "mv")
            flag = 9;
        else if (c == "cp")
            flag = 10;
        else if (c == "rm")
            flag = 11;
        else if (c == "ln")
            flag = 12;
        else if (c == "cat")
            flag = 13;
        else if (c == "touch")
            flag = 15;
        else if (c == ">>")
            flag = 16;
    }
    aa: switch(flag) {
        // ls
        case 1: {
            cout << ls() << endl;
            break;
        };
        // chmod (chmod 权限数 文件名/目录名)
        case 2: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string mod = pattern.substr(0, subPos2);
                string fileName = pattern.substr(subPos2 + 1);
                fileName = trim(fileName);
                int subPos3 = fileName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    unsigned short m = atoi(mod.c_str());
                    char name[MAX_NAME_SIZE];
                    strcpy(name,fileName.c_str());
                    int result = chmod(name, m);
                    if (result == STATUS_MOD_ERROR)
                        cout << "Error: Pattern is illegal..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                }
            }
            break;
        };
        // chown
        case 3: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string ownerId = pattern.substr(0, subPos2);
                string fileName = pattern.substr(subPos2 + 1);
                fileName = trim(fileName);
                int subPos3 = fileName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    unsigned short oi = atoi(ownerId.c_str());
                    char name[MAX_NAME_SIZE];
                    strcpy(name, fileName.c_str());
                    int result = chown(name, oi);
                    if (result == STATUS_OWNER_NONEXIST)
                        cout << "Error: OwnerId is nonexisted..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                }
            }
            break;
        };
        // chgrp
        case 4: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string groupId = pattern.substr(0, subPos2);
                string fileName = pattern.substr(subPos2 + 1);
                fileName = trim(fileName);
                int subPos3 = fileName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    unsigned short gi = atoi(groupId.c_str());
                    char name[MAX_NAME_SIZE];
                    strcpy(name,fileName.c_str());
                    int result = chgrp(name, gi);
                    if (result == STATUS_GROUP_NONEXIST)
                        cout << "Error: GroupId is nonexisted..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                }
            }
            break;
        };
        // pwd
        case 5: {
            cout << pwd() << endl;
            break;
        };
        // cd
        case 6:  {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error DirectName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = cd(name);
                if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: Dir is not existed..." << endl;
                else if (result == STATUS_NOT_DIRECT)
                    cout << "Error: Not a direct..." << endl;
            }
            break;
        };
        // mkdir
        case 7: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = mkdir(curINode, name);
                if(result == STATUS_NO_BLOCK)
                    cout << "Error: No Free Block..." << endl;
                else if (result == STATUS_NO_INODE)
                    cout << "Error: No Free INode..." << endl;
                else if (result == STATUS_BEYOND_DIRECT_NUM)
                    cout << "Error: Beyond Max Direct Num..." << endl;
                else if (result == STATUS_READ_ONLY)
                    cout << "Error: The Dir can not be written..." << endl;
                else if (result == STATUS_ERROR)
                    cout << "Error: Unexpected..." << endl;
                else if (result == STATUS_FILENAME_EXIST)
                    cout << "Error: FileName has existed..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond Your Right..." << endl;
            }
            break;
        };
        // rmdir
        case 8: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = rmdir(name);
                if (result == STATUS_NOT_DIRECT)
                    cout << "Error: Not a direct..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Byond your right..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
            }
            break;
        };
        // mv
        case 9: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string oldName = pattern.substr(0, subPos2);
                string newName = pattern.substr(subPos2 + 1);
                newName = trim(newName);
                int subPos3 = newName.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    char oldName1[MAX_NAME_SIZE];
                    strcpy(oldName1, oldName.c_str());
                    char newName1[MAX_NAME_SIZE];
                    strcpy(newName1, newName.c_str());
                    int result = mv(oldName1, newName1);
                    if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName is nonexisted..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond Your Right..." << endl;
                    else if (result == STATUS_FILENAME_EXIST)
                        cout << "Error: NewName has existed..." << endl;
                }
            }
            break;
        };
        // cp
        case 10: {
            string pattern = command.substr(subPos+1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string source = pattern.substr(0, subPos2);
                string des = pattern.substr(subPos2 + 1);
                des = trim(des);
                int subPos3 = des.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    char s[MAX_NAME_SIZE];
                    strcpy(s, source.c_str());
                    char d[MAX_NAME_SIZE];
                    strcpy(d, des.c_str());
                    int result = cp(s, d);
                    if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName doesn't exist..." << endl;
                    else if (result == STATUS_SDNAME_OVERLAP)
                        cout << "Error: SourceName overlap desName..." << endl;
                    else if (result == STATUS_NOT_FILE)
                        cout << "Error: Not a file..." << endl;
                    else if (result == STATUS_READ_ONLY)
                        cout << "Error: Read only..." << endl;
                    else if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond your right..." << endl;
                    else if (result == STATUS_NO_BLOCK)
                        cout << "Error: No free block..." << endl;
                    else if (result == STATUS_NO_INODE)
                        cout << "Error: No free iNode..." << endl;
                }
            }
            break;
        };
        // rm
        case 11: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = rm(name);
                if (result == STATUS_NOT_FILE)
                    cout << "Error: Not a file..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond your right..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
            }
            break;
        };
        case 12: {
            string pattern = command.substr(subPos + 1);
            pattern = trim(pattern);
            int subPos2 = pattern.find_first_of(" ");
            if (subPos2 == -1)
                cout << "Error Pattern..." << endl;
            else {
                string source = pattern.substr(0, subPos2);
                string des = pattern.substr(subPos2 + 1);
                des = trim(des);
                int subPos3 = des.find_first_of(" ");
                if (subPos3 != -1)
                    cout << "Error pattern..." << endl;
                else {
                    char s[MAX_NAME_SIZE];
                    strcpy(s, source.c_str());
                    char d[MAX_NAME_SIZE];
                    strcpy(d, des.c_str());
                    int result = ln(s, d);
                    if (result == STATUS_BEYOND_RIGHT)
                        cout << "Error: Beyond your right..." << endl;
                    else if (result == STATUS_FILENAME_NONEXIST)
                        cout << "Error: FileName doesn't exist..." << endl;
                    else if (result == STATUS_TYPE_NOT_MATCH)
                        cout << "Error: Source Type doesn't match des type..." << endl;
                    else if (result == STATUS_SDNAME_OVERLAP)
                        cout << "Error: SourceName overlap desName..." << endl;
                }
            };
            break;
        }
        // cat
        case 13: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = cat(name);
                if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond your right..." << endl;
                else if (result == STATUS_NOT_FILE)
                    cout << "Error: Not a file..." << endl;
                else if (result == STATUS_WRITE_ONLY)
                    cout << "Error: Write only..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
                else if (result == STATUS_FILE_OPEN_ERROR)
                    cout << "Error: File open error..." << endl;
            }
            break;
        }
        // password
        case 14: {
            int result = passwd();
            if (result == STATUS_PASSWORD_WRONG)
                cout << "Error: Password is wrong..." << endl;
            else if (result == STATUS_CONFIRM_WRONG)
                cout << "Error: Confirm password is wrong..." << endl;
            else if (result == STATUS_ERROR)
                cout << "Error: Unexpected error..." << endl;
            break;
        };
        // touch
        case 15: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name,fileName.c_str());
                int result = touch(curINode, name);
                if (result == STATUS_NO_BLOCK)
                    cout << "Error: No Free Block..." << endl;
                else if (result == STATUS_NO_INODE)
                    cout << "Error: No Free INode..." << endl;
                else if (result == STATUS_BEYOND_DIRECT_NUM)
                    cout << "Error: Beyond Max Direct Num..." << endl;
                else if (result == STATUS_READ_ONLY)
                    cout << "Error: The Dir can not be written..." << endl;
                else if (result == STATUS_ERROR)
                    cout << "Error: Unexpected..." << endl;
                else if (result == STATUS_FILENAME_EXIST)
                    cout << "Error: FileName has existed..." << endl;
                else if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Beyond Your Right..." << endl;
            }
            break;
        };
        // >>
        case 16: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                char name[MAX_NAME_SIZE];
                strcpy(name, fileName.c_str());
                int result = textAppend(name);
                if (result == STATUS_BEYOND_RIGHT)
                    cout << "Error: Byond your right..." << endl;
                else if (result == STATUS_NOT_FILE)
                    cout << "Error: Not a file..." << endl;
                else if (result == STATUS_READ_ONLY)
                    cout << "Error: The file can not be written..." << endl;
                else if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: FileName doesn't exist..." << endl;
                else if (result == STATUS_BEYOND_SIZE)
                    cout << "Error: Beyond file's max size..." << endl;
                else if (result == STATUS_FILE_OPEN_ERROR)
                    cout << "Error: File open error..." << endl;
            }
            break;
        };
        // sp
        case 17: {
            cout << "FreeBlockNum:" << sp -> freeBlockNum << endl;
            cout << "NextFreeBlock:" << sp -> nextFreeBlock << endl;
            cout << "FreeBlock:" << endl;
            for (int i = 0; i <= sp -> nextFreeBlock; i ++)
                cout << "[" << i << "]:" << sp -> freeBlock[i] << endl;
            cout << "FreeINodeNum:" << sp -> freeINodeNum << endl;
            cout << "NextFreeINode:" << sp -> nextFreeINode << endl;
            cout << "FreeINode:" << endl;
            for (int j = 0; j <= sp -> nextFreeINode; j ++)
                cout << "[" << j << "]:" << sp -> freeINode[j] << endl;
            break;
        };
        // -h
        case 18: { // 提示指定命令用法
            int subPos = command.find_first_of(" ");
            string c = command.substr(0, subPos);
            help(c);
            break;
        }
        // help
        case 19: { // 提示所有命令用法
            displayCommands();
            break;
        }
        default: {
            cout << "Error Command..." << endl;
            break;
        };
    }
    commandDispatcher();
}

void UnixFIleSys :: help(string c) {
    if (c == "ls") {
        cout << "举例： ls" << endl;
        cout << "上例可以显示当前目录下的全部文件或目录" << endl;
    } else if (c == "chmod") {
        cout << "举例： chmod 1 zjut.txt" << endl;
        cout << "上例可以修改当前目录下的zjut.txt文件的权限为1（仅执行）" << endl;
        cout << "dir： 8" << endl;
        cout << "r  ： 4" << endl;
        cout << "w  ： 2" << endl;
        cout << "x  ： 1" << endl;
    } else if (c == "chown") {
        cout << "举例： chowner 2 zjut.txt" << endl;
        cout << "上例可以修改当前目录下的zjut.txt文件的权限为ownerId为2的用户" << endl;
    } else if (c == "chgrp") {
        cout << "举例： chgrp 2 zjut.txt" << endl;
        cout << "上例可以修改当前目录下的zjut.txt文件的为groupId为2的组" << endl;
    } else if (c == "pwd") {
        cout << "举例： pwd" << endl;
        cout << "上例可以展示当前路径" << endl;
    } else if (c == "cd") {
        cout << "举例： cd zjut" << endl;
        cout << "上例可以进入当前目录下的名为zjut的子目录" << endl;
    } else if (c == "mkdir") {
        cout << "举例： mkdir zjut" << endl;
        cout << "上例可以在当前目录下创建zjut目录，当且仅当当前用户拥有是父目录的拥有者或是管理员" << endl;
    } else if (c == "rmdir") {
        cout << "举例： rmdir zjut" << endl;
        cout << "上例可以删除当前目录下创建zjut目录，并且删除zjut目录下的全部文件和目录" << endl;
    } else if (c == "mv") {
        cout << "举例： mv zjut zjut1" << endl;
        cout << "上例可以修改当前目录下zjut目录名为zjut1" << endl;
    } else if (c == "cp") {
        cout << "举例： cp zjut.txt zjut1.txt" << endl;
        cout << "上例可以复制当前目录下zjut.txt文件，在当前目录下生成一个和zjut.txt内容相同的zjut1.txt文件" << endl;
    } else if (c == "rm") {
        cout << "举例： rm zjut.txt" << endl;
        cout << "上例可以删除当前目录下的zjut.txt，如果存在软链接就减去一个软链接并且删去目录项" << endl;
    } else if (c == "ln") {
        cout << "举例： ln zjut.txt zjut1.txt" << endl;
        cout << "上例可以为zjut.txt创建一个软链接到zjut1.txt" << endl;
    } else if (c == "cat") {
        cout << "举例： cat zjut.txt" << endl;
        cout << "上例可以展示zjut.txt文件的内容" << endl;
    } else if (c == "passwd") {
        cout << "举例： passwd" << endl;
        cout << "上例可以根据后续系统提示修改密码" << endl;
    } else if (c == "touch") {
        cout << "举例： touch zjut.txt" << endl;
        cout << "上例可以在当前目录下创建一个名为zjut.txt的文件" << endl;
    } else if (c == ">>") {
        cout << "举例： >> zjut.txt" << endl;
        cout << "上例可以在当前目录下为一个名为zjut.txt的文件追加内容。以：wq换行结束。" << endl;
    } else if (c == "sp") {
        cout << "举例： sp" << endl;
        cout << "上例可以展示此时超级块的信息" << endl;
    } else if (c == "-h") {
        cout << "举例： ls -h" << endl;
        cout << "上例可以展示命令ls的使用方法" << endl;
    } else if (c == "help") {
        cout << "举例： help" << endl;
        cout << "上例可以展示全部命令的使用方法" << endl;
    }
    else
        cout<<"Error Command..."<<endl;
}

// 展示指令
void UnixFIleSys :: displayCommands() {
    cout<<"ls		显示目录文件"<<endl;
    cout<<"chmod		改变文件权限"<<endl;
    cout<<"chown		改变文件拥有者"<<endl;
    cout<<"chgrp		改变文件所属组"<<endl;
    cout<<"pwd		显示当前目录"<<endl;
    cout<<"cd		改变当前目录"<<endl;
    cout<<"mkdir		创建子目录"<<endl;
    cout<<"rmdir		删除子目录"<<endl;
    cout<<"mv		改变文件名"<<endl;
    cout<<"cp		文件拷贝"<<endl;
    cout<<"rm		文件删除"<<endl;
    cout<<"ln		建立文件联接"<<endl;
    cout<<"cat		连接显示文件内容"<<endl;
    cout<<"passwd		修改用户口令"<<endl;
    cout<<"touch		创建文件"<<endl;
    cout<<">>		文本内容追加"<<endl;
}

// 展示当前目录下的全部目录和文件
// 遍历全局变量d（当前目录），打印每一个目录项的名字
string UnixFIleSys :: ls() {
    string ls;
    for (int i = 0; i < d -> dirNum; i++) {
        ls += d -> direct[i].name;
        ls += " ";
    }
    return ls;
}

// 进入目录
// 本质是切换curINode
int UnixFIleSys :: cd(char name[MAX_NAME_SIZE]) {
    Direct *dt = new Direct();
    // 若是要进入当前目录，则忽视这个操作
    if (strcmp(name, "./") == 0) {
        delete dt;
        return STATUS_OK;
    } else if (strcmp(name, "../") == 0) {
    // 若是要进入上级操作，将全局变量curINode换成父节点，同时路径栈弹出
        if (curINode -> parent != nullptr) {
            INode *temp = curINode;
            curINode = curINode -> parent;
            delete temp;
            Direct* t = ds[ds.size() - 1];
            ds.pop_back();
            delete t;
            readCurDir();
            return STATUS_OK;
        }
        delete dt;
        return STATUS_OK;
    }
    else {
    // 进入目录：通过name找到NodeId，读出INode，写到curINode
        bool dtExist = false;
        for (int i = 0; i < d -> dirNum; i ++)
            if (strcmp(d -> direct[i].name, name) == 0) {
                *dt = d -> direct[i];
                dtExist = true;
            }
        if (!dtExist) {
            delete dt;
            return STATUS_FILENAME_NONEXIST;
        } else {
            INode *now = new INode();
            now -> nodeId = dt -> iNodeId;
            now -> parent = curINode;
            now -> users = curINode -> users;
            readINode(now);
            if (now -> dINode.mod > 7) {
                curINode = now;
                ds.push_back(dt);
                readCurDir();
                return STATUS_OK;
            } else {
                delete dt;
                delete now;
                return STATUS_NOT_DIRECT;
            }
        }
    }
}

// 更改权限
// 本质是修改dINode的mod属性
int UnixFIleSys :: chmod(char name[MAX_NAME_SIZE], unsigned int mod)	{
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            if (mod == 8 || mod == 9 || mod == 11 || mod == 13 || mod > 14)
                return STATUS_MOD_ERROR;
            else {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (mod < 8 && temp -> dINode.mod < 8 && (temp -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
                    temp -> dINode.mod = mod;
                    writeINode(temp);
                    delete temp;
                    return STATUS_OK;
                } else if (mod > 9 && temp -> dINode.mod > 9 && (temp -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
                    temp -> dINode.mod = mod;
                    writeINode(temp);
                    delete temp;
                    return STATUS_OK;
                } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    delete temp;
                    return STATUS_MOD_ERROR;
                }
            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// 修改拥有者
// 本质是修改dINode的ownerId属性
int UnixFIleSys :: chown(char name[MAX_NAME_SIZE], unsigned short ownerId) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            for (int j = 0; j < os -> ownerNum; j ++)
                if (ownerId == os -> os[j].ownerId) {
                    INode *temp = new INode();
                    temp -> nodeId = d -> direct[i].iNodeId;
                    readINode(temp);
                    if (curOwner -> ownerId == temp -> dINode.ownerId || curOwner -> ownerId == ROOT) {
                        temp -> dINode.ownerId = ownerId;
                        writeINode(temp);
                        delete temp;
                        return STATUS_OK;
                    }  else {
                        delete temp;
                        return STATUS_BEYOND_RIGHT;
                    }
                }
            return STATUS_OWNER_NONEXIST;
        }
    return STATUS_FILENAME_NONEXIST;
}

// 修改所在组
// 本质是修改dINode的groupId属性
int UnixFIleSys :: chgrp(char name[MAX_NAME_SIZE], unsigned short groupId) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            for (int j = 0; j < gs -> groupNum; j ++)
                if (groupId == gs -> gs[j].groupId) {
                    INode *temp = new INode();
                    temp -> nodeId = d -> direct[i].iNodeId;
                    readINode(temp);
                    if (curOwner -> ownerId == temp -> dINode.ownerId || curOwner -> ownerId == ROOT) {
                        temp -> dINode.groupId = groupId;
                        writeINode(temp);
                        delete temp;
                        return STATUS_OK;
                    } else {
                        delete temp;
                        return STATUS_BEYOND_RIGHT;
                    }
                }
            return STATUS_GROUP_NONEXIST;
        }
    return STATUS_FILENAME_NONEXIST;
}

// 修改密码
// 本质是修改Owners中的对应Owner的ownerPassword属性
int UnixFIleSys :: passwd() {
    cout << "please input your old password:";
    char password[MAX_NAME_SIZE]={0};
    int i = 0;
    while (i < MAX_NAME_SIZE) {
        password[i] = getchar();
        if (password[i] == '\n' && i == 0) i = -1;
        if (i != -1 && password[i] == '\n') {
            cout << "检测到换行" << endl;
            for (int j = i; j <= 13; j ++) {
                password[j]='\0';
            }
            break;
        }
        if (password[i] == 13) {
            password[i]='\0';
            break;
        }
        i++;
    }
    cout<<endl;
    // 验证旧密码是否正确
    if (strcmp(curOwner -> ownerPassword, password) != 0)
        return STATUS_PASSWORD_WRONG;
    else {
        cout << "please input your new password:";
        char p1[MAX_NAME_SIZE]={0};
        int i1 = 0;
        while (i1 < MAX_NAME_SIZE) {
            password[i1] = getchar();
            if (password[i1] == '\n' && i1 == 0) i1 = -1;
            if (i1 != -1 && password[i1] == '\n') {
                cout << "检测到换行" << endl;
                for (int j = i1; j <= 13; j ++) {
                    password[j]='\0';
                }
                break;
            }
            if (password[i1] == 13) {
                password[i1]='\0';
                break;
            }
            i1 ++;
        }
        cout<<endl;
        cout << "please input your old password:";
        char p2[MAX_NAME_SIZE]={0};
        int i2 = 0;
        while (i2 < MAX_NAME_SIZE) {
            password[i2] = getchar();
            if (password[i2] == '\n' && i2 == 0) i2 = -1;
            if (i2 != -1 && password[i2] == '\n') {
                cout << "检测到换行" << endl;
                for (int j = i2; j <= 13; j ++) {
                    password[j]='\0';
                }
                break;
            }
            if (password[i2] == 13) {
                password[i2]='\0';
                break;
            }
            i2 ++;
        }
        cout << endl;
        if (strcmp(p1, p2) != 0)
            return STATUS_CONFIRM_WRONG;
        else {
            strcpy(curOwner -> ownerPassword, p1);
            for (int i = 0; i < os -> ownerNum; i ++)
                if (os -> os[i].ownerName == curOwner -> ownerName) {
                    strcpy(os->os[i].ownerPassword, p1);
                    writeOS();
                    return STATUS_OK;
                }
        }
    }
    return STATUS_ERROR;
}

// 重命名文件
// 本质是修改目录项的name
int UnixFIleSys :: mv(char oldName[MAX_NAME_SIZE], char newName[MAX_NAME_SIZE]) {
    for (int i = 0; i < d -> dirNum; i++) {
        if (strcmp(d -> direct[i].name, oldName) == 0) {
            INode* temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            } else {
                for (int j = 0; j < d -> dirNum; j ++)
                    if (strcmp(d -> direct[j].name, newName) == 0)
                        return STATUS_FILENAME_EXIST;
                strcpy(d -> direct[i].name, newName);
                writeDir(curINode -> dINode.addr[0], d);
                delete temp;
                return STATUS_OK;
            }
        }
    }
    return STATUS_FILENAME_NONEXIST;
}

// 新建文件
// 本质是获取一个空闲块（存内容），获取一个新的INode（存文件基础信息），然后修改目录
int UnixFIleSys :: touch(INode* parent, char name[MAX_NAME_SIZE]) {
    bool exist = checkFileName(name);
    // 如果原来的dINode的文件大小为0；
        // 获取一个空闲块（存内容），在父dINode中追加新文件内容存储的块号；
        // 获取一个新的INode（存文件基础信息），将INOdeId存入一个新的目录项，更新目录
    if (parent -> dINode.fileSize == 0 && parent -> dINode.mod != 12 && !exist &&
            (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int blockId = getFreeBlock();
        if (blockId < 0)
            return blockId;
        else {
            int iNodeId = getFreeINode();
            if (iNodeId < 0)
                return iNodeId;
            else {
                parent -> dINode.fileSize = sizeof(Dir);
                time_t now;
                now = time(nullptr);
                parent -> dINode.modifyTime = now;
                parent -> dINode.addr[0] = blockId;
                writeINode(parent);
                Direct *dt = new Direct();
                strcpy(dt -> name, name);
                dt -> iNodeId = iNodeId;
                DINode *di = new DINode();
                di -> createTime = now;
                di -> fileSize = 0;
                di -> groupId = parent -> dINode.groupId;
                di -> linkNum = 0;
                di -> mod = 6;
                di -> modifyTime = now;
                di -> ownerId = parent -> dINode.ownerId;
                di -> readTime = now;
                INode i;
                i.dINode = *di;
                i.nodeId = iNodeId;
                i.parent = parent;
                i.users = parent -> users;
                writeINode(&i);
                Dir	dd;
                dd.dirNum = 1;
                dd.direct[0] = *dt;
                writeDir(blockId, &dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    // 如果原来的dINode的文件大小不为0
        // 获取一个新的INode（存文件基础信息），将INOdeId存入一个新的目录项，更新目录
    } else if (parent -> dINode.fileSize != 0 && parent -> dINode.mod != 12 && !exist &&
            (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(nullptr);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            strcpy(dt -> name, name);
            dt -> iNodeId = iNodeId;
            DINode *di = new DINode();
            di -> createTime = now;
            di -> fileSize = 0;
            di -> groupId = parent -> dINode.groupId;
            di -> linkNum = 0;
            di -> mod = 6;
            di -> modifyTime = now;
            di -> ownerId = parent -> dINode.ownerId;
            di -> readTime = now;
            INode i;
            i.dINode = *di;
            i.nodeId = iNodeId;
            i.parent = parent;
            i.users = parent -> users;
            writeINode(&i);
            Dir dd;
            readDir(parent -> dINode.addr[0], &dd);
            if (dd.dirNum < MAX_DIRECT_NUM) {
                dd.direct[dd.dirNum] = *dt;
                dd.dirNum ++;
                writeDir(parent -> dINode.addr[0], &dd);
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
            else
                return STATUS_BEYOND_DIRECT_NUM;
        }
    }
    else if (parent -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
        return STATUS_BEYOND_RIGHT;
    else if (parent -> dINode.mod == 12)
        return STATUS_READ_ONLY;
    else
    if(exist)
        return STATUS_FILENAME_EXIST;
    return STATUS_ERROR;
}

// 追加内容
// 通过遍历目录对比文件名找到目录项，提取INodeId读出INode的信息，读取追加的字符信息，追加
int UnixFIleSys :: textAppend(char name[MAX_NAME_SIZE])	{
    for (int i = 0; i < d -> dirNum; i ++) {
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode* temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
                return STATUS_BEYOND_RIGHT;
            else if (temp -> dINode.mod > 7) {
                delete temp;
                return STATUS_NOT_FILE;
            } else if (temp -> dINode.mod != 2 && temp -> dINode.mod != 3 && temp -> dINode.mod != 6 && temp -> dINode.mod != 7)  {
                delete temp;
                return STATUS_READ_ONLY;
            } else {
                // 如果原文件大小允许继续追加，不断地读取字符组成字符串text，追加
                if (temp -> dINode.fileSize < FILE_MAX_SIZE) {
                    string text;
                    int j = 0;
                    while (1) {
                        text.push_back(getchar());
                        if (text[j] == 'q' && text[j - 1] == 'w' && text[j - 2] == ':' && text[j - 3] == '\n') {
                            cout << "检测到换行" << endl;
                            text.pop_back();
                            text.pop_back();
                            text.pop_back();
                            text.pop_back();
                            break;
                        }
                        j ++;
                    }
                    int result = writeText(temp, text);
                    writeINode(temp);
                    delete temp;
                    return result;
                }
            }
        }
    }
    return STATUS_FILENAME_NONEXIST;
}

// 读取文件内容
// 通过遍历目录对比文件名找到目录项，提取INodeId读出INode的信息，读取
int UnixFIleSys :: cat(char name[MAX_NAME_SIZE]) {
    for (int i = 0;i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT)
                return STATUS_BEYOND_RIGHT;
            else if (temp -> dINode.mod > 7)
                return STATUS_NOT_FILE;
            else if (temp -> dINode.mod < 4)
                return STATUS_WRITE_ONLY;
            else {
                int result = readText(temp);
                delete temp;
                return result;
            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// 删除
// 通过遍历目录对比文件名找到目录项，提取INodeId读出INode的信息
    // 如果有软链接，则删除目录项，而不是删除文件
    // 如果文件大小为0，回收INode后删除目录项
int UnixFIleSys :: rm(char name[MAX_NAME_SIZE]) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.mod > 7) {
                delete temp;
                return STATUS_NOT_FILE;
            } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            // 如果有软链接，则删除目录项，而不是删除文件
            } else if (temp -> dINode.linkNum > 0) {
                temp -> dINode.linkNum --;
                writeINode(temp);
                Dir* td = new Dir();
                td -> dirNum = d -> dirNum - 1;
                int k = 0;
                for (int j = 0; j < d -> dirNum; j ++)
                    // 覆盖掉要删除的目录项
                    if (j != i)
                        td -> direct[k ++] = d -> direct[j];
                writeDir(curINode -> dINode.addr[0], td);
                Dir* dd = d;
                d = td;
                delete temp;
                delete dd;
                return STATUS_OK;
            } else {
                int bs = temp -> dINode.fileSize / BLOCK_SIZE;
                int ls = temp -> dINode.fileSize % BLOCK_SIZE;
                // 如果文件大小为0，回收INode后删除目录项
                if (temp -> dINode.fileSize == 0) {
                    returnFreeINode(temp -> nodeId);
                    Dir* td=new Dir();
                    td -> dirNum = d -> dirNum - 1;
                    int k = 0;
                    for (int j = 0; j < d -> dirNum; j ++)
                        if (j != i)
                            td -> direct[k ++] = d -> direct[j];
                    writeDir(curINode -> dINode.addr[0], td);
                    Dir* dd = d;
                    d = td;
                    delete temp;
                    delete dd;
                    return STATUS_OK;
                } else {
                // 如果文件大小不为零，逐块回收，再回收INode，最后删除目录项
                    for (int ii = 0; ii < (ls > 0 ? bs + 1 : bs); ii ++)
                        returnFreeBlock(temp -> dINode.addr[ii]);
                    returnFreeINode(temp -> nodeId);
                    Dir* td = new Dir();
                    td -> dirNum = d -> dirNum - 1;
                    int k = 0;
                    for (int j = 0; j < d -> dirNum; j ++)
                        if (j != i)
                            td -> direct[k ++] = d -> direct[j];
                    writeDir(curINode -> dINode.addr[0], td);
                    Dir* dd = d;
                    d = td;
                    delete temp;
                    delete dd;
                    return STATUS_OK;
                }
            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// 创建软链接
// 本质是创建一个新的目录项，目录项中的INodeId为source文件的INodeId
int UnixFIleSys :: ln(char source[MAX_NAME_SIZE], char des[MAX_NAME_SIZE]) {
    if (strcmp(source, des) == 0)
        return STATUS_SDNAME_OVERLAP;
    else
        for (int i = 0; i < d -> dirNum; i ++)
            // 通过文件名找到source文件
            if(strcmp(d -> direct[i].name, source) == 0) {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    for (int j = 0; j < d -> dirNum; j ++)
                        // 如果目标文件已经存在，先删除
                        if (strcmp(d -> direct[j].name, des) == 0) {
                            INode* t = new INode();
                            t -> nodeId = d -> direct[j].iNodeId;
                            readINode(t);
                            if (t -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                                delete t;
                                delete temp;
                                return STATUS_BEYOND_RIGHT;
                            }
                            if((t -> dINode.mod < 8 && temp -> dINode.mod > 7) || (t -> dINode.mod > 7 && temp -> dINode.mod < 8)) {
                                delete t;
                                delete temp;
                                return STATUS_TYPE_NOT_MATCH;
                            }
                            rmIter(d -> direct[j].iNodeId);
                            d -> direct[j].iNodeId = d -> direct[i].iNodeId;
                            writeDir(curINode -> dINode.addr[0], d);
                            delete t;
                            delete temp;
                            return STATUS_OK;
                        }
                    // 建立目录项，INodeId对应被引用的文件的INodeId
                    Direct dt;
                    dt.iNodeId = d -> direct[i].iNodeId;
                    strcpy(dt.name, des);
                    d -> direct[d -> dirNum] = dt;
                    d -> dirNum ++;
                    writeDir(curINode -> dINode.addr[0], d);
                    delete temp;
                    return STATUS_OK;
                }
            }
    return STATUS_FILENAME_NONEXIST;
}

// 删除（文件或目录）
// 通过传入的INodeId判断使文件还是目录，删除的本质使是归还节点和块
void UnixFIleSys :: rmIter(unsigned short iNodeId) {
    INode* temp = new INode();
    temp -> nodeId = iNodeId;
    readINode(temp);
    // 如果传进来的inodeid对应的是文件
    if (temp -> dINode.mod < 8) {
        // 如果连接数大于0，就减少一个连接数
        if (temp -> dINode.linkNum > 0) {
            temp -> dINode.linkNum --;
            writeINode(temp);
            delete temp;
        } else {
        // 如果连接数为0，直接删除
            // 归还节点
            returnFreeINode(temp -> nodeId);
            if (temp -> dINode.fileSize != 0) {
                int bs = temp -> dINode.fileSize / BLOCK_SIZE;
                int ls = temp -> dINode.fileSize % BLOCK_SIZE;
                for (int i = 0; i < (ls > 0 ? bs + 1 : bs); i ++)
                    returnFreeBlock(temp -> dINode.addr[i]);
                delete temp;
            }
        }
    } else {
    // 如果传进来的是目录
        // 如果连接数大于0，就减少一个连接数
        if (temp -> dINode.linkNum > 0) {
            temp -> dINode.linkNum --;
            writeINode(temp);
            delete temp;
        } else {
        // 如果连接数为0，直接删除
            // 归还节点
            returnFreeINode(temp -> nodeId);
            if (temp -> dINode.fileSize != 0) {
                Dir *dd=new Dir();
                readDir(temp -> dINode.addr[0], dd);
                // 深入目录删除每一项
                for (int i = 0; i < dd -> dirNum; i ++)
                    rmIter(dd -> direct[i].iNodeId);
                returnFreeBlock(temp -> dINode.addr[0]);
                delete dd;
                delete temp;
            }
        }
    }
}

// 删除目录
// 实质是进入目录删除目录中的内容（通过目录项的INodeId），再删除目录项（移动之前的目录使待删除的目录项被覆盖）
int UnixFIleSys :: rmdir(char name[MAX_NAME_SIZE])	{
    for (int i = 0; i < d -> dirNum; i ++)
        // 找到名字匹配的目录
        if (strcmp(d -> direct[i].name, name) == 0) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            // 是文件就报错
            if (temp -> dINode.mod < 8) {
                delete temp;
                return STATUS_NOT_DIRECT;
            // 权限校验未通过
            } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            // 通过权限校验
            } else {
                // 深入目录删除每一项
                rmIter(d -> direct[i]. iNodeId);
                Dir* td = new Dir();
                td -> dirNum = d -> dirNum - 1;
                int k = 0;
                // 覆盖需要删除的目录项
                for (int j = 0; j < d -> dirNum; j ++)
                    if (j != i)
                        td -> direct[k ++] = d -> direct[j];
                writeDir(curINode -> dINode.addr[0], td);
                Dir* dd = d;
                d = td;
                delete temp;
                delete dd;
                return STATUS_OK;

            }
        }
    return STATUS_FILENAME_NONEXIST;
}

// 拷贝
// 新建目录项（name是目标文件名），获取一个空闲INode，获取一个空闲块（调用touch方法），写入内容（内容是源文件的内容）
int UnixFIleSys :: cp(char source[MAX_NAME_SIZE], char des[MAX_NAME_SIZE]) {
    if (strcmp(source, des) == 0)
        return STATUS_SDNAME_OVERLAP;
    else
        for (int i = 0; i < d -> dirNum; i ++)
            if (strcmp(d -> direct[i].name, source) == 0) {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (temp -> dINode.mod > 7) {
                    delete temp;
                    return STATUS_NOT_FILE;
                } else if (temp -> dINode.mod < 4) {
                    delete temp;
                    return STATUS_READ_ONLY;
                } else if (temp -> dINode.ownerId != curOwner -> ownerId && temp -> dINode.ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    for (int j = 0; j < d -> dirNum; j ++)
                        // 如果目标文件已经存在，先删除
                        if (d -> direct[j].name == des) {
                            int r1 = rm(des);
                            if (r1 != STATUS_OK) {
                                delete temp;
                                return r1;
                            }
                        }
                    int r2 = touch(curINode, des);
                    if (r2 != STATUS_OK) {
                        delete temp;
                        return r2;
                    }
                    for (int k = 0; k < d -> dirNum; k ++)
                        if (strcmp(d -> direct[k].name, des) == 0) {
                            INode *t = new INode();
                            t -> nodeId = d -> direct[k].iNodeId;
                            readINode(t);
                            string text = getText(temp);
                            writeText(t, text);
                            writeINode(t);
                            delete t;
                            delete temp;
                            return STATUS_OK;
                        }
                }
            }
    return STATUS_FILENAME_NONEXIST;
}

