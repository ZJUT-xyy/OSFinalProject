#include <iostream>
#include <string>
#include "UnixFileSys.h"
#include "status.h"
#include "define.h"
using namespace std;

int main() {
    UnixFIleSys *unixFIleSys = new UnixFIleSys();
    unixFIleSys -> initSystem();

    if(unixFIleSys -> login() == STATUS_OK) {
        unixFIleSys -> displayCommands();
        unixFIleSys -> commandDispatcher();
    }
}