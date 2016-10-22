#include "libssh2_config.h"
#include <libssh2.h>
 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
 
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>
#include <stdexcept>

#include "SshClient.h"

using namespace std;

void ssh2_init()
{
    int rc = libssh2_init(0);
    assert(rc == 0 && "libssh2 initialization failed\n");
}

void ssh2_exit()
{
    libssh2_exit();
}

bool SshClient::conn(const char* hostname, int port)
{
    struct sockaddr_in sin;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(hostname);
    if (::connect(sock, (struct sockaddr*)(&sin),
                sizeof(struct sockaddr_in)) != 0) 
    {
        fprintf(stderr, "failed to connect!\n");
        return false;
    }

    return true;
}

SshClient::SshClient(const char *hostname, int port)
{
    assert(conn(hostname, port));
    session = libssh2_session_init();
    assert(session);
}

SshClient::~SshClient()
{
    libssh2_session_disconnect(session, "normal Shutdown");
    libssh2_session_free(session);
    close(sock);
}

bool SshClient::connect(const char *username, const char *password)
{
    int rc = libssh2_session_handshake(session, sock);
    if(rc) {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        return false;
    }

    if (libssh2_userauth_password(session, username, password)) {
        fprintf(stderr, "Authentication by password failed.\n");
        return false;
    }

    return true;
}

bool SshClient::execute(const char *command)
{
    int exitcode;
    char *exitsignal=(char *)"none";

    fprintf(stdout, "execute command [%s]\n", command);

    LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(session);
    if( channel == NULL )
    {
        fprintf(stderr, "libssh2_channel_open_session() error\n");
        return false;
    }

    int rc = libssh2_channel_exec(channel, command);
    if( rc != 0 )
    {
        fprintf(stderr, "libssh2_channel_exec() error %d\n", rc);
        return false;
    }

    fprintf(stdout, "*****************response below*****************\n");
    do {
        char buffer[0x4000];
        rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );
        for(int i=0; i < rc; ++i )
            fputc(buffer[i], stdout);
        fprintf(stdout, "\n");
    } while(rc > 0);

    exitcode = 127;
    rc = libssh2_channel_close(channel);
    if( rc == 0 )
    {
        exitcode = libssh2_channel_get_exit_status( channel );
        libssh2_channel_get_exit_signal(channel, &exitsignal,
                NULL, NULL, NULL, NULL, NULL);
    }
 
    if (exitsignal) {
        fprintf(stdout, "Got signal: %s\n", exitsignal);
    }
    else {
        fprintf(stdout, "exit code: %d \n", exitcode);
    }
 
    libssh2_channel_free(channel);
    channel = NULL;

    return true;
}

bool SshClient::open_channel()
{
    /* Request a shell */ 
    if (!(channel = libssh2_channel_open_session(session))) {

        fprintf(stderr, "Unable to open a session\n");
        return false;
    }

    /* Some environment variables may be set,
     * It's up to the server which ones it'll allow though
     */ 
    //libssh2_channel_setenv(channel, "FOO", "bar");


    /* Request a terminal with 'vanilla' terminal emulation
     * See /etc/termcap for more options
     */ 
    if (libssh2_channel_request_pty(channel, "vanilla")) {

        fprintf(stderr, "Failed requesting pty\n");
        return false;
    }

    /* Open a SHELL on that pty */ 
    if (libssh2_channel_shell(channel)) {

        fprintf(stderr, "Unable to request shell on allocated pty\n");
        return false;
    }

    return true;
}

void SshClient::channel_read()
{
    int rc = 0;
    //do {
        char buffer[0x4000]{0};
        rc = libssh2_channel_read( channel, buffer, sizeof(buffer) );
        printf("%s\n", buffer);
        //for(int i=0; i < rc; ++i )
        //    fputc(buffer[i], stdout);
        //fprintf(stdout, "\n");
    //} while(rc > 0);
}

bool SshClient::channel_write(const char *command)
{
    char cmd[2048]{0};
    strcpy(cmd, command);
    strcat(cmd, "\n");
    libssh2_channel_write(channel, cmd, strlen(cmd));
    /* At this point the shell can be interacted with using
     * libssh2_channel_read()
     * libssh2_channel_read_stderr()
     * libssh2_channel_write()
     * libssh2_channel_write_stderr()
     *
     * Blocking mode may be (en|dis)abled with: libssh2_channel_set_blocking()
     * If the server send EOF, libssh2_channel_eof() will return non-0
     * To send EOF to the server use: libssh2_channel_send_eof()
     * A channel can be closed with: libssh2_channel_close()
     * A channel can be freed with: libssh2_channel_free()
     */ 
    return true;
}

LIBSSH2_SFTP *SshClient::open_sftp()
{
    return libssh2_sftp_init(session);
}

SftpClient::SftpClient(SshClient *ssh)
{
    sftp_session = ssh->open_sftp();
    if (!sftp_session) {
        throw runtime_error("Unable to init SFTP session");
    }
}

SftpClient::~SftpClient()
{
    libssh2_sftp_shutdown(sftp_session);
}

int SftpClient::get(const char *remotepath, const char *localpath)
{
    LIBSSH2_SFTP_HANDLE *sftp_handle = 
        libssh2_sftp_open(sftp_session, remotepath, LIBSSH2_FXF_READ, 0);
    if (!sftp_handle) {
        fprintf(stderr, "Unable to open file with SFTP %s\n", remotepath);
        return -1;
    }

    FILE *fp = fopen(localpath, "wb");
    if (!fp) {
        fprintf(stderr, "Can't open local file %s\n", localpath);
        return -1;
    }

    int rc;
    do {
        char mem[1024];
        rc = libssh2_sftp_read(sftp_handle, mem, sizeof(mem));

        if (rc > 0) {
            fwrite(mem, 1, rc, fp);
        } else {
            break;
        }
    } while (1);

    fclose(fp);
    libssh2_sftp_close(sftp_handle);

    return 0;
}

int SftpClient::put(const char *localpath, const char *remotepath)
{
    LIBSSH2_SFTP_HANDLE *sftp_handle = 
        libssh2_sftp_open(sftp_session, remotepath,
                LIBSSH2_FXF_WRITE|LIBSSH2_FXF_CREAT|LIBSSH2_FXF_TRUNC,
                LIBSSH2_SFTP_S_IRUSR|LIBSSH2_SFTP_S_IWUSR|
                LIBSSH2_SFTP_S_IRGRP|LIBSSH2_SFTP_S_IROTH);
    if (!sftp_handle) 
    {
        fprintf(stderr, "Unable to open file with SFTP %s\n", remotepath);
        return -1;
    }

    FILE *fp = fopen(localpath, "rb");
    if (!fp) {
        fprintf(stderr, "Can't open local file %s\n", localpath);
        return -1;
    }

    int rc;
    char mem[1024*100];
    size_t nread;
    char *ptr;
    do {
        nread = fread(mem, 1, sizeof(mem), fp);
        if (nread <= 0) {
            /* end of file */ 
            break;
        }
        ptr = mem;
        do {
            /* write data in a loop until we block */ 
            rc = libssh2_sftp_write(sftp_handle, ptr, nread);
            if(rc < 0)
                break;
            ptr += rc;
            nread -= rc;
        } while (nread);
    } while (rc > 0);

    fclose(fp);
    libssh2_sftp_close(sftp_handle);

    return 0;
}
