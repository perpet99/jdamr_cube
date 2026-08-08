# JDAMR Cube - ROS2 2-Wheel Mobile Robot Firmware

**JDAMR Cube**는 ROS2 학습 및 자율주행 로봇 실습을 위한 **2바퀴 차동구동(Differential Drive) 모바일 로봇**입니다.

이 저장소는 JDAMR Cube의 **ESP32 기반 하위 제어기(Low-Level Controller) 펌웨어 및 하드웨어 테스트 예제**를 제공합니다.

상위 제어기는 Raspberry Pi에서 ROS2를 실행하고, 하위 제어기는 Waveshare ESP32 기반 Robot Controller Board를 사용합니다.

---

## 1. 시스템 구성

JDAMR Cube는 다음과 같은 구조로 구성됩니다.

```text
              ┌─────────────────────┐
              │    Raspberry Pi     │
              │                     │
              │       ROS2          │
              │   Navigation / SLAM │
              │      LiDAR Driver   │
              └──────────┬──────────┘
                         │
                    Serial / USB
                         │
              ┌──────────▼──────────┐
              │ Waveshare ESP32     │
              │ Robot Controller    │
              │                     │
              │ Motor Control       │
              │ Encoder Reading     │
              │ IMU Reading         │
              │ Battery Monitoring  │
              │ OLED Display        │
              └──────┬───────┬──────┘
                     │       │
              ┌──────▼─┐   ┌─▼──────┐
              │STS3215 │   │STS3215 │
              │ Left   │   │ Right  │
              └────────┘   └────────┘
```

---

# 2. 주요 하드웨어

## Main Controller

로봇의 하위 제어기로 **Waveshare ESP32 기반 Robot Controller Board**를 사용합니다.

ESP32는 다음 기능을 담당합니다.

* 좌/우 구동모터 제어
* STS3215 위치 정보 읽기
* IMU 데이터 읽기
* 배터리 전압 측정
* OLED 상태 표시
* Raspberry Pi와 통신

Raspberry Pi는 ESP32보다 상위 계층에서 ROS2를 실행합니다.

---

## 3. Drive Motor - STS3215

JDAMR Cube는 일반적인 DC 기어드 모터 대신 **STS3215 Serial Bus Servo**를 좌우 구동모터로 사용합니다.

```text
        Front

          ↑

    ┌─────────────┐
    │             │
    │             │
 ●──┤             ├──●
Left│             │Right
    │             │
    └─────────────┘
```

STS3215는 Wheel Mode를 이용하여 연속 회전 구동모터로 사용할 수 있습니다.

따라서 별도의 DC 모터 드라이버 없이 시리얼 버스를 이용하여 모터를 제어할 수 있습니다.

### STS3215의 역할

STS3215는 이 로봇에서 두 가지 역할을 동시에 수행합니다.

**1. 구동모터**

좌우 바퀴의 회전 속도와 방향을 제어합니다.

**2. Wheel Encoder**

STS3215 내부의 위치 센서를 이용하여 바퀴의 회전량을 측정합니다.

따라서 일반적인 모바일 로봇에서 사용하는 별도의 엔코더가 필요하지 않습니다.

```text
STS3215

Motor Control
     +
Position Feedback
     ↓
Wheel Encoder
```

저장소에는 STS3215의 동작을 확인하기 위한 예제 코드가 포함됩니다.

예제 코드를 통해 다음 기능을 단계적으로 테스트할 수 있습니다.

* Servo 통신 확인
* Motor ID 확인
* Wheel Mode 설정
* 정회전 / 역회전
* 속도 제어
* 정지
* 현재 위치 읽기
* 좌우 모터 독립 제어

---

# 4. IMU

로봇의 자세 및 움직임 측정을 위해 **Waveshare Robot Controller Board에 탑재된 IMU**를 사용합니다.

