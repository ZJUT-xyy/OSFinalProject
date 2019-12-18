//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_DINODE_H
#define OSFINAL1_DINODE_H

// 磁盘iNode
class DINode {
public:
    unsigned int fileSize;	    // 文件大小
    unsigned int linkNum;	    // 文件的链接数
    unsigned int addr[6];	    // 文件地址:四个直接块号，一个一级间址，一个二级间址
    unsigned short ownerId;	    // 文件拥有者Id
    unsigned short groupId;	    // 文件所属组Id
    unsigned int mod;	        // 文件权限和类型
    long int createTime;	    // 文件创建时间
    long int modifyTime;	    // 文件最后修改时间
    long int readTime;		    // 文件最后访问时间
};


#endif //OSFINAL1_DINODE_H
