static constexpr int SOUND_VOLUME = 1;
static constexpr unsigned long RUN_TIME = 1000 * 600;
static constexpr unsigned long MOTOR_CONTROL_INTERVAL = 1000;
static constexpr unsigned long MOTOR_ON_TIME_SLOW = 30;
static constexpr unsigned long MOTOR_ON_TIME_FAST = 40;
static constexpr int HIT_COUNT_THRESHOLD = 3;

static constexpr uint8_t POWER_PIN = D2;
static constexpr uint8_t HIT_PIN = D1;
static constexpr uint8_t MOTOR_IN2_PIN = D10;
static constexpr uint8_t MOTOR_IN1_PIN = D9;

enum class State {
  Ready,
  Run,
  Finish,
};

static State State_ = State::Ready;
static int HitCount_ = 0;

void setup() {
  Serial.begin(115200);
  // analogReadResolution(14);

  PowerBegin();
  HitBegin();
  MotorBegin();
  SoundBegin();
  delay(500);

  SoundSetVolume(SOUND_VOLUME);
}

void loop() {
  switch (State_) {
    case State::Ready:
      TaskReady();
      break;
    case State::Run:
      TaskRun();
      break;
    case State::Finish:
      TaskFinish();
      break;
  }

  // // Analog out
  // for (int i = 0; i < 4096; ++i) {
  //   analogWrite(A0, i);
  //   delay(10);
  // }
}

static void TaskReady() {
  while (true) {
    if (PowerIsOn()) {
      State_ = State::Run;
      return;
    }

    delay(2);
  }
}

static void TaskRun() {
  SoundPlayByIndex(2);
  delay(2000);

  const auto start = millis();
  bool hit = false;
  HitCount_ = 0;
  while (true) {
    if (!PowerIsOn()) {
      State_ = State::Ready;
      return;
    }

    const auto elapsed = millis() - start;
    if (elapsed >= RUN_TIME) {
      digitalWrite(MOTOR_IN1_PIN, LOW);
      State_ = State::Finish;
      return;
    }

    const bool preHit = hit;
    hit = HitGettingSlammed();
    if (!preHit && hit) {
      ++HitCount_;
      SoundPlayByIndex(random(3, 6));
    }

    if ((elapsed % MOTOR_CONTROL_INTERVAL) < (HitCount_ < HIT_COUNT_THRESHOLD ? MOTOR_ON_TIME_SLOW : MOTOR_ON_TIME_FAST)) {
      digitalWrite(MOTOR_IN1_PIN, HIGH);
    } else {
      digitalWrite(MOTOR_IN1_PIN, LOW);
    }

    delay(2);
  }
}

static void TaskFinish() {
  SoundPlayByIndex(6);
  delay(5000);

  while (true) {
    if (!PowerIsOn()) {
      State_ = State::Ready;
      return;
    }

    delay(2);
  }
}

static void PowerBegin() {
  pinMode(POWER_PIN, INPUT_PULLUP);
}

static bool PowerIsOn() {
  return digitalRead(POWER_PIN) ? false : true;
}

static void HitBegin() {
  pinMode(HIT_PIN, INPUT_PULLUP);
}

static bool HitGettingSlammed() {
  return digitalRead(HIT_PIN) ? false : true;
}

static void MotorBegin() {
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);
  digitalWrite(MOTOR_IN1_PIN, LOW);
  digitalWrite(MOTOR_IN2_PIN, LOW);
}

static void SoundBegin() {
  Serial1.begin(115200);
}

static void SoundSetVolume(int volume) {
  if (volume < 0) volume = 0;
  if (volume > 31) volume = 31;

  char command[20];
  snprintf(command, sizeof(command), "AT+VOL=%d\r", volume);
  Serial1.write(command);
}

static void SoundPlayByIndex(int index) {
  char command[20];
  snprintf(command, sizeof(command), "AT+PLAY=sd0,%d\r", index);
  Serial1.write(command);
}
