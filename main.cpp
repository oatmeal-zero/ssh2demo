#include <stdio.h>
#include <stdlib.h>

#include "SshClient.h"
#include "config.h"
#include "command.h"

const char *_VERSION = "1.0.0";

static void cls()
{
    system("clear");
}

static void suspend()
{
    printf("按回车键继续...\n");
    setbuf(stdin, NULL);
    getchar();
}

TSvrGrp* welcome()
{
    CSvrGrp svrgrp;
    svrgrp.load("config.json");
    TSvrGrp* grp;
    while (true) {
        cls();
        printf("欢迎使用服务器管理工具，当前版本：%s\n", _VERSION);
        svrgrp.list();

        printf("请选择> ");
        char input[128]{0};
        scanf("%s", input);

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
        Command::list();
        printf("请选择> ");
        int input;
        scanf("%d", &input);

        int rtCode = Command::execute(input, &hostmgr);
        if (rtCode < 0) {
            printf("无效的指令，请重新输入！\n");
        } else if (rtCode > 0) {
            if (rtCode == 1) return 0;
            if (rtCode == 2) return 1;
        }
        suspend();
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

    printf("bye.\n");
 
    return 0;
}
