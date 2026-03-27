// Copyright 2021 GHA Test Team

#ifndef INCLUDE_TIMEDDOOR_H_
#define INCLUDE_TIMEDDOOR_H_

#include <exception>
#include <thread>
#include <atomic>

class DoorTimerAdapter;
class Timer;
class Door;
class TimedDoor;

class TimerClient {
 public:
  virtual void Timeout() = 0;
};

class Door {
 public:
  virtual void lock() = 0;
  virtual void unlock() = 0;
  virtual bool isDoorOpened() = 0;
};

class DoorTimerAdapter : public TimerClient {
 private:
  TimedDoor& door;
 public:
  explicit DoorTimerAdapter(TimedDoor&);
  void Timeout();
};

class Timer {
 private:
  TimerClient *client;
  std::thread worker;
  std::atomic<bool> running;
  std::exception_ptr eptr;
  void sleep(int);
 public:
  Timer();
  ~Timer();
  void tregister(int, TimerClient*);
  void stop();
};

class TimedDoor : public Door {
 private:
  DoorTimerAdapter * adapter;
  Timer timer;
  int iTimeout;
  bool isOpened;
 public:
  explicit TimedDoor(int);
  ~TimedDoor();
  bool isDoorOpened();
  void unlock();
  void lock();
  int  getTimeOut() const;
  void throwState();
};

#endif  // INCLUDE_TIMEDDOOR_H_
