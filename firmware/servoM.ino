void write(int channel, int angle ){
 int pulse = map(angle  , 0, 180, SERVO_MIN, SERVO_MAX);
    pwm.setPWM(channel, 0, pulse);
}