별도의 외부 IMU 모듈을 추가하지 않고 ESP32에서 보드의 IMU 데이터를 읽습니다.

IMU를 통해 다음과 같은 정보를 얻을 수 있습니다.

```text
Accelerometer
     │
     ├── ax
     ├── ay
     └── az

Gyroscope
     │
     ├── gx
     ├── gy
     └── gz
```

향후 ROS2에서는 이 정보를 `sensor_msgs/Imu` 메시지로 변환하여 사용할 수 있습니다.

IMU 데이터는 Wheel Encoder 정보와 함께 로봇의 이동 상태를 추정하는 데 사용할 수 있습니다.

---

# 5. Wheel Encoder

JDAMR Cube에서는 별도의 엔코더를 장착하지 않습니다.

좌우 STS3215에서 제공하는 **현재 위치(Position) 정보**를 이용하여 바퀴의 회전량을 계산합니다.

```text
Left STS3215 Position
          │
          ▼
 Left Wheel Rotation


Right STS3215 Position
          │
          ▼
 Right Wheel Rotation
```

두 바퀴의 회전량을 이용하면 로봇의 이동 거리와 회전량을 계산할 수 있습니다.

향후 ROS2에서는 이 데이터를 이용하여 다음과 같은 구조로 Odometry를 구성할 수 있습니다.

```text
STS3215 Position
       │
       ▼
Wheel Rotation
       │
       ▼
Differential Drive Calculation
       │
       ▼
     Odometry
       │
       ▼
    /odom
```

---

# 6. OLED Display

로봇의 상태를 쉽게 확인할 수 있도록 **I2C OLED Display**를 연결합니다.

OLED는 디버깅 및 로봇 상태 확인 용도로 사용합니다.

표시할 수 있는 정보의 예는 다음과 같습니다.

```text
JDAMR CUBE

BAT : 11.8 V

LEFT  :  120
RIGHT :  118

IMU : OK
ROS : READY
```

펌웨어 개발 과정에서는 OLED를 통해 다음과 같은 정보를 확인할 수 있습니다.

* 시스템 부팅 상태
* 배터리 전압
* 좌/우 모터 상태
* STS3215 통신 상태
* IMU 상태
* Raspberry Pi 통신 상태
* 오류 메시지

PC의 Serial Monitor를 연결하지 않아도 기본적인 로봇 상태를 현장에서 확인할 수 있도록 하는 것이 목적입니다.

---

# 7. Battery Voltage Monitoring

ESP32 Robot Controller Board를 이용하여 로봇의 **배터리 전압을 측정**합니다.

측정된 전압은 OLED에 표시할 수 있습니다.

예:

```text
BATTERY

12.1 V
```

배터리 전압 모니터링을 통해 배터리 부족 상태를 확인하고, 향후에는 저전압 경고 기능도 구현할 수 있습니다.

```text
Battery
   │
   ▼
Voltage Measurement
   │
   ├── OLED Display
   │
   └── ROS2 Battery Status
```

---

# 8. LiDAR

LiDAR는 ESP32에 연결하지 않습니다.

LiDAR는 **ROS2를 실행하는 Raspberry Pi에 직접 연결**합니다.

```text
LiDAR
  │
 USB / Serial
  │
  ▼
Raspberry Pi
  │
ROS2 LiDAR Driver
  │
  ▼
/scan
```

ESP32는 실시간 모터 및 센서 제어를 담당하고, LiDAR와 같이 데이터 처리량이 큰 센서는 Raspberry Pi에서 직접 처리하도록 역할을 분리합니다.

---

# 9. Raspberry Pi와 ESP32의 역할

JDAMR Cube는 상위 제어기와 하위 제어기의 역할을 명확하게 분리합니다.

