#ifndef dmux38
#define dmux38

class Demux38 {
  private:
    uint8_t addrPins[3] = {0, 0, 0};
    uint8_t homeAddr;
    uint8_t addrPinsz = sizeof(addrPins) / sizeof(addrPins[0]);

  public:
    //params: addr0, addr1, addr2, homeAddr
    Demux38(uint8_t addr0, uint8_t addr1, uint8_t addr2, uint8_t homeAddr) {
      addrPins[0] = addr0;
      addrPins[1] = addr1;
      addrPins[2] = addr2;
      this->homeAddr = homeAddr;
    }

    //initialize demux in setup
    void demuxInit() {
      for (int i = 0; i < addrPinsz; i++){
        pinMode(addrPins[i], OUTPUT);
      }
      turnToAddress(homeAddr);
    }

    //activate address
    bool turnToAddress(uint8_t addr) {
      for (uint8_t i = 0; i < addrPinsz; i++) {
        digitalWrite(addrPins[i], (addr >> i) & 1);
      }
      return true;
    }
};

#endif
