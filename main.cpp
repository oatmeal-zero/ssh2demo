#include <stdio.h>
#include <stdlib.h>

#include "SshClient.h"
#include "config.h"

const char *_VERSION = "1.0.0";

void cls()
{
    system("clear");
}

void suspend()
{
    setbuf(stdin, NULL);
    cout << "按回车键继续..." << endl;
    getchar();
}

TSvrGrp* welcome()
{
    CSvrGrp svrgrp;
    svrgrp.load("config.json");
    TSvrGrp* grp;
    while (true) {
        cls();
        cout << "欢迎使用服务器管理工具，当前版本：" << _VERSION << endl;
        svrgrp.list();

        cout << "请选择> ";
        string input;
        cin >> input;

        grp = svrgrp.getGrp(input);
        if (grp) break;
    } 

    return grp;
}

int dispatch(TSvrGrp* grp)
{
    if (grp->name.empty()) return -1;

    CHostMgr hostmgr;
    hostmgr.load(grp->config.c_str());

    while (true) {
        cls();
        hostmgr.list();
        string input;
        cout << "[B]后退 [U]更新 [S]停服 [R]启动 [Q]退出" << endl;
        cout << "请选择> ";
        cin >> input;

        switch (input[0]) {
            case 'b':
            case 'B':
                return 1;
            case 'u':
            case 'U':
                hostmgr.update();
                suspend();
                break;
            case 's':
            case 'S':
                hostmgr.stop();
                suspend();
                break;
            case 'r':
            case 'R':
                hostmgr.run();
                suspend();
                break;
            case 'q':
            case 'Q':
                return 0;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    ssh2_init();

    int ret = 0;
    while (true) {
        TSvrGrp* grp = welcome();
        ret = dispatch(grp);
        if (ret == 0) break;
    }

    ssh2_exit();
 
    return 0;
}
