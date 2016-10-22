/*
 * 封装libssh2库，
 * 只为了管理远程服务器，封装几个常用的命令
 * 后续有时间或者有需求在细化
 *
 * 注意：
 * 1.应用程序在使用本类时需要先执行
 * ssh2_init();
 *
 * 2.应用程序在即将退出时应执行
 * ssh2_exit();
 */
#ifndef _SSH_CLIENT_H_
#define _SSH_CLIENT_H_

#include <libssh2.h>
#include <libssh2_sftp.h>

/* 以下两个函数非线程安全 */
/* 初始化libssh2库 */
void ssh2_init();
/* 回收libssh2使用的系统资源 */
void ssh2_exit();

class SshClient
{
public:
    SshClient(const char *hostname, int port);
    ~SshClient();

    bool connect(const char *username, const char *password);
    bool execute(const char *command);
    void channel_read();
    bool channel_write(const char *command);
    bool open_channel();

    LIBSSH2_SFTP *open_sftp();

private:
    bool conn(const char* hostname, int port);

private:
    int sock;
    LIBSSH2_SESSION *session;
    LIBSSH2_CHANNEL *channel;
};

class SftpClient
{
public:
    SftpClient(SshClient *ssh);
    ~SftpClient();

    int get(const char *remotepath, const char *localpath);
    int put(const char *localpath, const char *remotepath);

private:
    LIBSSH2_SFTP *sftp_session;
};

#endif
