#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <termios.h>
#include <stdbool.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

#include <mosquitto.h>

/* */

void mqtt_callback_log(struct mosquitto *mqtt, void *user, int level, const char *str)
{
	printf("INFO MOSQUITTO<%d> %s\n", level, str);
}

void mqtt_callback_publish(struct mosquitto *mqtt, void *user, int mid)
{
	printf("INFO MOSQUITTO: message %d published\n", mid);
}

void mqtt_callback_connect(struct mosquitto *mqtt, void *user, int result)
{
	printf("INFO MOSQUITTO: connection response %d\n", result);
}

/* */

void serial_test_usage(char *name)
{
	printf("usage: %s [-h] -p <serial> -s <baudrate>\n", name);
	printf("\t-h, --help\t\t\tthis help message\n");
	printf("\t-p, --port <serial>\t\tserial port, default is '/dev/ttyS1\n");
	printf("\t-s, --speed <baudrate>\t\tserial port rate, default is B2400\n");
	printf("\t--publish-message\t\tpublish messages to MQTT broker\n");
}

dump_data(char *b, int n)
{
    int p;

    for(p = 0; p < n; p++) {
        printf("0x%02x ", *(b + p));
        if ((p > 0) && ((p % 64) == 0))
            printf("\n");
    }

    printf("\n");
}

void publish_data(struct mosquitto *m, char *data, int n)
{
	char mqtt_message[64];
	char mqtt_topic[64];
	int i;

	memset(mqtt_message, 0x0, sizeof(mqtt_message));
	memset(mqtt_topic, 0x0, sizeof(mqtt_topic));
	printf("serial_data[%s]\n", data);
	strncpy(mqtt_message, data, sizeof(mqtt_message) - 1);
	strncpy(mqtt_topic, "test/serial", sizeof(mqtt_message) - 1);

	if(mosquitto_publish(m, NULL, mqtt_topic, strlen(mqtt_message), mqtt_message, 0, 0)) {
		printf("ERR: mosquitto could not publish message\n");
	}

    return;
}

int convert_baudrate(int speed)
{
	switch (speed) {
		case 1200:
			speed = B1200;
			break;
		case 2400:
			speed = B2400;
			break;
		case 4800:
			speed = B4800;
			break;
		case 9600:
			speed = B9600;
			break;
		case 19200:
			speed = B19200;
			break;
		case 38400:
			speed = B38400;
			break;
		case 57600:
			speed = B57600;
			break;
		case 115200:
			speed = B115200;
			break;
		default:
			speed = 0;
			break;
	}

	return speed;
}

void setup_serial_port(struct termios *ios, int baud)
{
	cfmakeraw(ios);

	/* Input flags */

	ios->c_iflag |= IGNBRK | IGNPAR;

	/* Control flags */

	ios->c_cflag |= CREAD | CLOCAL;
	ios->c_cflag &= ~CRTSCTS;

	/* Control char's */

	ios->c_cc[VINTR] = 0;
	ios->c_cc[VQUIT] = 0;
	ios->c_cc[VERASE] = 0;
	ios->c_cc[VKILL] = 0;
	ios->c_cc[VEOF] = 0;
	ios->c_cc[VMIN] = 0;
	ios->c_cc[VEOL] = 0;
	ios->c_cc[VTIME] = 0;
	ios->c_cc[VEOL2] = 0;
	ios->c_cc[VSTART] = 0;
	ios->c_cc[VSTOP] = 0;
	ios->c_cc[VSUSP] = 0;
	ios->c_cc[VLNEXT] = 0;
	ios->c_cc[VWERASE] = 0;
	ios->c_cc[VREPRINT] = 0;
	ios->c_cc[VDISCARD] = 0;

	cfsetspeed(ios, baud);
	return;
}
/* */

