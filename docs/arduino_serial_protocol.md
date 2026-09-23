# LionBot Arduino Serial Protocol

## Purpose

LionBot uses USB serial communication between the Raspberry Pi 5 and
Arduino UNO R4 Minima.

Current control path:

Raspberry Pi 5
-> ROS 2 Jazzy / ros2_control
-> lionbot_hardware
-> USB serial
-> Arduino UNO R4 Minima
-> RC-style signals
-> Cytron SmartDriveDuo-30 (MDDS30)
-> left and right drive motors

Encoder feedback will be added when the encoder hardware is installed.

## Serial Connection

- Port: /dev/ttyACM0
- Baud rate: 115200
- Commands: ASCII text
- Command terminator: newline

## Arduino / MDDS30 Connections

Current verified wiring:

- Arduino D5 -> MDDS30 motor signal channel
- Arduino D9 -> MDDS30 motor signal channel
- Arduino GND -> MDDS30 GND

The Arduino Servo library generates the RC-style control signals.

Current firmware signal values:

- Neutral: 1500 microseconds
- Minimum: 1000 microseconds
- Maximum: 2000 microseconds

Actual motor direction and final scaling still need to be verified during
powered testing.

## Default Safety State

At startup:

- Motor interface is DISARMED
- Left command = 0
- Right command = 0
- Both RC outputs are neutral

Nonzero drive commands are rejected until ARM is received.

## Commands

### PING

Command:

    PING

Response:

    PONG

Used to verify serial communication.

### STATUS

Command:

    STATUS

Response format:

    STATUS,OK,<STATE>,<LEFT>,<RIGHT>

Example:

    STATUS,OK,DISARMED,0,0

Possible states:

- ARMED
- DISARMED

### ARM

Command:

    ARM

Response:

    ARMED

ARM:

- Keeps both outputs neutral initially
- Enables DRIVE commands
- Starts the command timeout
- Turns on the Arduino built-in LED

### DISARM

Command:

    DISARM

Response:

    DISARMED

DISARM:

- Sets left command to 0
- Sets right command to 0
- Returns both signals to neutral
- Rejects further DRIVE commands
- Turns off the built-in LED

### STOP

Command:

    STOP

Response:

    STOPPED

STOP returns both motor commands to zero while preserving the current
armed state.

### DRIVE

Format:

    DRIVE,<LEFT>,<RIGHT>

Allowed range:

    -100 through 100

Examples:

    DRIVE,0,0
    DRIVE,25,25
    DRIVE,-25,-25
    DRIVE,25,-25

Current interpretation:

- 0 = neutral
- Positive = one motor direction
- Negative = opposite motor direction

Physical forward/reverse orientation still needs to be calibrated.

Successful response:

    DRIVE_OK,<LEFT>,<RIGHT>

Example:

    DRIVE_OK,0,0

Possible errors:

    ERROR,NOT_ARMED
    ERROR,OUT_OF_RANGE
    ERROR,BAD_DRIVE_COMMAND
    ERROR,UNKNOWN_COMMAND,<COMMAND>

## Communication Failsafe

Current timeout:

    250 ms

If the Arduino is ARMED and does not receive another valid DRIVE command
within 250 ms, it automatically:

1. Sets both commands to zero.
2. Returns both RC outputs to neutral.
3. Disarms the motor interface.
4. Turns off the built-in LED.

It reports:

    FAILSAFE,COMMAND_TIMEOUT

The verified state afterward is:

    STATUS,OK,DISARMED,0,0

This failsafe was successfully tested on the physical Arduino.

## Verified Tests

The following have been successfully verified on LionBot:

- Arduino detected at /dev/ttyACM0
- Firmware upload from Raspberry Pi
- PING / PONG communication
- STATUS command
- 100 consecutive serial commands with zero failures
- Python PySerial communication
- C++ LibSerial communication
- ARM command
- DRIVE,0,0 command
- STOP command
- DISARM command
- 250 ms communication-loss failsafe
- ros2_control hardware plugin discovery
- ros2_control hardware initialization
- Arduino connection through LionBotSystemHardware
- Automatic hardware activation
- Automatic hardware deactivation
- Single-command LionBot bringup

The MDDS30 motor power was OFF during these tests, so no powered motor
movement occurred.

## ros2_control Integration

ROS 2 hardware plugin:

    lionbot_hardware/LionBotSystemHardware

The plugin currently:

- Opens /dev/ttyACM0
- Verifies a valid STATUS response
- Connects during ros2_control configuration
- Activates successfully
- Sends DISARM during deactivation
- Closes the Arduino connection safely

Actual velocity output from diff_drive_controller is not implemented yet.

That will be added after the drive system and encoder feedback are fully
verified.

## Future Protocol Additions

Planned additions include:

- Left encoder count
- Right encoder count
- Encoder-derived velocity
- ros2_control velocity command output
- Motor-direction calibration
- Tracked-drive odometry calibration
- Additional diagnostic/error reporting

Encoder parameters will not be added until the actual encoder hardware
and counts-per-revolution values are known.
