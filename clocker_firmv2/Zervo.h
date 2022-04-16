#ifndef zvo
#define zvo

/*
   LEDC Chan to Group/Channel/Timer Mapping
** ledc: 0  => Group: 0, Channel: 0, Timer: 0
** ledc: 1  => Group: 0, Channel: 1, Timer: 0
** ledc: 2  => Group: 0, Channel: 2, Timer: 1
** ledc: 3  => Group: 0, Channel: 3, Timer: 1
** ledc: 4  => Group: 0, Channel: 4, Timer: 2
** ledc: 5  => Group: 0, Channel: 5, Timer: 2
** ledc: 6  => Group: 0, Channel: 6, Timer: 3
** ledc: 7  => Group: 0, Channel: 7, Timer: 3
** ledc: 8  => Group: 1, Channel: 0, Timer: 0
** ledc: 9  => Group: 1, Channel: 1, Timer: 0
** ledc: 10 => Group: 1, Channel: 2, Timer: 1
** ledc: 11 => Group: 1, Channel: 3, Timer: 1
** ledc: 12 => Group: 1, Channel: 4, Timer: 2
** ledc: 13 => Group: 1, Channel: 5, Timer: 2
** ledc: 14 => Group: 1, Channel: 6, Timer: 3
** ledc: 15 => Group: 1, Channel: 7, Timer: 3
*/

class Zervo {
  private:
    uint8_t pin;
    uint8_t channel;
    const uint8_t TIMER_DEPTH = 16; // 16 bit
    const uint8_t BASE_FREQ = 50; //50 hz

  public:
    //wont handle channel attach check for simplicity
    void zAttach(uint8_t pin, uint8_t channel) {
      this->pin = pin;
      this->channel = channel;
      ledcSetup(this->channel, 50, 16); // channel X, 50 Hz, 16-bit depth
      ledcAttachPin(this->pin, this->channel);
    }

    void writeDegree(int deg) {
      deg = constrain(deg, 0, 180);
      /*
         0 - minAngle , 180 - maxAgle, 544 - minTurnOnTime, 2400 - maxTurnOnTime
         adjust 544 and 2400 to reach desired angle
      */
      int tOn = map(deg, 0, 180, 544, 2400);
      // 20000 is the signal PERIOD in us -> 20ms at 50 hz with 10 percent duty cycle
      ledcWrite(this->channel, map(tOn, 0, 20000, 0, (1 << TIMER_DEPTH) - 1));
    }
};

#endif
