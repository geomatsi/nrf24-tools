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

#define DATA_SIZE	128

/* */

void mqtt_callback_log(struct mosquitto *mqtt, void *user, int level, const char *str)
{
	fprintf(stdout, "INFO MOSQUITTO<%d> %s\n", level, str);
    fflush(stdout);
}

void mqtt_callback_publish(struct mosquitto *mqtt, void *user, int mid)
{
	fprintf(stdout, "INFO MOSQUITTO: message %d published\n", mid);
    fflush(stdout);
}

void mqtt_callback_connect(struct mosquitto *mqtt, void *user, int result)
{
	fprintf(stdout, "INFO MOSQUITTO: connection response %d\n", result);
    fflush(stdout);
}

/* */

void serial_test_usage(char *name)
{
	printf("usage: %s [-h] -p <serial> -s <baudrate>\n", name);
	printf("\t-h, --help\t\t\tthis help message\n");
	printf("\t-d, --daemon\t\t\tput sensor observer into background after starting\n");
	printf("\t-l, --log\t\t\tlog file for daemon mode\n");
	printf("\t-p, --port <serial>\t\tserial port, default is '/dev/ttyS1\n");
	printf("\t-s, --speed <baudrate>\t\tserial port rate, default is B2400\n");
	printf("\t-n, --name <filename>\t\tfile to store messages\n");
	printf("\t--publish-message\t\tpublish messages to MQTT broker\n");
	printf("\t--print-message\t\tprint messages to console\n");
	printf("\t--dump-message\t\tdump messages to file on disk\n");
}

