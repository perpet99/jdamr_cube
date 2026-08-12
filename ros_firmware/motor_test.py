#!/usr/bin/env python3
"""Raspberry Pi -> ESP32 motor control test.

Talks to the Waveshare ESP32 Robot Controller Board over the 40-pin header
UART using the protocol implemented in ros_firmware_no_rtos.ino:

  Command (Pi -> ESP32), 5 bytes:
    0xF5 0xF5 0x51 <direction> <speed 0-255>
    direction: 1=FORWARD, 5=STOP, 6=RESET (BACKWARD/TURN are not yet
    handled by the current firmware loop(), so they are not sent here)

  Sensor packet (ESP32 -> Pi), 29 bytes, sent every ~100ms:
    0xF5 0xF5 <ax><ay><az><gx> ... <left_pos><right_pos> ... <checksum>
    (all int16 big-endian; only ax/ay/az/gx and left/right position are
    actually populated by the current firmware)

Usage:
  sudo python3 motor_test.py [--port /dev/ttyS0] [--speed 40] [--duration 2]
"""
import argparse
import glob
import struct
import sys
import time

import serial

BAUDRATE = 115200
CMD_PACKET_LEN = 5
SENSOR_PACKET_LEN = 29

GO_FORWARD = 1
STOP = 5
CMD_RESET = 6

CANDIDATE_PORTS = ["/dev/ttyS0", "/dev/ttyAMA0", "/dev/serial0"] + sorted(
    glob.glob("/dev/ttyUSB*") + glob.glob("/dev/ttyACM*")
)


def find_port():
    for p in CANDIDATE_PORTS:
        try:
            s = serial.Serial(p, BAUDRATE, timeout=0.2)
            s.close()
            return p
        except serial.SerialException:
            continue
    return None


def build_cmd(direction, speed=0):
    return bytes([0xF5, 0xF5, 0x51, direction, speed])


def read_sensor_packet(ser, timeout=0.5):
    """Read one 0xF5 0xF5-framed sensor packet, or None on timeout."""
    deadline = time.time() + timeout
    while time.time() < deadline:
        b = ser.read(1)
        if b != b"\xf5":
            continue
        if ser.read(1) != b"\xf5":
            continue
        payload = ser.read(SENSOR_PACKET_LEN - 2)
        if len(payload) != SENSOR_PACKET_LEN - 2:
            return None
        ax, ay, az, gx = struct.unpack(">hhhh", payload[0:8])
        l_pos, r_pos = struct.unpack(">hh", payload[18:22])
        return {
            "accel": (ax / 1000.0, ay / 1000.0, az / 1000.0),
            "gyro_x": gx / 100.0,
            "left_pos": l_pos,
            "right_pos": r_pos,
        }
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--port", default=None, help="serial device (default: auto-detect)")
    ap.add_argument("--speed", type=int, default=40, help="0-255, default 40 (gentle)")
    ap.add_argument("--duration", type=float, default=2.0, help="seconds to run forward")
    args = ap.parse_args()

    port = args.port or find_port()
    if not port:
        print("No serial port found/openable. Checked:", CANDIDATE_PORTS)
        print("Is the board seated on the 40-pin header? Run with sudo?")
        sys.exit(1)

    print(f"Opening {port} @ {BAUDRATE} baud ...")
    try:
        ser = serial.Serial(port, BAUDRATE, timeout=0.2)
    except serial.SerialException as e:
        print(f"Failed to open {port}: {e}")
        print("Try: sudo python3 motor_test.py")
        sys.exit(1)

    time.sleep(0.3)
    ser.reset_input_buffer()

    print("Listening for sensor packets to confirm ESP32 firmware is alive ...")
    baseline = read_sensor_packet(ser, timeout=1.5)
    if baseline is None:
        print("No sensor packets received. The wiring/port is open, but the")
        print("ESP32 does not seem to be running the firmware (or is on a")
        print("different UART). Flash ros_firmware_no_rtos.ino and retry.")
        ser.close()
        sys.exit(1)

    print(f"ESP32 alive. baseline left_pos={baseline['left_pos']} "
          f"right_pos={baseline['right_pos']} accel={baseline['accel']}")

    try:
        print(f"Sending FORWARD (speed={args.speed}) for {args.duration}s ...")
        ser.write(build_cmd(GO_FORWARD, args.speed))

        end = time.time() + args.duration
        while time.time() < end:
            pkt = read_sensor_packet(ser, timeout=0.3)
            if pkt:
                print(f"  left_pos={pkt['left_pos']:6d}  right_pos={pkt['right_pos']:6d}")
    finally:
        print("Sending STOP ...")
        ser.write(build_cmd(STOP))
        time.sleep(0.2)
        final = read_sensor_packet(ser, timeout=1.0)
        if final:
            print(f"final left_pos={final['left_pos']} right_pos={final['right_pos']}")
            dl = final["left_pos"] - baseline["left_pos"]
            dr = final["right_pos"] - baseline["right_pos"]
            print(f"delta left={dl} right={dr}")
            if dl == 0 and dr == 0:
                print("WARNING: encoder positions did not change - check motor "
                      "wiring/power or servo IDs.")
            else:
                print("Motor movement confirmed (encoder position changed).")
        ser.close()


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\nInterrupted.")
