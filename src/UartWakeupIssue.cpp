#include <Particle.h>
#include <functional>

SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

Serial1LogHandler sLogHandler(115200, LOG_LEVEL_INFO);

class UartCli {
  public:
    UartCli(Stream& pStream, std::function<void(const char*)> pCmdHanlder):mStream(pStream), mCmdHanlder(pCmdHanlder){}

    void consumeStream() {
      while(mStream.available()) {
        char c = mStream.read();
        if(c == '\r' || c == '\n' ) {
          if(mBufferIndex != 0 ) {
              mCmdHanlder(mBuffer.data());
          }
          mBufferIndex = 0;
        } else {
          mBuffer[mBufferIndex++] = c;
          if(mBufferIndex >= mBuffer.size()) {
              Log.error("Uart cli buffer full! => flush");
              mBufferIndex = 0;
          }
        }
        mBuffer[mBufferIndex] = 0;
      }
    }

  private:
    static constexpr size_t BUFFER_SIZE = 256;

    Stream& mStream;
    std::function<void(const char*)> mCmdHanlder;
    size_t mBufferIndex;
    std::array<char,BUFFER_SIZE> mBuffer;
};

UartCli sUartCli(Serial1, [](const char* pCmd){
  Log.info("Uart cli handle command => '%s'", pCmd);
  if(strcmp(pCmd,"dfu")==0) {
    System.dfu();
  }
});

constexpr pin_t BLUE_LED = D7;

constexpr system_tick_t stayAwakeForCommands = 5 * 1000;
system_tick_t sUartWakeupTime;
int sCounter = 0;

void setup() {
  pinMode(BLUE_LED, OUTPUT);
  sUartWakeupTime = millis() - stayAwakeForCommands;
}

void loop() {
  if (millis() - sUartWakeupTime >= stayAwakeForCommands) {
    Log.info("Go to sleep [%d]", sCounter++);
    SystemSleepConfiguration config;
    config
      .mode(SystemSleepMode::STOP)
      .duration(10s)
      .usart(Serial1);
    digitalWrite(BLUE_LED, HIGH);
    auto result = System.sleep(config);
    digitalWrite(BLUE_LED, LOW);
    auto reason = result.wakeupReason();
    if(reason == SystemSleepWakeupReason::BY_USART) {
      sUartWakeupTime = millis();  
    }
  }
  sUartCli.consumeStream();
}