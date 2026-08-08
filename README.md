
# JDAMR Cube Firmware (ROS 2 Two-Wheel Mobile Robot)

이 저장소는 **2바퀴 Differential Drive 구조의 ROS 2 모바일 로봇(JDAMR Cube)**을 제어하기 위한 **ESP32 기반 펌웨어 및 예제 코드**를 포함하고 있습니다.

---

## 🛠 주요 하드웨어 스펙 및 구성 (Hardware Overview)

| 구성 요소 | 제품 / 기술 스펙 | 비고 및 설명 |
| :--- | :--- | :--- |
| **메인 제어 보드** | **Waveshare Robot Controller Board (ESP32)** | ESP32 기반 로봇 전용 컨트롤러 보드 |
| **구동 모터 & 엔코더** | **STS3215 스마트 시리얼 서보 모터** | 구동 모터 및 내장 엔코더 피드백 활용 |
| **IMU (관성 측정 장치)** | Board Onboard IMU | 로봇 자세 측정 (내장 센서 활용) |
| **디스플레이** | **OLED Display (I2C)** | 배터리 전압 및 로봇 현재 상태 실시간 표시 |
| **라이다 (LiDAR)** | ROS 2 상위 제어기 연결 | **라즈베리파이(ROS 2 Host)**에서 직접 제어 및 데이터 수신 |

---

## 📐 시스템 아키텍처 (System Architecture)

+-------------------------------------------------------------+
|               ROS 2 Host (Raspberry Pi)                     |
|                                                             |
|   [ LiDAR Sensor ] ------ Direct USB/Serial Connection      |
|   [ ROS 2 Navigation / SLAM / Teleop ]                      |
+------------------------------+------------------------------+
| Micro-ROS / Serial (UART)
+------------------------------v------------------------------+
|       Waveshare ESP32 Robot Controller Board                |
|                                                             |
|   - Onboard IMU Sensor                                      |
|   - Battery Voltage Monitor                                 |
|   - OLED Display (I2C) -------- STATUS & VOLTAGE            |
+------------------------------+------------------------------+
| RS485 / Serial Bus
+------------------------------v------------------------------+
|            STS3215 Smart Servo Motors x 2                   |
|   (Left & Right Wheel Drives + Internal Encoders)           |
+-------------------------------------------------------------+


---

## 💡 주요 기능 (Key Features)

1. **STS3215 서보 모터 제어 및 엔코더 피드백**
   - 모터 구동 및 엔코더 위치/속도 피드백을 수신하여 2바퀴 차동 제어(Differential Drive) 수행
   - 저장소 내 제공되는 STS3215 제어 예제 코드를 참조하여 커스텀 가능

2. **내장 IMU 및 휠 엔코더 융합 오도메트리**
   - ESP32 보드에 탑재된 IMU 센서를 활용하여 정밀한 기heading/자세 데이터 수집

3. **I2C OLED 디스플레이 상태 출력**
   - 로봇 전원 전압(Voltage) 실시간 모니터링
   - 시스템 연결 상태 및 로봇 동작 모드 화면 표시

4. **라이다(LiDAR) 분리형 구성**
   - 라이다 센서는 MCU가 아닌 **라즈베리파이(ROS 2 제어기)**에 직접 연결하여 데이터 병목을 줄이고 ROS 2 노드에서 바로 SLAM/Navigation에 활용

---

## 📂 파일 및 폴더 구조 (Directory Structure)