int print_data(char *data, int n)
{
    int p;

    for(p = 0; p < n; p++) {
        fprintf(stdout, "0x%02x ", *(data + p));
        if ((p > 0) && ((p % 64) == 0))
            fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");
    fflush(stdout);

    return 0;
}

int publish_data(struct mosquitto *m, char *data, int n)
{
	char mqtt_message[DATA_SIZE];
	char mqtt_topic[64];
	int i;

	memset(mqtt_message, 0x0, sizeof(mqtt_message));
	memset(mqtt_topic, 0x0, sizeof(mqtt_topic));
	strncpy(mqtt_message, data, sizeof(mqtt_message) - 1);
	strncpy(mqtt_topic, "test/serial", sizeof(mqtt_topic) - 1);

	if(mosquitto_publish(m, NULL, mqtt_topic, strlen(mqtt_message), mqtt_message, 0, 0)) {
		perror("ERR: mosquitto could not publish message");
        return -1;
	}

    return 0;
}

int dump_data(FILE *fp, char *data, int n)
{
	time_t ticks;
    int rc;

    ticks = time(NULL);

	rc = fprintf(fp, "%lld;%s\n", (long long) ticks, data);
    if (rc < 0) {
        perror("ERR: couldn't dump message to file");
        return -1;
    }

    rc = fflush(fp);
    if (rc != 0) {
        perror("ERR: couldn't fflush");
        return -1;
    }

    return 0;
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

void setup_serial_termios(struct termios *ios, struct termios *old_ios, int baud)
{
	memcpy(ios, old_ios, sizeof(*ios));

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

void setup_stdin_termios(struct termios *ios, struct termios *old_ios)
{
	memcpy(ios, old_ios, sizeof(*ios));
	ios->c_lflag &= ~(ECHO|ICANON);

	return;
}

/* */

int main(int argc, char *argv[])
{
	/* */

	bool publish_message = false;
	bool print_message = false;
	bool dump_message = false;
	bool daemon_mode = false;

	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;

	struct mosquitto *mqtt = NULL;
	char *mqtt_id = "serial_data";

	/* */

	char *log_name = "/var/log/sensor_pub.log";
    FILE *lfp = NULL;
    pid_t pid;

	char *dump_name = "/data/data.txt";
    FILE *dfp = NULL;

	char *port_name = "/dev/ttyS1";
	int speed = B2400;

	struct termios sterm, old_sterm;
	struct termios kterm, old_kterm;
	struct timeval tv;

	char message[DATA_SIZE];
	int pos;
	char ch;

	int pfd, tfd, rc;
	fd_set rset;

	bool active = true;
	bool ready = false;

	/* command line options */

	int opt;
	const char opts[] = "p:s:n:l:hd";
    const struct option longopts[] = {
        {"port", required_argument, NULL, 'p'},
        {"speed", required_argument, NULL, 's'},
        {"name", required_argument, NULL, 'n'},
        {"log", required_argument, NULL, 'l'},
        {"daemon", no_argument, NULL, 'd'},
        {"publish-message", no_argument, NULL, '0'},
        {"print-message", no_argument, NULL, '1'},
        {"dump-message", no_argument, NULL, '2'},
        {"help", optional_argument, NULL, 'h'},
        {NULL,}
    };

    while (opt = getopt_long(argc, argv, opts, longopts, &opt), opt > 0) {
        switch (opt) {
			case 'p':
				port_name = strdup(optarg);
				break;
			case 'n':
				dump_name = strdup(optarg);
				break;
			case 'l':
				log_name = strdup(optarg);
				break;
			case 'd':
				daemon_mode = true;
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
			case '1':
				print_message = true;
				break;
			case '2':
				dump_message = true;
				break;
            case 'h':
			default:
				serial_test_usage(argv[0]);
				exit(0);
        }
    }

    /* go to background mode */

    if (daemon_mode) {

        pid = fork();

        if (pid < 0) {
            perror("first fork failed");
            exit(-1);
        }

        if (pid > 0) {
            /* let the parent terminate */
            exit(0);
        }

        if (0 > setsid()) {
            perror("setsid failed");
            exit(-1);
        }

        signal(SIGHUP, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);

        pid = fork();

        if (pid < 0) {
            perror("second fork failed");
            exit(-1);
        }

        if (pid > 0) {
            /* let the parent terminate */
            exit(0);
        }

        umask(0);
        chdir("/");

        if (NULL == (lfp = fopen(log_name, "a"))) {
            perror("ERR: couldn't not open log file");
            exit(-1);
        }

        dup2(fileno(lfp), fileno(stdout));
        dup2(fileno(lfp), fileno(stderr));
        close(fileno(stdin));
    }

	/* setup mosquitto */

    if (publish_message) {

        mosquitto_lib_init();

        mqtt = mosquitto_new(mqtt_id, clean_session, NULL);
        if (!mqtt) {
            fprintf(stderr, "ERR: can not allocate mosquitto context\n");
            exit(-1);
        }

        mosquitto_log_callback_set(mqtt, mqtt_callback_log);
        mosquitto_publish_callback_set(mqtt, mqtt_callback_publish);
        mosquitto_connect_callback_set(mqtt, mqtt_callback_connect);

        if(mosquitto_connect(mqtt, host, port, keepalive)) {
            fprintf(stderr, "ERR: can not connect to mosquitto server %s:%d\n", host, port);
            exit(-1);
        }

        if (mosquitto_loop_start(mqtt)) {
            fprintf(stderr, "ERR: can not start mosquitto thread\n");
            exit(-1);
        }
    }

    /* setup dump file */

    if (dump_message) {
        dfp = fopen(dump_name, "a");

        if (dfp == NULL) {
            perror("ERR: can not open dump file");
            goto mqtt_out;
        }
    }

	/* setup serial port */

	pfd = open(port_name, O_RDWR | O_NOCTTY);

	if (pfd < 0) {
		perror("ERR: can not open serial port");
		goto dump_out;
	}

	if (tcgetattr(pfd, (void *) &old_sterm) < 0) {
		perror("ERR: serial tcgetattr");
		close(pfd);
		goto dump_out;
	}

	setup_serial_termios(&sterm, &old_sterm, speed);

	if (tcsetattr(pfd, TCSANOW, &sterm) < 0) {
		perror("ERR: serial tcsetattr");
		close(pfd);
		goto serial_out;
	}

	/* setup non-blocking stdin if in foreground mode */

    if (daemon) {

        tfd = -1;

    } else {

        tfd = STDIN_FILENO;

        if (tcgetattr(tfd, (void *) &old_kterm) < 0) {
            perror("ERR: stdin tcgetattr");
            goto serial_out;
        }

        setup_stdin_termios(&kterm, &old_kterm);

        if (tcsetattr(tfd, TCSANOW, &kterm) < 0) {
            perror("ERR: stdin tcsetattr");
            goto stdin_out;
        }
    }

	/* main processing */

	memset(message, 0x0, sizeof(message));
	pos = 0;

	while (active) {

		FD_ZERO(&rset);
		FD_SET(pfd, &rset);

        if (!daemon)
            FD_SET(tfd, &rset);

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		rc = select((pfd > tfd) ? (pfd + 1) : (tfd + 1), &rset, NULL, NULL, &tv);

		if (rc == 0) {
			/* select timeout */
			continue;
		}

		if (rc == -1) {

			if (errno == EINTR) {
				fprintf(stdout, "select was interrupted, try again\n");
                fflush(stdout);
				continue;
			} else {
				perror("ERR: select");
				active = false;
				continue;
			}
		}

		if (FD_ISSET(tfd, &rset)) {

			rc = read(tfd, &ch, 1);

			if (rc < 0) {
				perror("ERR: read stdin");
				active = false;
				continue;
			}

			if (rc == 0) {
				fprintf(stdout, "got 0 bytes from stdin\n");
				continue;
			}

			switch (ch) {
				case 'x':
					fprintf(stdout, "exit command: [%c]\n", ch);
					active = false;
					break;
				case 't':
					fprintf(stdout, "test command: [%c]\n", ch);
					break;
				default:
					fprintf(stdout, " unknown command [%c]\n", ch);
					break;
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
				fprintf(stdout, "got 0 bytes from serial port\n");
				continue;
			}

			switch (ch) {
				case '\n':
					ready = true;
					break;
				case '\r':
					ready = true;
					break;
				default:
					message[pos++] = ch;
					break;
			}

			if ((ready) || ((pos + 1) == sizeof(message))) {

				/* process non-empty message */

				if (pos > 0) {

                    rc = 0;

					if (publish_message) {
						rc += publish_data(mqtt, (char *) message, strlen(message));
					}

                    if (print_message) {
                        rc += print_data((char *) message, strlen(message));
                    }

                    if (dump_message) {
					    rc += dump_data(dfp, (char *) message, strlen(message));
					}

                    if (rc < 0) {
					    fprintf(stderr, "exit on backend errors...\n");
                        active = false;
                    }
				}

				/* prepare for next serial message */

				ready = false;
				memset(message, 0x0, sizeof(message));
				pos = 0;
			}

		}
	}

	/* restore stdin settings before exit */

stdin_out:

    if (!daemon) {
        if (tcsetattr(tfd, TCSANOW, &old_kterm) < 0) {
            perror("ERR: restore stdin tcsetattr");
        }
    }

	/* restore serial settings before exit */

serial_out:

	if (tcsetattr(pfd, TCSANOW, &old_sterm) < 0) {
		perror("ERR: restore serial tcsetattr");
	}

	close(pfd);

dump_out:

    if (dump_message) {
        fclose(dfp);
    }

mqtt_out:

    if (publish_message) {
        mosquitto_destroy(mqtt);
    }

	return 0;
}
