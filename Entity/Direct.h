//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_DIRECT_H
#define OSFINAL1_DIRECT_H

#include <string>
using namespace std;

// 目录结构
class Direct {
public:
    string name;	            // 文件或目录的名字
    unsigned short iNodeId;	    // 文件或目录的i节点号
};

#endif //OSFINAL1_DIRECT_H
