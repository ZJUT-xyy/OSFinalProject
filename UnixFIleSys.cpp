//
// Created by xyy on 2019/12/17.
//
#include <cstring>
#include <cstdio>
#include <ctime>
#include "UnixFileSys.h"
#include "status.h"

// 初始化数据
void UnixFIleSys :: initGlobal(FILE* f) {
    cout << "现在开始初始化数据" << endl;
    sp -> size = DISK_SIZE;
    sp -> freeBlockNum = BLOCK_NUM; // 初始所有的盘块都为空闲盘块
    sp -> freeINodeNum = INODE_NUM - 1;
    for (int j = 0; j < INODE_NUM; j ++)
        sp -> freeINode[j] = INODE_NUM + 1 - j; // 暂时理解为算偏移量
    sp -> nextFreeINode = INODE_NUM - 2;

    root -> parent = nullptr;
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
    root -> dINode.ownerId = 0;
    root -> dINode.readTime = now;

    os -> ownerNum = 2;
    os -> os[0].ownerId = 1;
    os -> os[0].groupId = 1;
    os -> os[0].ownerName = "coderTT";
    os -> os[0].ownerPassword = "123456";
    os -> os[1].ownerId = 2;
    os -> os[1].groupId = 2;
    os -> os[1].ownerName = "TT";
    os -> os[1].ownerPassword = "123456";

    gs -> groupNum = 2;
    gs -> gs[0].groupId = 1;
    gs -> gs[0].groupName = "zjut";
    gs -> gs[1].groupId = 2;
    gs -> gs[1].groupName = "zjwz";

    f = fopen(FILE_PATH,"wb");
    int bGs = BLOCK_NUM / BLOCK_GROUP_SIZE;
    int left = BLOCK_NUM - bGs * BLOCK_GROUP_SIZE;

    fseek(f, DISK_SIZE, 0);
    fwrite("?", 1, 1, f);
    if (left == 0) {
        for (int i = 0; i < BLOCK_GROUP_SIZE; i ++)
            sp -> freeBlock[i] = BLOCK_START + BLOCK_GROUP_SIZE - 1 - i;
        sp -> nextFreeBlock = BLOCK_GROUP_SIZE - 1;

        for (int j = 0; j < bGs; j ++) {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            unsigned int blocks[BLOCK_GROUP_SIZE];
            for (int k = 0; k < BLOCK_GROUP_SIZE; k ++)
                blocks[k] = left + BLOCK_START + BLOCK_GROUP_SIZE * (j + 1) - k - 1;
            fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * j - 1), SEEK_SET);
            fwrite(&blocksNum, sizeof(blocksNum), 1, f);
            fwrite(&blocks,sizeof(blocks), 1, f);
        }
        unsigned int blocksNum = 0;
        unsigned int blocks[BLOCK_GROUP_SIZE];
        fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * bGs - 1), SEEK_SET);
        fwrite(&blocksNum, sizeof(blocksNum), 1, f);
        fwrite(&blocks,sizeof(blocks), 1, f);
    } else {
        for (int i = 0; i < left; i ++)
            sp -> freeBlock[i] = BLOCK_START + left - 1 - i;
        sp -> nextFreeBlock = left - 1;

        for(int j = 0; j < bGs; j ++) {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            unsigned int blocks[BLOCK_GROUP_SIZE];
            for (int k = 0; k < BLOCK_GROUP_SIZE; k ++)
                blocks[k] = left + BLOCK_START + BLOCK_GROUP_SIZE * (j + 1) - k - 1;
            fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * j - 1), SEEK_SET);
            fwrite(&blocksNum,sizeof(blocksNum), 1, f);
            fwrite(&blocks, sizeof(blocks), 1, f);
        }
        unsigned int blocksNum = 0;
        unsigned int blocks[BLOCK_GROUP_SIZE];
        fseek(f, BLOCK_SIZE * (left + BLOCK_START + BLOCK_GROUP_SIZE * bGs - 1), SEEK_SET);
        fwrite(&blocksNum, sizeof(blocksNum), 1, f);
        fwrite(&blocks, sizeof(blocks), 1, f);
    }
    fseek(f, BLOCK_SIZE,SEEK_SET);
    fwrite(sp, sizeof(SuperBlock), 1, f);	//д??superBlock
    fwrite(os, sizeof(Owners), 1, f);		//д???????
    fwrite(gs, sizeof(Groups), 1, f);		//д???鼯??
    fseek(f, BLOCK_SIZE *root -> nodeId, SEEK_SET);	//д?????iNode
    fwrite(&root -> dINode, sizeof(DINode), 1, f);
    fclose(f);
    auto* dt = new Direct();
    dt -> iNodeId = 2;
    dt -> name = "root";
    ds.push_back(dt);
    superMkdir(root, "coderTT", 1, 1);
    superMkdir(root, "TT", 2, 2);
}


