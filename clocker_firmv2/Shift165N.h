#ifndef sh165n
#define sh165n

// ShIFT REGISTER 165 N BTN HANDLERS PIN
#define BT_PL 26
#define BT_CLK 25
#define BT_CKEN 33
#define BT_QH 34

#define SH_CHP 1 // number of 165n shift registers
#define PULSE_WIDTH_USEC   5 // interval pulse load latch


class Shift165N {
  private:
    uint8_t oldVal = 0;
    uint8_t newVal = 0;

  public:
    void shInit() {
      pinMode(BT_PL, OUTPUT);
      pinMode(BT_CKEN, OUTPUT);
      pinMode(BT_CLK, OUTPUT);
      pinMode(BT_QH, INPUT);

      digitalWrite(BT_CLK, LOW);
      digitalWrite(BT_PL, HIGH);
    }

    // updates the newVal
    void readNow() {
      // Trigger a parallel Load to latch the state of the data lines
      digitalWrite(BT_CKEN, HIGH);
      digitalWrite(BT_PL, LOW);
      delayMicroseconds(PULSE_WIDTH_USEC);
      digitalWrite(BT_PL, HIGH);
      digitalWrite(BT_CKEN, LOW);

      // Get States
      this->newVal = shiftIn(BT_QH, BT_CLK, LSBFIRST);
    }

    /**
       Updates the Old Val
    */
    void updateOldVal() {
     this->oldVal = this->newVal;
    }

    uint8_t getOldVal(){
      return this->oldVal;
    }

    uint8_t getNewVal(){
      return this->newVal;
    }

    uint8_t evaluateBits(uint8_t bitIdx){
      if(bitRead(this->oldVal, 7 - bitIdx) == 0 && bitRead(this->newVal, 7 - bitIdx) == 1){
        // previous states 0 then goes to 1 -> Locking States
        return 0;
      } else if(bitRead(this->oldVal, 7 - bitIdx) == 1 && bitRead(this->newVal, 7 - bitIdx) == 0){
        // previous states 1 then goes to 0 -> Opening State
        return 1;
      }
      return -1; // no changes
    }

    bool areCellsClosed(){
      return this->newVal == 248;
    }

    uint8_t getBitValue(uint8_t fromVal, uint8_t bitIdx){
      return bitRead(fromVal, 7 - bitIdx);
    }

    // TODO CREATE A LISTENER TASK
};

#endif
