#define SBUS_ACK	0x06
#define SBUS_NAK	0x15
#define PWM_DEG		((double)(14008 / 90))
#define SBUS_TIMEOUT1	20000
#define ASPEED_MAX	PWM_DEG

/** Init RM Board **/
int sbus_init();

/** Reset RM Board **/
int sbus_reset();

/** Set PWM Signal **/
int sbus_setpwm();

/** Get Temporary Width from all servomotors **/
int sbus_getwidth();

/** Set Initial width to all servomotors **/
int sbus_setwidth();

/** Set angle value to num-th servomotor **/
void sbus_setserv( int num, int value );

/** Get angle value from num-th servomotor **/
int sbus_getserv( int num );

/** Include servomotor in party **/
int sbus_addserv( int num, int value );

/** Print value of every servomotor **/
void sbus_printserv();

/** Prepare arrays for move action **/
void sbus_initnext();

/** Set next angle to num-th servomotor **/
void sbus_setnext( int num, int value );

/** Get angle value from num-th servomotor **/
int sbus_getnext( int num );

/** Do movement to setted position **/
void sbus_movesync( int speed );

/** Print the values of nextone position **/
void sbus_printnext();

/** Get AD value from nth output pin  **/
unsigned short sbus_getad(int num);

/** Use initial position or not  **/
void sbus_balance( int value );

/** Prepare robot for movement  **/
void sbus_moveinit();