```text
jdamr_cube/
├── examples/             # STS3215 모터 및 보드 제어 예제 코드
│   ├── sts3215_basic/    # STS3215 구동 기본 예제
│   ├── oled_voltage/     # OLED 전압 표시 예제
│   └── imu_read/         # 온보드 IMU 데이터 수신 예제
├── include/              # 헤더 파일
├── src/                  # 메인 펌웨어 소스 코드
└── platformio.ini        # PlatformIO 설정 파일 (또는 CMakeLists.txt)
🚀 시작하기 (Getting Started)
1. 개발 환경 설정
IDE: VS Code + PlatformIO (추천) 또는 Arduino IDE

Board: ESP32 Dev Module (Waveshare Robot Board)

2. 펌웨어 업로드 (Upload)
보드와 PC를 USB 케이블로 연결합니다.

PlatformIO 환경에서 프로젝트를 열고 핀 맵(Pinout) 설정을 확인합니다.

빌드 후 펌웨어를 업로드합니다.

Bash
# PlatformIO CLI 사용 시
pio run --target upload
🔗 연동 가이드 (ROS 2 Host Configuration)
LiDAR 센서: 라즈베리파이의 USB 포트에 직접 연결한 후, 각 라이다 드라이버 ROS 2 노드를 실행하세요.

Micro-ROS / Serial Communication: ESP32 보드의 시리얼 포트와 라즈베리파이를 연결하여 /cmd_vel 수신 및 /odom, /imu 데이터를 발행합니다.

📜 라이선스 (License)
본 프로젝트는 MIT License에 따라 제공됩니다.

하나의 md파일로 만들어 주세요

+-------------------------------------------------------------+
|               ROS 2 Host (Raspberry Pi)                     |
|                                                             |
|   [ LiDAR Sensor ] ------ Direct USB/Serial Connection      |
|   [ ROS 2 Navigation / SLAM / Teleop ]                      |
+------------------------------+------------------------------+
| Micro-ROS / Serial (UART)
+------------------------------v------------------------------+
|       Waveshare ESP32 Robot Controller Board                |
|                                                             |
|   - Onboard IMU Sensor                                      |
|   - Battery Voltage Monitor                                 |
|   - OLED Display (I2C) -------- STATUS & VOLTAGE            |
+------------------------------+------------------------------+
| RS485 / Serial Bus
+------------------------------v------------------------------+
|            STS3215 Smart Servo Motors x 2                   |
|   (Left & Right Wheel Drives + Internal Encoders)           |
+-------------------------------------------------------------+


---

## 💡 주요 기능 (Key Features)

1. **STS3215 서보 모터 제어 및 엔코더 피드백**
   - 모터 구동 및 엔코더 위치/속도 피드백을 수신하여 2바퀴 차동 제어(Differential Drive) 수행
   - 저장소 내 제공되는 STS3215 제어 예제 코드를 참조하여 커스텀 가능

2. **내장 IMU 및 휠 엔코더 융합 오도메트리**
   - ESP32 보드에 탑재된 IMU 센서를 활용하여 정밀한 기heading/자세 데이터 수집

3. **I2C OLED 디스플레이 상태 출력**
   - 로봇 전원 전압(Voltage) 실시간 모니터링
   - 시스템 연결 상태 및 로봇 동작 모드 화면 표시

4. **라이다(LiDAR) 분리형 구성**
   - 라이다 센서는 MCU가 아닌 **라즈베리파이(ROS 2 제어기)**에 직접 연결하여 데이터 병목을 줄이고 ROS 2 노드에서 바로 SLAM/Navigation에 활용

---

## 📂 파일 및 폴더 구조 (Directory Structure)

```text
jdamr_cube/
├── examples/             # STS3215 모터 및 보드 제어 예제 코드
│   ├── sts3215_basic/    # STS3215 구동 기본 예제
│   ├── oled_voltage/     # OLED 전압 표시 예제
│   └── imu_read/         # 온보드 IMU 데이터 수신 예제
├── include/              # 헤더 파일
├── src/                  # 메인 펌웨어 소스 코드
└── platformio.ini        # PlatformIO 설정 파일 (또는 CMakeLists.txt)
🚀 시작하기 (Getting Started)
1. 개발 환경 설정
IDE: VS Code + PlatformIO (추천) 또는 Arduino IDE

Board: ESP32 Dev Module (Waveshare Robot Board)

2. 펌웨어 업로드 (Upload)
보드와 PC를 USB 케이블로 연결합니다.

PlatformIO 환경에서 프로젝트를 열고 핀 맵(Pinout) 설정을 확인합니다.

빌드 후 펌웨어를 업로드합니다.

Bash
# PlatformIO CLI 사용 시
pio run --target upload
🔗 연동 가이드 (ROS 2 Host Configuration)
LiDAR 센서: 라즈베리파이의 USB 포트에 직접 연결한 후, 각 라이다 드라이버 ROS 2 노드를 실행하세요.

Micro-ROS / Serial Communication: ESP32 보드의 시리얼 포트와 라즈베리파이를 연결하여 /cmd_vel 수신 및 /odom, /imu 데이터를 발행합니다.