void UnixFIleSys :: initSystem() {
    FILE *f = fopen(FILE_PATH,"rb");
    // f = nullptr;
    if(f == nullptr) {
        cout << "系统暂无数据" << endl;
        initGlobal(f);
    } else {
        fseek(f, BLOCK_SIZE, SEEK_SET);
        fread(sp, sizeof(SuperBlock), 1, f);
        fread(os, sizeof(Owners), 1, f);
        fread(gs, sizeof(Groups), 1, f);
        root -> nodeId = 2;
        root -> parent = nullptr;
        root -> users = 2;
        fseek(f, BLOCK_SIZE * root -> nodeId, SEEK_SET);
        fread(&root -> dINode, sizeof(DINode), 1, f);
        Direct* dt = new Direct();
        dt -> iNodeId = 2;
        dt -> name = "root";
        ds.push_back(dt);
        fclose(f);
        readCurDir();
    }
}

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

bool UnixFIleSys :: writeDir(unsigned int blockId, Dir* d) {
    FILE *f = fopen(FILE_PATH, "rb+");
    if (f == NULL)
        return false;
    else {
        fseek(f, BLOCK_SIZE *blockId, SEEK_SET);
        fwrite(d, sizeof(Dir), 1, f);
        fclose(f);
        return true;
    }
}


bool UnixFIleSys :: readDir(unsigned int blockId, Dir* d) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE *blockId, SEEK_SET);
        fread(d, sizeof(Dir), 1, f);
        fclose(f);
        return true;
    }
}


bool UnixFIleSys :: readNextBG() {
    FILE *f = fopen(FILE_PATH, "rb");
    if(f == nullptr)
        return false;
    else {
        fseek(f, BLOCK_SIZE *sp -> freeBlock[0], SEEK_SET);
        fread(&sp -> nextFreeBlock, sizeof(sp -> nextFreeBlock), 1, f);
        fread(&sp -> freeBlock, sizeof(sp -> freeBlock), 1, f);
        sp -> freeBlockNum --;
        sp -> nextFreeBlock --;
        fclose(f);
        writeSuperBlock();
        return true;
    }
}