| 장치           | 주요 역할            |
| ------------ | ---------------- |
| Raspberry Pi | ROS2 실행          |
| Raspberry Pi | LiDAR 처리         |
| Raspberry Pi | SLAM             |
| Raspberry Pi | Navigation       |
| Raspberry Pi | Odometry 및 TF 처리 |
| ESP32        | STS3215 모터 제어    |
| ESP32        | STS3215 위치 읽기    |
| ESP32        | IMU 데이터 읽기       |
| ESP32        | 배터리 전압 측정        |
| ESP32        | OLED 표시          |

전체 구조는 다음과 같습니다.

```text
                ROS2
                  │
          ┌───────▼───────┐
          │ Raspberry Pi  │
          │               │
LiDAR ───►│ /scan         │
          │ SLAM / Nav2   │
          │ Odometry      │
          └───────┬───────┘
                  │
              Serial
                  │
          ┌───────▼───────┐
          │     ESP32     │
          │               │
          │ Motor Control │
          │ Encoder       │
          │ IMU           │
          │ Battery       │
          │ OLED          │
          └──────┬─┬──────┘
                 │ │
           ┌─────┘ └─────┐
           ▼             ▼
       STS3215        STS3215
       Left Motor     Right Motor
```

---

# 10. Firmware Development

펌웨어는 기능별 예제 프로그램을 이용하여 하드웨어를 먼저 테스트한 후 전체 로봇 펌웨어로 통합하는 방식을 권장합니다.

권장 테스트 순서는 다음과 같습니다.

```text
ESP32 동작 확인
      ↓
STS3215 통신 확인
      ↓
왼쪽 모터 구동
      ↓
오른쪽 모터 구동
      ↓
Wheel Mode 테스트
      ↓
Position 읽기
      ↓
IMU 테스트
      ↓
OLED 테스트
      ↓
Battery Voltage 테스트
      ↓
Raspberry Pi 통신
      ↓
ROS2 연동
```

특히 ROS2를 연결하기 전에 ESP32 단독으로 모터, 엔코더, IMU가 정상적으로 동작하는지 확인하는 것을 권장합니다.

---

# 11. ROS2 데이터 구조

최종적으로 Raspberry Pi와 ESP32 사이에서는 다음과 같은 정보가 교환됩니다.

### Raspberry Pi → ESP32

```text
cmd_vel

linear.x
angular.z
```

ESP32에서는 이를 좌우 바퀴 속도로 변환합니다.

```text
        cmd_vel
           │
           ▼
 Differential Drive
     Calculation
       │       │
       ▼       ▼
     Left    Right
     Speed   Speed
       │       │
       ▼       ▼
    STS3215 STS3215
```

### ESP32 → Raspberry Pi

ESP32에서는 다음 정보를 Raspberry Pi로 전달할 수 있습니다.

```text
Left Wheel Position
Right Wheel Position

IMU
 ├─ Acceleration
 └─ Gyroscope

Battery Voltage
```

Raspberry Pi의 ROS2 노드는 이 데이터를 ROS2 메시지로 변환합니다.

---

# 12. 개발 목표

JDAMR Cube의 목표는 단순히 완성된 ROS2 로봇을 사용하는 것이 아니라,

**모터 → 엔코더 → IMU → Odometry → ROS2**

과정을 직접 이해할 수 있는 교육용 모바일 로봇 플랫폼을 만드는 것입니다.

특히 다음 구조를 학습하는 것을 목표로 합니다.

```text
Hardware
   ↓
ESP32 Firmware
   ↓
Serial Communication
   ↓
ROS2 Node
   ↓
Odometry / TF
   ↓
LiDAR
   ↓
SLAM
   ↓
Navigation
```

이를 통해 완성된 ROS2 로봇을 사용하는 것보다 로봇의 하위 제어기와 ROS2 사이의 관계를 단계적으로 학습할 수 있습니다.

---

# Repository

JDAMR Cube Firmware

https://github.com/JD-edu/jdamr_cube

이 저장소의 코드는 교육 및 연구 목적으로 지속적으로 업데이트될 수 있습니다.
