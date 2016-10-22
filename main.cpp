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

    SshClient client(hostname, port);
    client.connect(username, password);
    //client.execute(commandline);
    //client.open_channel();
    //client.channel_read();
    //client.channel_write("cd carv3");
    //client.channel_read();
    //client.channel_write("svn up ");
    //client.channel_read();
    //SftpClient sftp(&client);
    //sftp.get("cdkey_record.csv", "batch_record.csv");
    //sftp.put("dir.mk", "dir.mk");

    ssh2_exit();
 
    return 0;
}
