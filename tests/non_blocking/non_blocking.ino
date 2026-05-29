#include <ok_logging.h>

OkLoggingContext OK_CONTEXT("nb");

// A small ring-buffer Print sink so we can apply real backpressure.
// availableForWrite() reports the remaining slots; write() drops bytes when
// full, mimicking a saturated UART/USB CDC TX path.
class RingPrint : public Print {
 public:
  static constexpr int CAPACITY = 200;
  uint8_t buf[CAPACITY];
  int head = 0, tail = 0;
  int dropped = 0;

  int filled() const { return (head - tail + CAPACITY) % CAPACITY; }

  size_t write(uint8_t b) override {
    int const next = (head + 1) % CAPACITY;
    if (next == tail) { ++dropped; return 0; }
    buf[head] = b;
    head = next;
    return 1;
  }

  int availableForWrite() override { return CAPACITY - 1 - filled(); }

  void drain_to(Print& out) {
    while (tail != head) {
      out.write(buf[tail]);
      tail = (tail + 1) % CAPACITY;
    }
  }
};

static RingPrint ring;

void setup() {
  Serial.begin(115200);
  Serial.println("BEGIN-TEST");

  ok_logging_stream = &ring;
  ok_logging_non_blocking = true;

  // Five notes fit; 6th overflows and should print "BUF FULL" (avail at
  // that point ~39 bytes, enough for the marker). 7th is silently dropped
  // (marker only prints once per drop run). Each message is sized so the
  // ring buffer hits the drop condition exactly on message #6.
  OK_NOTE("padded msg number 1");
  OK_NOTE("padded msg number 2");
  OK_NOTE("padded msg number 3");
  OK_NOTE("padded msg number 4");
  OK_NOTE("padded msg number 5");
  OK_NOTE("padded msg number 6");
  OK_NOTE("padded msg number 7");

  // Drain the captured buffer to the real serial port for the test to see.
  ok_logging_stream = &Serial;
  ok_logging_non_blocking = false;
  Serial.println("---CAPTURED---");
  ring.drain_to(Serial);
  Serial.println("---END---");
  Serial.printf("dropped_bytes=%d\n", ring.dropped);

  // After full drain, new messages should pass through again.
  ok_logging_stream = &ring;
  ok_logging_non_blocking = true;
  OK_NOTE("after drain");

  ok_logging_stream = &Serial;
  ok_logging_non_blocking = false;
  Serial.println("---CAPTURED-2---");
  ring.drain_to(Serial);
  Serial.println("---END-2---");

  OK_FATAL("end of test");
}

void loop() {
  Serial.println("TEST-FAIL");
  delay(1000);
}
