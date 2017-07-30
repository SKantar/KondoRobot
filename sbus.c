#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <sys/types.h>
#include "sbus.h"

#define SBUS_PORT	"/dev/sbus"
#define SBUS_BAUD_RATE	B115200
#define BUFF_SIZE	4096
#define PWM_MIN		13640
#define PWM_MAX		41656
#define PWM_CENT	((PWM_MIN+PWM_MAX)/2)

#define DEBUG
//#define DEBUG_SBUS

#ifdef DEBUG
    #define PRINTF(x) printf(x)
    #define PRINTF2(x,xx) printf(x,xx)
#else
    #define PRINTF(x)
    #define PRINTF2(x,xx)
#endif

typedef struct termios termios;
typedef struct sbus_sv {
    unsigned short s[24];
} sbus_sv;

sbus_sv serv_curr, serv_next;

static int serv_dir[24] = {
        0, 1, -1, 1, 1, 0, 1, -1, -1, 0, 1, 1, 1, -1, -1, 1, 1, -1, -1, 1, 1, -1, 0, 0
};

static int serv_balance[24] = {
    0, 620, 10540, 13950, 0, 0, -11005, -3585, 0, 0, 310, -575, 930, -6510, 3255, 0, -775, 155, 0, 6510, -3255, 1240, 0, 0
};

static int balanced = 0;
static int sbus_fd = -1;

static unsigned char buffer[BUFF_SIZE];

/**
 * Init RM Board
 * @return
 */
int sbus_init() {
    termios tio;
    int i;

    for ( i = 0; i < 24; i ++ ) {
        serv_curr.s[i] = PWM_CENT;
        serv_next.s[i] = PWM_CENT;
    }

    if ( sbus_fd < 0 ) {
        printf("sbus_init\n");
        sbus_fd = open(SBUS_PORT, O_RDWR);
        if ( sbus_fd < 0 ) {
            PRINTF("SBUS OpenError\n") ;
            return -1;
        }
        memset(&tio, 0, sizeof(tio));
        tio.c_cflag = CS8 | CLOCAL | CREAD;
        tio.c_cc[VTIME] = 100;
        cfsetospeed(&tio, SBUS_BAUD_RATE);
        cfsetispeed(&tio, SBUS_BAUD_RATE);
        tcsetattr( sbus_fd, TCSANOW, &tio );
    }

  return sbus_reset();
}

/**
 * Reset RM Board
 * @return
 */
int sbus_reset() {
    int len = -1;
    int time_out = SBUS_TIMEOUT1;

    if ( sbus_fd < 0 ) {
        PRINTF("SBUS OpenError\n");
        return -1;
    }

    buffer[0] = 0x00;
    buffer[1] = 0x10;
    write( sbus_fd, buffer, 2 );

    PRINTF("RESET BOARD : ");
    while ( time_out -- ) {
        len = read( sbus_fd, buffer, 1);
        if ( len > 0 )
            break;
    }

    if ( len == 1 )
        PRINTF("ON\n");
    else if ( len < 0 ) {
        PRINTF("NG:DEVICE CONNECT ERROR)\n");
        return -1;
    } else {
        PRINTF("NG:UNKNOWN ERROR\n");
        return -1;
    }
    return 0;
}

/**
 * Set PWM Signal
 * @return
 */
int sbus_setpwm() {
    int len = -1;
    int time_out = SBUS_TIMEOUT1;

    if ( sbus_fd < 0 ) {
        PRINTF("SBUS OpenError\n");
        return -1;
    }

    buffer[0] = 0x00;
    buffer[1] = 0x20;
    buffer[2] = 0x00;
    write( sbus_fd, buffer, 3 );

    while ( time_out -- ) {
        len = read( sbus_fd, buffer, 1 );
        if ( len > 0 )
            break;
    }

    if ( buffer[0] != SBUS_ACK ) {
        PRINTF("START PWM : NG");
        return -1;
    }

    return 0;
}

/**
 * Get Temporary Width from all servomotors
 * @return
 */
int sbus_getwidth() {

    int len = -1;
    int i, j;
    int time_out = SBUS_TIMEOUT1;

    if ( sbus_fd < 0 ) {
        PRINTF("SBUS OpenError\n");
        return -1;
    }

    buffer[0] = 0x10;
    buffer[1] = 0x01;
    write( sbus_fd, buffer, 2 );

    while ( time_out -- ) {
        len = read(sbus_fd, buffer, BUFF_SIZE);
        if (len > 2)
            break;
    }

    if ( buffer[0] != SBUS_ACK ) {
        PRINTF("GET PWM VALUE : NG\n");
        return -1;
    }

    for ( i = 1, j = 0; i < 48; i += 2, j ++ ) {
        unsigned short value = ((unsigned short)((int)buffer[i] << 8 | (int)buffer[i + 1]));
        value += balanced * serv_balance[j];
        serv_curr.s[j] = value;
    }
    return 0;
}

/**
 * Set Initial width to all servomotors
 * @return
 */
int sbus_setwidth() {
    int len = -1;
    int i, j, csum;
    int time_out = SBUS_TIMEOUT1;

    if ( sbus_fd < 0 ) {
        PRINTF("SBUS OpenError\n");
        return -1;
    }
    buffer[0] = 0x10;
    buffer[1] = 0x00;
    for ( i = 0, j = 2; i < 24 ; i ++, j += 2 ) {
        unsigned short value = serv_curr.s[i];
        value -= balanced * serv_balance[i];
        value = value > PWM_MAX ? PWM_MAX : value;
        value = value < PWM_MIN ? PWM_MIN : value;
        buffer[j]   = (unsigned char) (value >> 8);
        buffer[j+1] = (unsigned char) value;
    }
    buffer[62] = 0xff;
    csum = 0;
    for ( i = 2; i < 63; i ++ ) {
        csum += buffer[i];
    }
    buffer[63] = (unsigned char)csum;
    write( sbus_fd, buffer, 64 );
    while ( time_out -- ) {
        len = read( sbus_fd, buffer, 1 );
        if ( len > 0 ) {
            break;
        }
    }
    if ( buffer[0] != SBUS_ACK ) {
        PRINTF("SET PWM WIDTH : NG\n");
        return -1;
    }
    return 0;
}

