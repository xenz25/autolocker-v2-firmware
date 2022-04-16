#include "Zervo.h"
#include "Demux38.h"
#include "Shift165N.h"

Zervo zv;
Demux38 zv_addresser = Demux38(5, 18, 19, 0);
Shift165N btnHandler;

// SERVO
#define PPWM 27
#define PWMCH 15
#define CL_DEG 50
#define OP_DEG 0
#define ADDR_0 5
#define ADDR_1 18
#define ADDR_2 19

// this function is under the TaskCallBack
bool compare(const char *al1, const char *);

uint8_t detectorValue = 0; // use in sensing little changes in device door states

void initLocks() {
  zv_addresser.demuxInit();
  zv.zAttach(PPWM, 15);
  btnHandler.shInit(); // for switches

  btnHandler.readNow();
  btnHandler.updateOldVal();
  detectorValue = btnHandler.getOldVal();
  // check locks one by one and initiate instruction for closing
}

bool svLock(uint8_t sv_addr) {
  zv_addresser.turnToAddress(sv_addr);
  zv.writeDegree(CL_DEG);
  return true;
}

bool svUnlock(uint8_t sv_addr) {
  zv_addresser.turnToAddress(sv_addr);
  zv.writeDegree(OP_DEG);
  return true;
}

void updateActiveCellIndex(const char * cell_number) {
  if (compare(cell_number, "1")) {
    ACTIVE_CELL_INDEX = 0;
  } else if (compare(cell_number, "2")) {
    ACTIVE_CELL_INDEX = 1;
  } else if (compare(cell_number, "3")) {
    ACTIVE_CELL_INDEX = 2;
  } else if (compare(cell_number, "4")) {
    ACTIVE_CELL_INDEX = 3;
  } else if (compare(cell_number, "5")) {
    ACTIVE_CELL_INDEX = 4;
  } else {
    //default
    ACTIVE_CELL_INDEX = NON_SELECTABLE_ADDRESS;
  }
}

// TODO TASK FOR VERIFICATION
bool lockStateManager(uint8_t addr, uint8_t evaluation) {
  bool actionState = false;
  switch (evaluation) {
    case 0:
      actionState = svLock(addr);
      break;
    case 1:
      actionState = svUnlock(addr);
      break;
    default:
      //      Serial.println("DEFAULT CATCH");
      break;
  }
  return actionState;
}