📜 라이선스 (License)
본 프로젝트는 MIT License에 따라 제공됩니다.
"""

file_path = "README.md"
with open(file_path, "w", encoding="utf-8") as f:
f.write(content)

print(f"File created successfully: {file_path}")


```text?code_stdout&code_event_index=1
File created successfully: README.md

요청하신 내용(STS3215 구동모터 및 예제, Waveshare ESP32 로봇제어보드, 온보드 IMU, I2C OLED 전압/상태 체크, 라즈베리파이 LiDAR 직접 제어)을 반영한 README.md 파일 생성이 완료되었습니다.

Your Markdown file is ready

MD 아이콘
README
 MD 
열기



Gemini는 AI이며 인물 등에 관한 정보 제공 시 실수를 할 수 있습니다. 개인 정보 보호 및 Gemini새 창에서 열기

# JDAMR Cube Firmware (ROS 2 Two-Wheel Mobile Robot)

이 저장소는 **2바퀴 Differential Drive 구조의 ROS 2 모바일 로봇(JDAMR Cube)**을 제어하기 위한 **ESP32 기반 펌웨어 및 예제 코드**를 포함하고 있습니다.

---

## 🛠 주요 하드웨어 스펙 및 구성 (Hardware Overview)

| 구성 요소 | 제품 / 기술 스펙 | 비고 및 설명 |
| :--- | :--- | :--- |
| **메인 제어 보드** | **Waveshare Robot Controller Board (ESP32)** | ESP32 기반 로봇 전용 컨트롤러 보드 |
| **구동 모터 & 엔코더** | **STS3215 스마트 시리얼 서보 모터** | 구동 모터 및 내장 엔코더 피드백 활용 |
| **IMU (관성 측정 장치)** | Board Onboard IMU | 로봇 자세 측정 (내장 센서 활용) |
| **디스플레이** | **OLED Display (I2C)** | 배터리 전압 및 로봇 현재 상태 실시간 표시 |
| **라이다 (LiDAR)** | ROS 2 상위 제어기 연결 | **라즈베리파이(ROS 2 Host)**에서 직접 제어 및 데이터 수신 |

---

## 📐 시스템 아키텍처 (System Architecture)

```
+-------------------------------------------------------------+
|               ROS 2 Host (Raspberry Pi)                     |
|                                                             |
|   [ LiDAR Sensor ] ------ Direct USB/Serial Connection      |
|   [ ROS 2 Navigation / SLAM / Teleop ]                      |
+------------------------------+------------------------------+
                               | Micro-ROS / Serial (UART)
+------------------------------v------------------------------+
|       Waveshare ESP32 Robot Controller Board                |
|                                                             |
|   - Onboard IMU Sensor                                      |
|   - Battery Voltage Monitor                                 |
|   - OLED Display (I2C) -------- STATUS & VOLTAGE            |
+------------------------------+------------------------------+
                               | RS485 / Serial Bus
+------------------------------v------------------------------+
|            STS3215 Smart Servo Motors x 2                   |
|   (Left & Right Wheel Drives + Internal Encoders)           |
+-------------------------------------------------------------+
```

---

## 💡 주요 기능 (Key Features)

1. **STS3215 서보 모터 제어 및 엔코더 피드백**
   - 모터 구동 및 엔코더 위치/속도 피드백을 수신하여 2바퀴 차동 제어(Differential Drive) 수행
   - 저장소 내 제공되는 STS3215 제어 예제 코드를 참조하여 커스텀 가능

2. **내장 IMU 및 휠 엔코더 융합 오도메트리**
   - ESP32 보드에 탑재된 IMU 센서를 활용하여 정밀한 기heading/자세 데이터 수집

3. **I2C OLED 디스플레이 상태 출력**
   - 로봇 전원 전압(Voltage) 실시간 모니터링
   - 시스템 연결 상태 및 로봇 동작 모드 화면 표시

4. **라이다(LiDAR) 분리형 구성**
   - 라이다 센서는 MCU가 아닌 **라즈베리파이(ROS 2 제어기)**에 직접 연결하여 데이터 병목을 줄이고 ROS 2 노드에서 바로 SLAM/Navigation에 활용

---

## 📂 파일 및 폴더 구조 (Directory Structure)

```text
jdamr_cube/
├── examples/             # STS3215 모터 및 보드 제어 예제 코드
│   ├── sts3215_basic/    # STS3215 구동 기본 예제
│   ├── oled_voltage/     # OLED 전압 표시 예제
│   └── imu_read/         # 온보드 IMU 데이터 수신 예제
├── include/              # 헤더 파일
├── src/                  # 메인 펌웨어 소스 코드
└── platformio.ini        # PlatformIO 설정 파일 (또는 CMakeLists.txt)
```

---

## 🚀 시작하기 (Getting Started)

### 1. 개발 환경 설정
* **IDE**: [VS Code](https://code.visualstudio.com/) + [PlatformIO](https://platformio.org/) (추천) 또는 Arduino IDE
* **Board**: ESP32 Dev Module (Waveshare Robot Board)

### 2. 펌웨어 업로드 (Upload)
1. 보드와 PC를 USB 케이블로 연결합니다.
2. PlatformIO 환경에서 프로젝트를 열고 핀 맵(Pinout) 설정을 확인합니다.
3. 빌드 후 펌웨어를 업로드합니다.
   ```bash
   # PlatformIO CLI 사용 시
   pio run --target upload
   ```

---

## 🔗 연동 가이드 (ROS 2 Host Configuration)

* **LiDAR 센서**: 라즈베리파이의 USB 포트에 직접 연결한 후, 각 라이다 드라이버 ROS 2 노드를 실행하세요.
* **Micro-ROS / Serial Communication**: ESP32 보드의 시리얼 포트와 라즈베리파이를 연결하여 `/cmd_vel` 수신 및 `/odom`, `/imu` 데이터를 발행합니다.

---

## 📜 라이선스 (License)

본 프로젝트는 [MIT License](LICENSE)에 따라 제공됩니다.
README.md
README.md 항목을 표시하는 중입니다.
