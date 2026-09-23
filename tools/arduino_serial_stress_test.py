#!/usr/bin/env python3

import serial
import time
import sys

PORT = "/dev/ttyACM0"
BAUD = 115200
PING_COUNT = 100


def send_command(ser, command, expected, timeout=1.0):
    ser.reset_input_buffer()

    start = time.monotonic()

    ser.write((command + "\n").encode())
    ser.flush()

    while time.monotonic() - start < timeout:
        response = ser.readline().decode(errors="replace").strip()

        if not response:
            continue

        if response == expected:
            return True, response, time.monotonic() - start

        return False, response, time.monotonic() - start

    return False, "TIMEOUT", time.monotonic() - start


def main():
    print(f"Opening {PORT} at {BAUD} baud...")

    try:
        with serial.Serial(PORT, BAUD, timeout=0.2) as ser:
            time.sleep(2)
            ser.reset_input_buffer()

            print("Starting simulated robot action...")

            ok, response, _ = send_command(
                ser,
                "START_TEST",
                "TEST_STARTED"
            )

            if not ok:
                print(f"FAIL: START_TEST returned: {response}")
                return 1

            print("TEST_STARTED received.")
            print(f"Sending {PING_COUNT} PING commands...")

            successful_pings = 0
            latencies = []

            for i in range(1, PING_COUNT + 1):
                ok, response, latency = send_command(
                    ser,
                    "PING",
                    "PONG"
                )

                if not ok:
                    print(
                        f"FAIL at PING {i}: "
                        f"expected PONG, received {response}"
                    )
                    return 1

                successful_pings += 1
                latencies.append(latency)

                if i % 10 == 0:
                    ok, response, _ = send_command(
                        ser,
                        "STATUS",
                        "STATUS,OK,TEST_RUNNING"
                    )

                    if not ok:
                        print(
                            f"FAIL after PING {i}: "
                            f"STATUS returned {response}"
                        )
                        return 1

                    print(f"  {i}/{PING_COUNT} PINGs passed")

                time.sleep(0.05)

            print("Stopping simulated robot action...")

            ok, response, _ = send_command(
                ser,
                "STOP_TEST",
                "TEST_STOPPED"
            )

            if not ok:
                print(f"FAIL: STOP_TEST returned: {response}")
                return 1

            ok, response, _ = send_command(
                ser,
                "STATUS",
                "STATUS,OK,TEST_STOPPED"
            )

            if not ok:
                print(f"FAIL: final STATUS returned: {response}")
                return 1

            average_latency_ms = (
                sum(latencies) / len(latencies)
            ) * 1000

            maximum_latency_ms = max(latencies) * 1000

            print()
            print("========== LIONBOT SERIAL TEST ==========")
            print(f"PINGs passed:    {successful_pings}/{PING_COUNT}")
            print(f"Failures:        0")
            print(f"Average latency: {average_latency_ms:.1f} ms")
            print(f"Maximum latency: {maximum_latency_ms:.1f} ms")
            print("Start command:   PASS")
            print("Running status:  PASS")
            print("Stop command:    PASS")
            print("Final status:    PASS")
            print("RESULT:          PASS")
            print("=========================================")

            return 0

    except serial.SerialException as exc:
        print(f"SERIAL ERROR: {exc}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
