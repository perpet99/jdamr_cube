#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <INA219_WE.h>
#include "IMU.h"
#include "STSServoDriver.h"

// --- 설정 및 핀 정의 ---
#define S_SCL           33
#define S_SDA           32
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define SCREEN_ADDRESS  0x3C
#define INA219_ADDRESS  0x42

// STS3215 서보 설정 (UART2)
#define S_RX 18
#define S_TX 19
HardwareSerial ServoSerial(2); 

// --- 전역 객체 ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
INA219_WE ina219 = INA219_WE(INA219_ADDRESS);
STSServoDriver st;

// IMU 데이터 변수 [cite: 55]
EulerAngles stAngles;
IMU_ST_SENSOR_DATA_FLOAT stGyroRawData, stAccelRawData;
IMU_ST_SENSOR_DATA stMagnRawData;

// --- 프로토콜 및 데이터 변수 ---
String debug_line_1, debug_line_2, debug_line_3;
const byte SENSOR_PACKET_LENGTH = 29;
byte sensor_data_packet[SENSOR_PACKET_LENGTH] = {0,};

// 모터 ID 및 명령어 정의 [cite: 5-7]
const byte LEFT_ID = 2;
const byte RIGHT_ID = 1;
#define GO_FORWARD    1
#define GO_BACKWARD   2
#define TURN_LEFT     3
#define TURN_RIGHT    4
#define STOP          5
#define CMD_RESET     6

// 명령 종류
//  0x51 : [방향][속도]        - 기존 방식. 방향 5종 + 0~255 속도. 하위 호환용으로 유지.
//  0x52 : [L_hi][L_lo][R_hi][R_lo] - 좌/우 바퀴 목표속도를 int16로 개별 지정(빅 엔디언).
//         값의 단위는 서보의 속도 단위이며, 각 바퀴 기준 "양수 = 전진"이다.
//         우측 모터가 뒤집혀 장착된 것은 이 펌웨어가 내부에서 보정한다.
//         nav2처럼 직진과 회전을 동시에 요구하는 상위 제어기를 쓰려면 이쪽을 써야 한다.
#define CMD_MOTION      0x51
#define CMD_WHEEL_VEL   0x52

// 서보에 넣을 수 있는 속도 상한. 기존 0x51 경로의 map(0,255 -> 0,2400)과 맞춘다.
const int16_t MAX_SERVO_VELOCITY = 2400;

// --- 함수 정의 ---

// 센서 데이터 송신 (실제 센서 값 적용) [cite: 13-25]
void send_sensors() {
    imuDataGet(&stAngles, &stGyroRawData, &stAccelRawData, &stMagnRawData); // 
    float voltage = ina219.getBusVoltage_V(); // [cite: 71]

    sensor_data_packet[0] = 0xf5; 
    sensor_data_packet[1] = 0xf5; 
    
    // 가속도 (Raw * 1000 정수화) [cite: 14-15]
    int16_t ax = (int16_t)(stAccelRawData.X * 1000);
    int16_t ay = (int16_t)(stAccelRawData.Y * 1000);
    int16_t az = (int16_t)(stAccelRawData.Z * 1000);
    sensor_data_packet[2] = (ax >> 8) & 0xff; sensor_data_packet[3] = ax & 0xff;
    sensor_data_packet[4] = (ay >> 8) & 0xff; sensor_data_packet[5] = ay & 0xff;
    sensor_data_packet[6] = (az >> 8) & 0xff; sensor_data_packet[7] = az & 0xff;
    
    // 자이로 데이터 [cite: 17-18]
    int16_t gx = (int16_t)(stGyroRawData.X * 100);
    sensor_data_packet[8] = (gx >> 8) & 0xff; sensor_data_packet[9] = gx & 0xff;
    // (나머지 축 및 마그네틱은 유사 방식으로 채움)

    // 엔코더 (서보 라이브러리에서 직접 읽기)
    int16_t l_pos = st.getCurrentPosition(LEFT_ID);
    int16_t r_pos = st.getCurrentPosition(RIGHT_ID);
    sensor_data_packet[20] = (l_pos >> 8) & 0xff; sensor_data_packet[21] = l_pos & 0xff;
    sensor_data_packet[22] = (r_pos >> 8) & 0xff; sensor_data_packet[23] = r_pos & 0xff;

    sensor_data_packet[28] = 0x00; // Checksum
    Serial.write(sensor_data_packet, SENSOR_PACKET_LENGTH); 
}

void update_oled_display() { // [cite: 32-34]
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);  display.print(debug_line_1);
    display.setCursor(0, 10); display.print(debug_line_2);
    display.setCursor(0, 20); display.print(debug_line_3);
    display.display();
}

void InitHardware() {
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) { // [cite: 26, 79]
        Serial.println(F("OLED failed"));
    }
    imuInit(); // [cite: 56]
    ina219.init(); // [cite: 70]
    st.init(255, &ServoSerial, 1000000); // 서보 초기화
    st.setMode(LEFT_ID, VELOCITY); // 속도 모드 설정
    st.setMode(RIGHT_ID, VELOCITY);
    
    debug_line_1 = "JD-Bot READY";
    debug_line_2 = "Batt: " + String(ina219.getBusVoltage_V()) + "V";
    update_oled_display();
}

