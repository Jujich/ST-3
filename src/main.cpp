// Copyright 2021 GHA Test Team
#include "TimedDoor.h"
#include <iostream>

int main() {
  Timer timer;
  TimedDoor tDoor(5);
  tDoor.setTimer(&timer);
  tDoor.lock();
  tDoor.unlock();

  return 0;
}
