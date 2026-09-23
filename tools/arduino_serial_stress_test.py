#!/usr/bin/env python3

import serial
import sys
import time

PORT = "/dev/ttyACM0"
BAUD = 115200
DRIVE_COUNT = 100


def send_command(ser, command, expected, timeout=1.0):
    ser.reset_input_buffer()

    start = time.monotonic()

    ser.write((command + "\n").encode())
    ser.flush()

    while time.monotonic() - start < timeout:
        response = ser.readline().decode(errors="replace").strip()

        if not response:
            continue

        elapsed = time.monotonic() - start

        return response == expected, response, elapsed

    return False, "TIMEOUT", time.monotonic() - start


def fail(message):
    print(f"FAIL: {message}")
    return 1


def main():
    print(f"Opening {PORT} at {BAUD} baud...")

    try:
        with serial.Serial(PORT, BAUD, timeout=0.2) as ser:
            time.sleep(2)
            ser.reset_input_buffer()

            print("Checking communication...")

            ok, response, _ = send_command(
                ser,
                "PING",
                "PONG"
            )

            if not ok:
                return fail(f"PING returned {response}")

            print("PING: PASS")

            ok, response, _ = send_command(
                ser,
                "STATUS",
                "STATUS,OK,DISARMED,0,0"
            )

            if not ok:
                return fail(f"Initial STATUS returned {response}")

            print("Initial safe state: PASS")

            print("Checking drive rejection while disarmed...")

            ok, response, _ = send_command(
                ser,
                "DRIVE,0,0",
                "ERROR,NOT_ARMED"
            )

            if not ok:
                return fail(
                    f"Disarmed DRIVE returned {response}"
                )

            print("Disarmed DRIVE rejection: PASS")

            print("Arming Arduino...")

            ok, response, _ = send_command(
                ser,
                "ARM",
                "ARMED"
            )

            if not ok:
                return fail(f"ARM returned {response}")

            print("ARM: PASS")

            print(
                f"Sending {DRIVE_COUNT} zero-drive commands..."
            )

            latencies = []

            for i in range(1, DRIVE_COUNT + 1):
                ok, response, latency = send_command(
                    ser,
                    "DRIVE,0,0",
                    "DRIVE_OK,0,0"
                )

                if not ok:
                    return fail(
                        f"DRIVE {i} returned {response}"
                    )

                latencies.append(latency)

                if i % 10 == 0:
                    print(
                        f"  {i}/{DRIVE_COUNT} "
                        "zero-drive commands passed"
                    )

                time.sleep(0.05)

            print("Checking malformed command rejection...")

            ok, response, _ = send_command(
                ser,
                "DRIVE,0",
                "ERROR,BAD_DRIVE_COMMAND"
            )

            if not ok:
                return fail(
                    f"Malformed DRIVE returned {response}"
                )

            print("Malformed DRIVE rejection: PASS")

            # Refresh the valid-drive watchdog before the next test.
            ok, response, _ = send_command(
                ser,
                "DRIVE,0,0",
                "DRIVE_OK,0,0"
            )

            if not ok:
                return fail(
                    f"Watchdog refresh returned {response}"
                )

            print("Checking out-of-range rejection...")

            ok, response, _ = send_command(
                ser,
                "DRIVE,101,0",
                "ERROR,OUT_OF_RANGE"
            )

            if not ok:
                return fail(
                    f"Out-of-range DRIVE returned {response}"
                )

            print("Out-of-range DRIVE rejection: PASS")

            ok, response, _ = send_command(
                ser,
                "STOP",
                "STOPPED"
            )

            if not ok:
                return fail(f"STOP returned {response}")

            print("STOP: PASS")

            ok, response, _ = send_command(
                ser,
                "DISARM",
                "DISARMED"
            )

            if not ok:
                return fail(f"DISARM returned {response}")

            print("DISARM: PASS")

            ok, response, _ = send_command(
                ser,
                "STATUS",
                "STATUS,OK,DISARMED,0,0"
            )

            if not ok:
                return fail(
                    f"Final STATUS returned {response}"
                )

            average_latency_ms = (
                sum(latencies) / len(latencies)
            ) * 1000

            maximum_latency_ms = max(latencies) * 1000

            print()
            print(
                "======= LIONBOT SERIAL PROTOCOL TEST ======="
            )
            print(
                f"Zero-drive commands: {DRIVE_COUNT}/{DRIVE_COUNT}"
            )
            print("Failures:            0")
            print(
                f"Average latency:     "
                f"{average_latency_ms:.1f} ms"
            )
            print(
                f"Maximum latency:     "
                f"{maximum_latency_ms:.1f} ms"
            )
            print("PING:                PASS")
            print("Safe startup:        PASS")
            print("Disarmed protection: PASS")
            print("ARM:                 PASS")
            print("Malformed rejection: PASS")
            print("Range protection:    PASS")
            print("STOP:                PASS")
            print("DISARM:              PASS")
            print("Final safe state:    PASS")
            print("RESULT:              PASS")
            print(
                "============================================"
            )

            return 0

    except serial.SerialException as exc:
        print(f"SERIAL ERROR: {exc}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
