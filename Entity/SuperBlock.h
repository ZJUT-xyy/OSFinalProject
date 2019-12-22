//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_SUPERBLOCK_H
#define OSFINAL1_SUPERBLOCK_H

#include"../define.h"
#include <string>
using namespace std;


// 超级块
class SuperBlock {
public:
    unsigned int size;	                               // 磁盘大小
    unsigned int freeINodeNum;	                       // 空闲INode数
    unsigned int freeINode[INODE_NUM];	               // 空闲INode栈
    unsigned int nextFreeINode;	                       // 栈中下一个INode
    unsigned int freeBlockNum;	                       // 空闲组数
    unsigned int freeBlock[BLOCK_GROUP_SIZE];	       // 当前组的空闲块栈（每一项的值是空闲块的块号）
    unsigned int nextFreeBlock;	                       // 一级空闲栈中下一组的第一块（相对于组内的编号）
};

#endif //OSFINAL1_SUPERBLOCK_H