int main(int argc, char *argv[])
{
	/* */

	bool publish_message = false;

	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;

	struct mosquitto *mqtt = NULL;
	char *mqtt_id = "serial_data";

	/* */

	char *port_name = "/dev/ttyS1";
	int speed = B2400;

	struct termios ios, old_ios;
	struct timeval tv;

	char buffer[64];
	char *ptr;
	char ch;

	int pfd, rc;
	fd_set rset;

	bool active = true;

	/* command line options */

	int opt;
	const char opts[] = "p:s:h:";
    const struct option longopts[] = {
        {"port", required_argument, NULL, 'p'},
        {"speed", required_argument, NULL, 's'},
        {"publish-message", no_argument, NULL, '0'},
        {"help", optional_argument, NULL, 'h'},
        {NULL,}
    };

    while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
        switch (opt) {
			case 'p':
				port_name = strdup(optarg);
				break;
			case 's':
                speed = atoi(optarg);
                if (0 == (speed = convert_baudrate(speed))) {
                    printf("ERR: invalid serial speed %s\n", optarg);
                    serial_test_usage(argv[0]);
                    exit(-1);
                }
                break;
			case '0':
				publish_message = true;
				break;
            case 'h':
			default:
				serial_test_usage(argv[0]);
				exit(0);
        }
    }

	/* setup serial port */

	pfd = open(port_name, O_RDWR | O_NOCTTY);

	if (pfd < 0) {
		perror("ERR: can not open serial port");
		exit(-1);
	}

	setup_serial_port(&ios, speed);

	if (tcgetattr(pfd, &old_ios) < 0) {
		perror("ERR: tcgetattr");
		close(pfd);
		exit(-1);
	}

	if (tcsetattr(pfd, TCSANOW, &ios) < 0) {
		perror("ERR: tcsetattr");
		close(pfd);
		exit(-1);
	}

	/* setup mosquitto */

	mosquitto_lib_init();

	mqtt = mosquitto_new(mqtt_id, clean_session, NULL);
	if (!mqtt) {
		printf("ERR: can not allocate mosquitto context\n");
		exit(-1);
	}

	mosquitto_log_callback_set(mqtt, mqtt_callback_log);
	mosquitto_publish_callback_set(mqtt, mqtt_callback_publish);
	mosquitto_connect_callback_set(mqtt, mqtt_callback_connect);

	if(mosquitto_connect(mqtt, host, port, keepalive)) {
		printf("ERR: can not connect to mosquitto server %s:%d\n", host, port);
		exit(-1);
	}

	if (mosquitto_loop_start(mqtt)) {
		printf("ERR: can not start mosquitto thread\n");
		exit(-1);
	}

	/* main processing */

	bzero(buffer, sizeof(buffer));
	ptr = buffer;

	while (active) {

		FD_ZERO(&rset);
		FD_SET(pfd, &rset);
		FD_SET(0, &rset);

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		rc = select(pfd + 1, &rset, NULL, NULL, &tv);

		if (rc == 0) {
			/* select timeout */
			continue;
		}

		if (rc == -1) {

			if (errno == EINTR) {
				fprintf(stdout, "select was interrupted, try again\n");
				continue;
			} else {
				perror("ERR: select");
				active = false;
				continue;
			}
		}

		if (FD_ISSET(0, &rset)) {

			rc = read(0, &ch, 1);

			if (rc < 0) {
				perror("ERR: read stdin");
				active = false;
				continue;
			}

			if (rc == 0) {
				fprintf(stdout, "got 0 bytes from stdin\n");
				continue;
			}

			if (ch == 'x') {
				fprintf(stdout, " exit command: [%c]\n", ch);
				active = false;
			} else {
				fprintf(stdout, " unknown command [%c[\n", ch);
			}
		}

		if (FD_ISSET(pfd, &rset)) {

			rc = read(pfd, &ch, 1);

			if (rc < 0) {
				perror("ERR: read serial");
				active = false;
				continue;
			}

			if (rc == 0) {
				fprintf(stdout, "got 0 bytes from port\n");
				continue;
			}

			if ((ch == '\n') || (strlen(buffer) + 1 == sizeof(buffer))) {

				/* process serial line */

				if (publish_message) {
					publish_data(mqtt, (char *) buffer, strlen(buffer));
				} else {
					dump_data((char *) buffer, strlen(buffer));
				}

				/* prepare for next serial line */

				bzero(buffer, sizeof(buffer));
				ptr = buffer;
				continue;
			}

			*ptr++ = ch;
			continue;
		}
	}


	/* restore serial settings before exit */

	if (tcsetattr(pfd, TCSANOW, &ios) < 0) {
		perror("ERR: restore settings using tcsetattr");
		close(pfd);
		exit(-1);
	}

	/* TODO: mqtt client graceful shutdown */

	return 0;
}
