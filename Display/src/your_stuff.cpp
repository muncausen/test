//#include <chrono>
#include "your_stuff.hpp"

#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>
#include <thread>

#include "can_common.hpp"
#include "candb.hpp"

void yourStuff::YouHaveJustRecievedACANFrame(const canfd_frame *const _frame) {
  DisplayCanFrame display_can_frame;

  switch (_frame->can_id) {
    case kDisplayCanFrameId: {
      memcpy(&display_can_frame, _frame->data, sizeof(display_can_frame));

      this->InstrumentCluster.ignite(display_can_frame.ignition);
      this->InstrumentCluster.setRPM(display_can_frame.rpm);
      this->InstrumentCluster.setSpeed(display_can_frame.speed);
      this->InstrumentCluster.setGearPindle_int(display_can_frame.gear_select);
      this->InstrumentCluster.setGear(display_can_frame.automatic_gear);

      struct _icons icons {};
      switch (display_can_frame.turn_indicator) {
        case static_cast<unsigned int>(TurnIndicator::kRight):
          icons.right_blinker = 1;
          icons.left_blinker = 0;
          break;

        case static_cast<unsigned int>(TurnIndicator::kLeft):
          icons.right_blinker = 0;
          icons.left_blinker = 1;
          break;

        case static_cast<unsigned int>(TurnIndicator::kHazard):
          icons.hazard = 1;
          break;

        case static_cast<unsigned int>(TurnIndicator::kOff):
          icons.right_blinker = 0;
          icons.left_blinker = 0;
          icons.hazard = 0;
          break;

        default:
          break;
      }
      this->InstrumentCluster.setIcon(&icons);

    } break;

    default:
      break;
  }
}

/******************************* ANYTHING BELOW THIS LINE IS JUST BORING STUFF *******************************/

yourStuff::yourStuff(const std::string &_ifName, QObject *_vs) {
  if (!(this->CANReader.open(_ifName))) exit(-1);  // emit die();

  this->InstrumentCluster.init(_vs);

  this->startTimer(1);
}

bool yourStuff::run() {
  bool ret = true;
  canfd_frame frame;
  CANOpener::ReadStatus status = this->CANReader.read(&frame);

  if (status == CANOpener::ReadStatus::OK) {
    this->YouHaveJustRecievedACANFrame(&frame);
  }

  if (frame.can_id == kShutdownCanFrameId) {
    ret = false;
  }

  if (status == CANOpener::ReadStatus::ERROR)
    ret = false;
  else if (status == CANOpener::ReadStatus::NAVAL || status == CANOpener::ReadStatus::ENDOF) {
    this->Counter++;
  } else {
    this->Counter = 0;
  }

  // if (this->Counter > 2000) {
  //   ret = false;
  // }

  return ret;
}

void yourStuff::timerEvent(QTimerEvent *) {
  if (this->run())
    ;
  else {
    //    emit this->die();
    exit(-1);
  }
}