/**
 * Use initial position or not
 * @param value
 */
void sbus_balance( int value ) {
    int i;

    balanced = value ? 1 : 0;
    for ( i = 0; i < 24; i ++ ) {
        sbus_setserv( i, 0 );
    }
}

/**
 * Get angle value from num-th servomotor
 * @param num
 * @return
 */
int sbus_getserv( int num ) {
    int value;

    if ( num < 0 || num >= 24 ) {
        PRINTF("GETSERV: INVALID NUM\n");
        return 0;
    }
    value = serv_curr.s[num];
    value -= PWM_CENT;
    value *= serv_dir[num];
    return value;
}

/**
 * Include servomotor in party
 * @param num
 * @param value
 * @return
 */
int sbus_addserv( int num, int value ) {
    if ( num < 0 || num >= 24 ) {
        PRINTF("ADDSERV: INVALID NUM\n");
        return 0;
    }
    value += serv_curr.s[num];
    serv_curr.s[num] = value;
    return (value - PWM_CENT) * serv_dir[num];
}

/**
 * Set angle value to num-th servomotor
 * @param num
 * @param value
 */
void sbus_setserv( int num, int value ) {
    if ( num < 0 || num >= 24 ) {
        PRINTF("SETSERV: INVALID NUM\n");
        return;
    }
    value *= serv_dir[num];
    value += PWM_CENT;
    serv_curr.s[num] = value;
}

/**
 * Print value of every servomotor
 */
void sbus_printserv() {
    int i;

    for ( i = 0; i < 24; i ++ ) {
        unsigned short value = serv_curr.s[i];
        int real = value + balanced * serv_balance[i];
        printf( "serv[%d] = %d (%d)\n", i, (int) value, real );
    }
    printf( "\n" );
}

/**
 * Prepare robot for movement
 */
void sbus_moveinit() {

    printf("SBUS INIT :");
    if ( sbus_init() != 0 ) {
        printf("NG\n");
        exit(1);
    }
    printf("OK\n");
    sbus_reset();
    sbus_getwidth();
    sbus_balance(1);
    sbus_setpwm();
    sbus_setwidth();
    sbus_printserv();
}

/**
 * Prepare arrays for move action
 */
void sbus_initnext() {
    int i;

    for ( i = 0; i < 24; i ++ ) {
        serv_next.s[i] = serv_curr.s[i];
    }
}

/**
 * Get angle value from num-th servomotor
 * @param num
 * @return
 */
int sbus_getnext( int num ) {
    int value;

    if ( num < 0 || num >= 24 ) {
        PRINTF("GETNEXT: INVALID NUM\n");
        return 0;
    }
    value = serv_next.s[num];
    value -= PWM_CENT;
    value *= serv_dir[num];
    return value;
}

/**
 * Set next angle to num-th servomotor
 * @param num
 * @param value
 */
void sbus_setnext( int num, int value ) {
    if ( num < 0 || num >= 24 ) {
        PRINTF("SETNEXT: INVALID NUM\n");
        return;
    }
    value *= serv_dir[num];
    value += PWM_CENT;
    serv_next.s[num] = value;
}

/**
 * Do movement to setted position
 * @param speed
 */
void sbus_movesync( int speed ) {
    int i, j;
    int deltas[24];
    int max = 1, count;

    for ( i = 0; i < 24; i ++ ) {
        int delta = (int) serv_next.s[i] - (int) serv_curr.s[i];
        int absdelta = delta > 0 ? delta : - delta;
        deltas[i] = delta;
        max = max > absdelta ? max : absdelta;
    }
    count = (max - 1) / speed;
    if ( count > 0 ) {
        for ( i = 0; i < 24; i ++ ) {
            deltas[i] /= count + 1;
        }
        do {
            for ( i = 0; i < 24; i ++ ) {
                sbus_addserv( i, deltas[i] );
            }
            sbus_setwidth();
        } while ( -- count );
    }
    for ( i = 0; i < 24; i ++ ) {
        serv_curr.s[i] = serv_next.s[i];
    }
    sbus_setwidth();
}

/**
 * Print the values of nextone position
 */
void sbus_printnext() {
    int i;

    for ( i = 0; i < 24; i ++ ) {
        printf( "%d ", (int) serv_next.s[i] - PWM_CENT );
    }
    printf( "\n" );
}

/**
 * Get AD value from nth output pin
 * @param num
 * @return
 */
unsigned short sbus_getad(int num) {
    //printf("sbus_getad(%d)", num);
    int len = -1;
    int time_out = SBUS_TIMEOUT1;

    if (sbus_fd < 0) {
        PRINTF("SBUS OpenError\n");
        return 0;
    }
    if (num < 0 || num > 7) {
        PRINTF("NG: PORT\n");
        return 0;
    }
    buffer[0] = 0x20;
    buffer[1] = (((unsigned short) (num)) << 4) | 0x01;

    write(sbus_fd, buffer, 2);

    while (time_out--) {
        len = read(sbus_fd, buffer, BUFF_SIZE);
        if (len > 2) {
            break;
        }
    }
    if (buffer[0] == SBUS_ACK) {
    } else {
        PRINTF("GET AD : NG\n");
        return 0;
    }
    return ((unsigned short) ((int) buffer[1] << 8 | (int) buffer[2]));
}


