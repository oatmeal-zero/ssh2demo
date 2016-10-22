#include <stdio.h>
#include <stdlib.h>

#include "SshClient.h"

int main(int argc, char *argv[])
{
    const char *hostname="192.168.1.45";
    int port = 22;
    const char *username="car";
    const char *password="111";
    const char *commandline="ls -l";
 
    ssh2_init();

    SshClient ssh(hostname, port);
    ssh.connect(username, password);
    //ssh.execute(commandline);
    //SftpClient sftp(&ssh);
    //sftp.get("test.txt", "test.txt");
    //sftp.put("test2.txt", "test2.txt");

    ssh2_exit();
 
    return 0;
}