int UnixFIleSys :: mkdir(INode* parent, const string& name) {
    bool exist = checkFileName(name);
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
                dt->name = name;
                dt->iNodeId = iNodeId;
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
                dd.dirNum=1;
                dd.direct[0]=*dt;
                writeDir(blockId,&dd);
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    } else if (parent -> dINode.fileSize != 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int iNodeId = getFreeINode();
        if(iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(NULL);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            dt -> name = name;
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
            readDir(parent -> dINode.addr[0], &dd);
            if (dd.dirNum < MAX_DIRECT_NUM) {
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


int UnixFIleSys :: getFreeBlock() {
    if(sp -> freeBlockNum > 0) {
        if(sp -> nextFreeBlock > 0) {
            sp -> freeBlockNum --;
            sp -> nextFreeBlock --;
            writeSuperBlock();
            return sp -> freeBlock[sp -> nextFreeBlock + 1];
        } else {
            readNextBG();
            return getFreeBlock();
        }
    } else
        return STATUS_NO_BLOCK;
}

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


int UnixFIleSys :: login() {
    cout << "please input your userName:";
    string userName;
    cin >> userName;
    cout << "please input your password:";
    string password;
    cin >> password;
    // 限制12字符
    password = password.substr(0,12);
    cout << endl;
    string un;
    un = userName;
    cout << "总共有" << os -> ownerNum << "个用户" << endl;
    for (int j = 0; j < os -> ownerNum; j ++) {
        cout << os -> os[j].ownerName << "     " << os -> os[j].ownerPassword << endl;
        if (un == os -> os[j].ownerName && password == os -> os[j].ownerPassword) {
            curOwner = &os -> os[j];
            return STATUS_OK;
        }
    }
    cout<<"UserName or Password Wrong!!!"<<endl;
    return login();
}

void UnixFIleSys :: commandDispatcher() {
    cout << curOwner -> ownerName << "@UnixFileSystem:~" << pwd() << "$";
    char c[100];
    cin.getline(c,100,'\n');
    while(c[0] == NULL)
        cin.getline(c,100,'\n');
    string command = c;
    command = trim(command);
    int flag = -1;
    int subPos = command.find_first_of(" ");
    if (subPos == -1) {
        if (command == "ls")
            flag = 1;
        else if (command == "pwd")
            flag=5;
        else if (command == "passwd")
            flag = 14;
        else if (command == "sp")
            flag = 17;
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
    switch(flag) {
        case 1: {
            cout << ls() << endl;
            break;
        };
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
                    string name;
                    name = fileName;
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
                    string name;
                    name = fileName;
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
                    string name;
                    name = fileName;
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
        case 5: {
            cout << pwd() << endl;
            break;
        };
        case 6:  {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error DirectName..." << endl;
            else {
                string name;
                name = fileName;
                int result = cd(name);
                if (result == STATUS_FILENAME_NONEXIST)
                    cout << "Error: Dir is not existed..." << endl;
                else if (result == STATUS_NOT_DIRECT)
                    cout << "Error: Not a direct..." << endl;
            }
            break;
        };
        case 7: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                string name;
                name = fileName;
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
        case 8: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                string name;
                name = fileName;
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
                    string oldName1;
                    oldName1 = oldName;
                    string newName1;
                    newName1 = newName;
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
                    string s;
                    s = source;
                    string d;
                    d = des;
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
        case 11: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                string name;
                name = fileName;
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
                    string s;
                    s = source;
                    string d;
                    d = des;
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
        case 13: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                string name;
                name = fileName;
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
        case 15: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                string name;
                name = fileName;
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
        case 16: {
            string fileName = command.substr(subPos + 1);
            fileName = trim(fileName);
            int subPos2 = fileName.find_first_of(" ");
            if (subPos2 != -1)
                cout << "Error FileName..." << endl;
            else {
                string name;
                name = fileName;
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
        }
        default: {
            cout << "Error Command..." << endl;
            break;
        };
    }
    commandDispatcher();
}

void UnixFIleSys :: displayCommands() {
    cout<<"ls		????????"<<endl;
    cout<<"chmod		?????????"<<endl;
    cout<<"chown		???????????"<<endl;
    cout<<"chgrp		????????????"<<endl;
    cout<<"pwd		????????"<<endl;
    cout<<"cd		??????"<<endl;
    cout<<"mkdir		????????"<<endl;
    cout<<"rmdir		???????"<<endl;
    cout<<"mv		????????"<<endl;
    cout<<"cp		???????"<<endl;
    cout<<"rm		??????"<<endl;
    cout<<"ln		???????????"<<endl;
    cout<<"cat		??????????????"<<endl;
    cout<<"passwd		??????????"<<endl;
    cout<<"touch		???????"<<endl;
    cout<<">>		??????????"<<endl;
}


string UnixFIleSys :: pwd() {
    string path;
    for (int i = 0; i < ds.size(); i ++) {
        path += ds[i] -> name;
        path += "/";
    }
    return path;
}

string UnixFIleSys :: trim(string s) {
    if (s.empty())
        return s;
    s.erase(0, s.find_first_not_of(" "));
    s.erase(s.find_last_not_of(" ") + 1);
    return s;
}

int UnixFIleSys :: readCurDir() {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return STATUS_FILE_OPEN_ERROR;
    else {
        if (curINode -> dINode.fileSize == 0)
            d -> dirNum = 0;
        else {
            fseek(f, BLOCK_SIZE *curINode -> dINode.addr[0], SEEK_SET);
            fread(d, sizeof(Dir), 1, f);
            fclose(f);
        }
        return STATUS_OK;
    }
}

string UnixFIleSys :: ls() {
    string ls;
    for (int i = 0; i < d -> dirNum; i++) {
        ls += d -> direct[i].name;
        ls += " ";
    }
    return ls;
}

bool UnixFIleSys :: checkFileName(string name) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return true;
    else {
        Dir d;
        fseek(f, BLOCK_SIZE *curINode -> dINode.addr[0], SEEK_SET);
        fread(&d, sizeof(Dir), 1, f);
        fclose(f);
        for (int i = 0; i < d.dirNum; i ++)
            if(d.direct[i].name == name)
                return true;
        return false;
    }
}

int UnixFIleSys :: cd(string name) {
    Direct *dt = new Direct();
    if (name == "./") {
        delete dt;
        return STATUS_OK;
    } else if (name == "../") {
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
        bool dtExist = false;
        for (int i = 0; i < d -> dirNum; i ++)
            if(d->direct[i].name == name) {
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



int UnixFIleSys :: superMkdir(INode* parent, string name, unsigned short ownerId, unsigned short groupId) {
    bool exist = checkFileName(name);
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
                dt -> name = name;
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
            dt -> name = name;
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

int UnixFIleSys :: chmod(string name, unsigned int mod)	{
    for (int i = 0; i < d -> dirNum; i ++)
        if(d -> direct[i].name == name) {
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

int UnixFIleSys :: chown(string name,  unsigned short ownerId) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (d -> direct[i].name == name) {
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

int UnixFIleSys :: chgrp(string name, unsigned short groupId) {
    for (int i = 0; i < d -> dirNum; i ++)
        if(d->direct[i].name == name) {
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

int UnixFIleSys :: passwd() {
    cout << "please input your old password:";
    string password;
    cin >> password;
    // 控制长度为12
    password = password.substr(0, 12);
    cout << endl;
    if (curOwner -> ownerPassword != password)
        return STATUS_PASSWORD_WRONG;
    else {
        cout << "please input your new password:";
        string p1;
        cin >> p1;
        // 控制长度为12
        p1.substr(0, 12);
        cout << endl;
        cout << "please input confirm your new password:";
        string p2;
        // 控制长度为12
        p2.substr(0, 12);
        cout << endl;
        if (p1 != p2)
            return STATUS_CONFIRM_WRONG;
        else {
            curOwner -> ownerPassword = p1;
            for (int i = 0; i < os -> ownerNum; i ++)
                if (os -> os[i].ownerName == curOwner -> ownerName) {
                    os -> os[i].ownerPassword = p1;
                    writeOS();
                    return STATUS_OK;
                }
        }
    }
    return STATUS_ERROR;
}

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

int UnixFIleSys :: mv(string oldName, string newName) {
    for (int i = 0; i < d -> dirNum; i++) {
        if (d -> direct[i].name == oldName)  {
            INode* temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            } else {
                for (int j = 0; j < d -> dirNum; j ++)
                    if (d -> direct[j].name == newName)
                        return STATUS_FILENAME_EXIST;
                d -> direct[i].name = newName;
                writeDir(curINode -> dINode.addr[0], d);
                delete temp;
                return STATUS_OK;
            }
        }
    }
    return STATUS_FILENAME_NONEXIST;
}

int UnixFIleSys :: touch(INode* parent, string name) {
    bool exist = checkFileName(name);
    if (parent -> dINode.fileSize == 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int blockId=getFreeBlock();
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
                dt -> name = name;
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
                writeDir(blockId, &dd);	//???blockд??????
                //delete dd;
                *d = dd;
                delete di;
                delete dt;
                return STATUS_OK;
            }
        }
    } else if (parent -> dINode.fileSize != 0 && parent -> dINode.mod != 12 && !exist && (parent -> dINode.ownerId == curOwner -> ownerId || curOwner -> ownerId == ROOT)) {
        int iNodeId = getFreeINode();
        if (iNodeId < 0)
            return iNodeId;
        else {
            time_t now;
            now = time(NULL);
            parent -> dINode.modifyTime = now;
            Direct *dt = new Direct();
            dt -> name = name;
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

int UnixFIleSys :: textAppend(string name)	{
    for (int i = 0; i < d -> dirNum; i ++) {
        if (d -> direct[i].name == name) {
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
                if (temp -> dINode.fileSize < FILE_MAX_SIZE) {
                    string text;
                    getline(cin, text, char(17));
                    if (text.size() + temp -> dINode.fileSize > FILE_MAX_SIZE) {
                        delete temp;
                        return STATUS_BEYOND_SIZE;
                    } else {
                        int result = writeText(temp, text);
                        writeINode(temp);
                        delete temp;
                        return result;
                    }
                }
            }
        }
    }
    return STATUS_FILENAME_NONEXIST;
}

int UnixFIleSys :: writeText(INode* temp, string text) {
    FILE *f = fopen(FILE_PATH,"rb+");
    if (f == NULL)
        return STATUS_FILE_OPEN_ERROR;
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;
        int ps = temp -> dINode.fileSize % BLOCK_SIZE;
        int bs = text.size() / BLOCK_SIZE;
        int ls = text.size() % BLOCK_SIZE;
        int pos = 0;
        if (ps == 0) {
            for (int i = 0; i < bs; i ++) {
                int blockId = getFreeBlock();
                if (blockId < 0) {
                    fclose(f);
                    return blockId;
                } else {
                    temp -> dINode.addr[as ++] = blockId;
                    fseek(f, BLOCK_SIZE *blockId, SEEK_SET);
                    fwrite(text.substr(pos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                    temp -> dINode.fileSize += BLOCK_SIZE;
                    pos += BLOCK_SIZE;
                }
            }
            if (ls > 0) {
                int blockId = getFreeBlock();
                if (blockId < 0) {
                    fclose(f);
                    return blockId;
                } else {
                    temp -> dINode.addr[as ++] = blockId;
                    fseek(f, BLOCK_SIZE *blockId, SEEK_SET);
                    fwrite(text.substr(pos, ls).c_str(), BLOCK_SIZE, 1, f);
                    temp -> dINode.fileSize += ls;
                    pos += ls;
                }
                fclose(f);
                return STATUS_OK;
            }
            fclose(f);
            return STATUS_OK;
        } else {
            int lps = BLOCK_SIZE - ps;
            if (text.size() <= lps) {
                fseek(f, BLOCK_SIZE *temp -> dINode.addr[as] + ps, SEEK_SET);
                fwrite(text.c_str(), text.size(), 1, f);
                temp -> dINode.fileSize += text.size();
                fclose(f);
                return STATUS_OK;
            } else {
                fseek(f, BLOCK_SIZE *temp -> dINode.addr[as ++] + ps, SEEK_SET);
                fwrite(text.c_str(), lps, 1, f);
                temp -> dINode.fileSize += lps;
                int lts = text.size() - lps;
                int lbs = lts / BLOCK_SIZE;
                int lls = lts % BLOCK_SIZE;
                int tpos = lps;


                for (int i = 0; i < lbs; i ++) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        temp -> dINode.addr[as ++] = blockId;
                        fseek(f, BLOCK_SIZE *blockId, SEEK_SET);
                        fwrite(text.substr(tpos, BLOCK_SIZE).c_str(), BLOCK_SIZE, 1, f);
                        temp -> dINode.fileSize += BLOCK_SIZE;
                        tpos += BLOCK_SIZE;
                    }
                }
                if(lls > 0) {
                    int blockId = getFreeBlock();
                    if (blockId < 0) {
                        fclose(f);
                        return blockId;
                    } else {
                        temp -> dINode.addr[as ++] = blockId;
                        fseek(f, BLOCK_SIZE *blockId, SEEK_SET);
                        fwrite(text.substr(tpos, lls).c_str(), BLOCK_SIZE, 1, f);
                        temp -> dINode.fileSize += lls;
                        tpos += lls;
                    }
                    fclose(f);
                    return STATUS_OK;
                }
                fclose(f);
                return STATUS_OK;
            }
        }
    }
}

int UnixFIleSys :: cat(string name) {
    for (int i = 0;i < d -> dirNum; i ++)
        if (d -> direct[i].name == name) {
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

int UnixFIleSys :: readText(INode *temp) {
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return STATUS_FILE_OPEN_ERROR;
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;
        int ls = temp -> dINode.fileSize % BLOCK_SIZE;
        char content[BLOCK_SIZE];
        for (int i = 0; i< as; i ++) {
            fseek(f, BLOCK_SIZE *temp -> dINode.addr[i], SEEK_SET);
            fread(content, BLOCK_SIZE, 1, f);
            cout << content;
        }
        if (ls > 0) {
            fseek(f, BLOCK_SIZE *temp -> dINode.addr[as], SEEK_SET);
            fread(content, ls, 1, f);
            for (int i = 0; i < ls; i ++)
                cout << content[i];
        }
        fclose(f);
        return STATUS_OK;
    }
}

int UnixFIleSys :: rm(string name) {
    for (int i = 0; i < d -> dirNum; i ++)
        if (d -> direct[i].name == name) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.mod > 7) {
                delete temp;
                return STATUS_NOT_FILE;
            } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            } else if (temp -> dINode.linkNum > 0) {
                temp -> dINode.linkNum --;
                writeINode(temp);
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
            } else {
                int bs = temp -> dINode.fileSize / BLOCK_SIZE;
                int ls = temp -> dINode.fileSize % BLOCK_SIZE;
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

int UnixFIleSys :: returnFreeBlock(unsigned int blockId) {
    if (sp -> nextFreeBlock <= 18) {
        sp -> freeBlockNum ++;
        sp -> nextFreeBlock ++;
        sp -> freeBlock[sp -> nextFreeBlock] = blockId;
        writeSuperBlock();
        return STATUS_OK;
    } else {
        FILE *f = fopen(FILE_PATH, "rb+");
        if (f == NULL)
            return STATUS_FILE_OPEN_ERROR;
        else {
            unsigned int blocksNum = BLOCK_GROUP_SIZE;
            fseek(f, BLOCK_SIZE *(sp -> freeBlock[0] - BLOCK_GROUP_SIZE * 2), SEEK_SET);
            fwrite(&blocksNum, sizeof(unsigned int), 1, f);
            fwrite(&sp -> freeBlock, sizeof(sp -> freeBlock), 1, f);
            fclose(f);
            sp -> freeBlockNum += 2;
            sp -> freeBlock[0] = sp -> freeBlock[0] - BLOCK_GROUP_SIZE * 2;
            sp -> freeBlock[1] = blockId;
            sp -> nextFreeBlock = 1;
            writeSuperBlock();
            return STATUS_OK;
        }
    }
}

int UnixFIleSys :: returnFreeINode(unsigned int iNodeId) {
    sp -> freeINodeNum ++;
    sp -> nextFreeINode ++;
    sp -> freeINode[sp -> nextFreeINode] = iNodeId;
    writeSuperBlock();
    return STATUS_OK;
}

int UnixFIleSys :: ln(string source, string des) {
    if (source == des)
        return STATUS_SDNAME_OVERLAP;
    else
        for (int i = 0; i < d -> dirNum; i ++)
            if(d -> direct[i].name == source) {
                INode *temp = new INode();
                temp -> nodeId = d -> direct[i].iNodeId;
                readINode(temp);
                if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                    delete temp;
                    return STATUS_BEYOND_RIGHT;
                } else {
                    for (int j = 0; j < d -> dirNum; j ++)
                        if (d -> direct[j].name == des) {
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
                    Direct dt;
                    dt.iNodeId = d -> direct[i].iNodeId;
                    dt.name = des;
                    d -> direct[d -> dirNum] = dt;
                    d -> dirNum ++;
                    writeDir(curINode -> dINode.addr[0], d);
                    delete temp;
                    return STATUS_OK;
                }
            }
    return STATUS_FILENAME_NONEXIST;
}

int UnixFIleSys :: rmdir(string name)	{
    for (int i = 0; i < d -> dirNum; i ++)
        if (d->direct[i].name == name) {
            INode *temp = new INode();
            temp -> nodeId = d -> direct[i].iNodeId;
            readINode(temp);
            if (temp -> dINode.mod < 8) {
                delete temp;
                return STATUS_NOT_DIRECT;
            } else if (temp -> dINode.ownerId != curOwner -> ownerId && curOwner -> ownerId != ROOT) {
                delete temp;
                return STATUS_BEYOND_RIGHT;
            } else {
                rmIter(d -> direct[i]. iNodeId);
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

            }
        }
    return STATUS_FILENAME_NONEXIST;
}

void UnixFIleSys :: rmIter(unsigned short iNodeId) {
    INode* temp = new INode();
    temp -> nodeId = iNodeId;
    readINode(temp);
    if (temp -> dINode.mod < 8) {
        if (temp -> dINode.linkNum > 0) {
            temp -> dINode.linkNum --;
            writeINode(temp);
            delete temp;
        } else {
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
        if (temp -> dINode.linkNum > 0) {
            temp -> dINode.linkNum --;
            writeINode(temp);
            delete temp;
        } else {
            returnFreeINode(temp -> nodeId);
            if (temp -> dINode.fileSize != 0) {
                Dir *dd=new Dir();
                readDir(temp -> dINode.addr[0], dd);
                for (int i = 0; i < dd -> dirNum; i ++)
                    rmIter(dd -> direct[i].iNodeId);
                returnFreeBlock(temp -> dINode.addr[0]);
                delete dd;
                delete temp;
            }
        }
    }
}


int UnixFIleSys :: cp(string source, string des) {
    if (source == des)
        return STATUS_SDNAME_OVERLAP;
    else
        for (int i = 0; i < d -> dirNum; i ++)
            if (d -> direct[i].name == source) {
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
                        if (d -> direct[j].name == des) {
                            int r1=rm(des);
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
                        if (d->direct[k].name == des) {
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

string UnixFIleSys :: getText(INode* temp) {
    string text;
    FILE *f = fopen(FILE_PATH, "rb");
    if (f == NULL)
        return "";
    else {
        int as = temp -> dINode.fileSize / BLOCK_SIZE;
        int ls = temp -> dINode.fileSize % BLOCK_SIZE;
        char content[BLOCK_SIZE];
        for (int i = 0; i < as; i ++) {
            fseek(f, BLOCK_SIZE *temp -> dINode.addr[i], SEEK_SET);
            fread(content, BLOCK_SIZE, 1, f);
            text += content;
        }
        if (ls > 0) {
            fseek(f, BLOCK_SIZE *temp -> dINode.addr[as], SEEK_SET);
            fread(content, ls, 1, f);
            for (int i = 0; i < ls; i ++)
                text += content[i];
        }
        fclose(f);
        return text;
    }
}

