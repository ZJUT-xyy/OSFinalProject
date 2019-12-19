//
// Created by xyy on 2019/12/17.
//

#ifndef OSFINAL1_GROUP_H
#define OSFINAL1_GROUP_H

#include <string>
using namespace std;
struct a {
    int a;
    double b;
};
// 组
class Group {
public:
    unsigned short groupId;	            // 用户组Id
    char groupName[MAX_NAME_SIZE];      // 组名
};

#endif //OSFINAL1_GROUP_H
