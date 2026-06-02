// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <thread>
#include <atomic>
#include <chrono>
#include "TimedDoor.h"

using ::testing::Return;
using ::testing::_;

// ================= Mock Classes =================

class DoorMock : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimerClientMock : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

// ================= Test Fixtures =================

class TimedDoorInterfaceTest : public ::testing::Test {
 protected:
  DoorMock mock_door;
};

class TimedDoorTest : public ::testing::Test {
 protected:
  TimedDoor* test_door;

  void SetUp() override {
    test_door = new TimedDoor(2);
  }

  void TearDown() override {
    delete test_door;
  }
};

// ================= Interface Mocks Tests =================

TEST_F(TimedDoorInterfaceTest, testLock) {
  EXPECT_CALL(mock_door, lock()).Times(1);
  EXPECT_CALL(mock_door, isDoorOpened()).WillRepeatedly(Return(false));

  mock_door.lock();
  EXPECT_FALSE(mock_door.isDoorOpened());
}

TEST_F(TimedDoorInterfaceTest, testUnlock) {
  EXPECT_CALL(mock_door, unlock()).Times(1);
  EXPECT_CALL(mock_door, isDoorOpened()).WillRepeatedly(Return(true));

  mock_door.unlock();
  EXPECT_TRUE(mock_door.isDoorOpened());
}

TEST_F(TimedDoorInterfaceTest, testIsOpened) {
  EXPECT_CALL(mock_door, isDoorOpened())
      .Times(2)
      .WillRepeatedly(Return(true));

  EXPECT_TRUE(mock_door.isDoorOpened());
  EXPECT_TRUE(mock_door.isDoorOpened());
}

// ================= TimedDoor State Tests =================

TEST_F(TimedDoorTest, testInitialState) {
  EXPECT_FALSE(test_door->isDoorOpened());
}

TEST_F(TimedDoorTest, testUnlockOpens) {
  test_door->unlock();
  EXPECT_TRUE(test_door->isDoorOpened());
}

TEST_F(TimedDoorTest, testLockCloses) {
  test_door->unlock();
  test_door->lock();
  EXPECT_FALSE(test_door->isDoorOpened());
}

TEST_F(TimedDoorTest, testNoTimeoutIfClosed) {
  test_door->unlock();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  test_door->lock();

  std::this_thread::sleep_for(std::chrono::seconds(2));
  SUCCEED();
}

TEST_F(TimedDoorTest, testTimerReset) {
  test_door->unlock();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  test_door->lock();
  test_door->unlock();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  test_door->lock();
  SUCCEED();
}

// ================= DoorTimerAdapter Tests =================

TEST(DoorAdapterTest, testExceptionIfOpen) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.unlock();
  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(DoorAdapterTest, testNoExceptionIfClosed) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.lock();
  EXPECT_NO_THROW(adapter.Timeout());
}

// ================= Timer Class Tests =================

TEST(TimerTest, testTimeoutTrigger) {
  Timer timer;
  TimerClientMock mock_client;

  EXPECT_CALL(mock_client, Timeout()).Times(1);

  timer.tregister(1, &mock_client);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  timer.stop();
}

TEST(TimerTest, testStopTimer) {
  Timer timer;
  TimerClientMock mock_client;

  EXPECT_CALL(mock_client, Timeout()).Times(0);

  timer.tregister(2, &mock_client);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  timer.stop();

  std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST(TimerTest, testMultipleTimer) {
  Timer timer;
  TimerClientMock mock_client;

  EXPECT_CALL(mock_client, Timeout()).Times(1);
  timer.tregister(1, &mock_client);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  timer.stop();

  ::testing::Mock::VerifyAndClearExpectations(&mock_client);

  EXPECT_CALL(mock_client, Timeout()).Times(1);
  timer.tregister(1, &mock_client);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  timer.stop();
}

// ================= TimedDoor Extra Tests =================

TEST(TimedDoorExtraTest, testTimeoutValue) {
  TimedDoor door(5);
  EXPECT_EQ(door.getTimeOut(), 5);
}

TEST(TimedDoorExtraTest, testThrowState) {
  TimedDoor door(1);
  try {
    door.throwState();
    FAIL() << "Expected std::runtime_error to be thrown";
  } catch (const std::runtime_error& err) {
    EXPECT_STREQ(err.what(), "Timeout: the door has been left open!");
  } catch (...) {
    FAIL() << "Expected std::runtime_error, but caught another exception";
  }
}