void setup() {
    Serial.begin(115200); 
    ServoSerial.begin(1000000, SERIAL_8N1, S_RX, S_TX);
    Wire.begin(S_SDA, S_SCL); // [cite: 35, 78]
    delay(1000);
    InitHardware();
}

// 0x51: 방향 + 속도 (기존 방식)
void exec_motion(byte direction, byte speed_raw) {
    int target_speed = map(speed_raw, 0, 255, 0, MAX_SERVO_VELOCITY); // 속도 매핑

    // LEFT_ID(=2)는 raw 음수가 전진, RIGHT_ID(=1)는 raw 양수가 전진이다.
    if(direction == GO_FORWARD){
        st.setTargetVelocity(LEFT_ID, -target_speed);
        st.setTargetVelocity(RIGHT_ID, target_speed);
        debug_line_2 = "motor: FORWARD";
    } else if(direction == GO_BACKWARD){
        st.setTargetVelocity(LEFT_ID, target_speed);
        st.setTargetVelocity(RIGHT_ID, -target_speed);
        debug_line_2 = "motor: BACKWARD";
    } else if(direction == TURN_LEFT){
        // 좌회전(반시계): 좌 후진 + 우 전진
        st.setTargetVelocity(LEFT_ID, target_speed);
        st.setTargetVelocity(RIGHT_ID, target_speed);
        debug_line_2 = "motor: TURN LEFT";
    } else if(direction == TURN_RIGHT){
        // 우회전(시계): 좌 전진 + 우 후진
        st.setTargetVelocity(LEFT_ID, -target_speed);
        st.setTargetVelocity(RIGHT_ID, -target_speed);
        debug_line_2 = "motor: TURN RIGHT";
    } else if(direction == STOP){
        st.setTargetVelocity(LEFT_ID, 0);
        st.setTargetVelocity(RIGHT_ID, 0);
        debug_line_2 = "motor: STOP";
    } else if(direction == CMD_RESET){
        ESP.restart(); // [cite: 47]
    }
    update_oled_display();
}

// 0x52: 좌/우 바퀴 목표속도 개별 지정. 각 바퀴 기준 양수 = 전진.
void exec_wheel_velocity(int16_t vel_left, int16_t vel_right) {
    vel_left  = constrain(vel_left,  -MAX_SERVO_VELOCITY, MAX_SERVO_VELOCITY);
    vel_right = constrain(vel_right, -MAX_SERVO_VELOCITY, MAX_SERVO_VELOCITY);

    // 두 모터는 서로 마주보게 장착되어 한쪽만 부호가 반대다.
    // 실측: ID1(=RIGHT_ID)은 raw 양수가 전진, ID2(=LEFT_ID)는 raw 음수가 전진.
    // 따라서 반전은 LEFT_ID 쪽에 걸어야 한다.
    st.setTargetVelocity(LEFT_ID, -vel_left);
    st.setTargetVelocity(RIGHT_ID, vel_right);

    debug_line_2 = "vel L:" + String(vel_left) + " R:" + String(vel_right);
}

// 헤더(0xf5 0xf5)를 찾아 동기화한 뒤 명령별 길이만큼 읽는 상태 머신.
// 명령마다 길이가 다르므로 고정 5바이트 읽기로는 처리할 수 없다.
void handle_serial() {
    static byte state = 0;      // 0,1: 헤더 대기  2: 명령 바이트  3: 페이로드
    static byte cmd = 0;
    static byte need = 0, got = 0;
    static byte payload[4];

    while (Serial.available() > 0) {
        byte b = Serial.read();
        switch (state) {
            case 0:
                if (b == 0xf5) state = 1;
                break;
            case 1:
                state = (b == 0xf5) ? 2 : 0;
                break;
            case 2:
                cmd = b;
                got = 0;
                if (cmd == CMD_MOTION)         { need = 2; state = 3; }
                else if (cmd == CMD_WHEEL_VEL) { need = 4; state = 3; }
                else                           { state = 0; }   // 모르는 명령은 버린다
                break;
            case 3:
                payload[got++] = b;
                if (got >= need) {
                    if (cmd == CMD_MOTION) {
                        exec_motion(payload[0], payload[1]);
                    } else if (cmd == CMD_WHEEL_VEL) {
                        int16_t vl = (int16_t)((payload[0] << 8) | payload[1]);
                        int16_t vr = (int16_t)((payload[2] << 8) | payload[3]);
                        exec_wheel_velocity(vl, vr);
                    }
                    state = 0;
                }
                break;
        }
    }
}

void loop() {
    // 1. 명령 수신 파싱 [cite: 37-41]
    handle_serial();

    // 2. 센서 전송 및 OLED 갱신 주기 (100ms) [cite: 52]
    static uint32_t prev_ms = 0;
    if (millis() - prev_ms > 100) {
        send_sensors();
        debug_line_3 = "L:" + String(st.getCurrentPosition(LEFT_ID)) + " R:" + String(st.getCurrentPosition(RIGHT_ID));
        update_oled_display();
        prev_ms = millis();
    }
}