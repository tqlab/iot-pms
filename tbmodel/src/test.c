#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <getopt.h>

int set_interface_attribs(int fd, int speed);

int main(int argc, char *argv[]) {

    char *dev_file_path = "/dev/ttyUSB0";
    int dev_fd = open(dev_file_path, O_RDWR | O_NOCTTY);
    if (dev_fd < 0) {
        printf("Error opening %s: %s\n", dev_file_path, strerror(errno));
        return -1;
    }
    set_interface_attribs(dev_fd, B9600);

    tcdrain(dev_fd);    /* delay for output */

    uint8_t frameBuf[64];
    ssize_t rdlen = 0;

    int st = 0;

    while ((rdlen = read(dev_fd, frameBuf, 64)) > 0) {

        for (int i = 0; i < rdlen; ++i) {
            if (st == 0 && frameBuf[i]==255) {
                st = 1;
            }

            if (st == 1 && frameBuf[i] == 255) {
                st = 2;
            }

            if (st == 2 && frameBuf[i] == 170) {
                st = 3;
            }

            printf("%d ", frameBuf[i]);

            if (st == 3) {
                st = 0;
                printf("\n");
            }
        }


        fflush(stdout);
    }



}

int set_interface_attribs(int fd, int speed) {
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t) speed);
    cfsetispeed(&tty, (speed_t) speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}