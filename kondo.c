/*
 * Library for moving Kondo robot
 * Version 0.1  2016-05-21 Sladjan Kantar (School of Computing)
 *
 *
 *		kondo.c
 */

/**
 * Make one's bow
 */
void bow(){
    int speed = 200;

    sbus_initnext();
    sbus_setnext( 18, 60 * PWM_DEG );   //groin
    sbus_setnext( 12, 60 * PWM_DEG );   //groin
    sbus_setnext( 19, 40 * PWM_DEG );   //knee
    sbus_setnext( 13, 40 * PWM_DEG );   //knee
    sbus_setnext( 6, -70 * PWM_DEG );   //shoulder
    sbus_movesync( speed );

    sbus_initnext();
    sbus_setnext( 18, 80 * PWM_DEG );   //groin
    sbus_setnext( 12, 80 * PWM_DEG );   //groin
    sbus_setnext( 8, -70 * PWM_DEG );   //elbow
    sbus_movesync( speed );

    sleep(2);

    sbus_initnext();
    sbus_setnext( 18, 60 * PWM_DEG );   //groin
    sbus_setnext( 12, 60 * PWM_DEG );   //groin
    sbus_setnext( 8, 0 * PWM_DEG );     //elbow
    sbus_movesync( speed );

    sbus_initnext();
    sbus_setnext( 18, 0 * PWM_DEG );    //groin
    sbus_setnext( 12, 0 * PWM_DEG );    //groin
    sbus_setnext( 19, 0 * PWM_DEG );    //knee
    sbus_setnext( 13, 0 * PWM_DEG );    //knee
    sbus_setnext( 6, 0 * PWM_DEG );     //shoulder
    sbus_movesync( speed );
}

/**
 * Denial of head
 */
void denial(){
    sbus_initnext();
    sbus_setnext( 0, 45 * PWM_DEG );   //head
    sbus_movesync( speed );

    sbus_initnext();
    sbus_setnext( 0, -45 * PWM_DEG );   //head
    sbus_movesync( speed );

    sbus_initnext();
    sbus_setnext( 0, 45 * PWM_DEG );   //head
    sbus_movesync( speed );

    sbus_initnext();
    sbus_setnext( 0, -45 * PWM_DEG );   //head
    sbus_movesync( speed );

    sbus_initnext();
    sbus_setnext( 0, 0 * PWM_DEG );     //head
    sbus_movesync( speed );
}

/**
 * Make an forward step
 */
void step_forward(){

}

/**
 * Make an backward step
 */
void step_backward(){

}

/**
 * Turn to the left
 */
void rotate_left(){

}

/**
 * Turn to the right
 */
void rotate_right(){

}