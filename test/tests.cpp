// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <stdexcept>
#include "TimedDoor.h"

using ::testing::_;
using ::testing::Invoke;
using ::testing::NiceMock;

class MockTimer : public Timer {
 public:
  MOCK_METHOD(void, tregister, (int delay, TimerClient * client), (override));
};

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class TimedDoorTest : public ::testing::Test {
 protected:
  void SetUp() override {
    mock_timer_ = std::make_unique<NiceMock<MockTimer>>();
    door_ = std::make_unique<TimedDoor>(100);
    door_->setTimer(mock_timer_.get());
  }

  void TearDown() override {
    door_.reset();
    mock_timer_.reset();
  }

  std::unique_ptr<NiceMock<MockTimer>> mock_timer_;
  std::unique_ptr<TimedDoor> door_;
};

TEST_F(TimedDoorTest, DoorStartsClosed) {
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, LockKeepsDoorClosed) {
  door_->lock();
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
  EXPECT_CALL(*mock_timer_, tregister(_, _)).Times(1);
  door_->unlock();
  EXPECT_TRUE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsConstructorValue) {
  EXPECT_EQ(door_->getTimeOut(), 100);
}

TEST_F(TimedDoorTest, UnlockPassesTimeoutAndAdapterToTimer) {
  EXPECT_CALL(*mock_timer_, tregister(100, _))
      .Times(1)
      .WillOnce(::testing::Return());
  door_->unlock();
}

TEST_F(TimedDoorTest, TimeoutWhileDoorOpenThrows) {
  EXPECT_CALL(*mock_timer_, tregister(_, _))
      .WillOnce(Invoke([](int, TimerClient * c) {
        ASSERT_NE(c, nullptr);
        c->Timeout();
      }));
  EXPECT_THROW(door_->unlock(), std::runtime_error);
}

TEST_F(TimedDoorTest, TimeoutAfterDoorClosedDoesNotThrow) {
  EXPECT_CALL(*mock_timer_, tregister(_, _))
      .WillOnce(Invoke([this](int, TimerClient * c) {
        door_->lock();
        ASSERT_NE(c, nullptr);
        c->Timeout();
      }));
  EXPECT_NO_THROW(door_->unlock());
  EXPECT_FALSE(door_->isDoorOpened());
}

TEST_F(TimedDoorTest, ThrowStateThrowsWhenDoorOpen) {
  door_->unlock();
  EXPECT_THROW(door_->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, ThrowStateNoThrowWhenDoorClosed) {
  door_->lock();
  EXPECT_NO_THROW(door_->throwState());
}

TEST_F(TimedDoorTest, UnlockWithoutTimerSkipsRegister) {
  auto door = std::make_unique<TimedDoor>(50);
  EXPECT_NO_THROW(door->unlock());
  EXPECT_TRUE(door->isDoorOpened());
}

TEST(TimerIntegrationTest, TRegisterEventuallyCallsClientTimeout) {
  NiceMock<MockTimerClient> client;
  EXPECT_CALL(client, Timeout()).Times(1);
  Timer timer;
  timer.tregister(0, &client);
}

TEST(MockDoorInterfaceTest, DoorInterfaceMethodsAreMockable) {
  NiceMock<MockDoor> door;
  EXPECT_CALL(door, lock()).Times(1);
  EXPECT_CALL(door, unlock()).Times(1);
  EXPECT_CALL(door, isDoorOpened())
      .WillOnce(::testing::Return(false))
      .WillOnce(::testing::Return(true));
  door.lock();
  door.unlock();
  EXPECT_FALSE(door.isDoorOpened());
  EXPECT_TRUE(door.isDoorOpened());
}
