#!/usr/bin/env python3
"""
Read all sense/control registers from the Power Management chip (I2C 0x16)
and report whether voltages are within expected limits.

Protocol: read 28 bytes starting at register 0x00.
  bytes  0-19 : 10 x uint16_t (little-endian) – sense registers
  bytes 20-27 :  8 x uint8_t                  – control registers

Run on the Raspberry Pi:
  pip install smbus2
  python3 check_voltages.py
"""

import struct
import sys

try:
    from smbus2 import SMBus
except ImportError:
    sys.exit("smbus2 not found — run: pip install smbus2")

# ── configuration ──────────────────────────────────────────────────────────────
I2C_BUS  = 0       # /dev/i2c-0
I2C_ADDR = 0x16

# (min_V, max_V) for each sense channel. Adjust to your actual rail targets.
# Set to None to skip the OK/FAIL check for that channel.
VOLTAGE_LIMITS = {
    "CM4":       (4.75, 5.25),
    "Lidar1":    (4.50, 5.50),
    "Lidar2":    (4.50, 5.50),
    "CanBus":    (4.50, 5.50),
    "Accessory": (4.50, 5.50),
    "Input5v":   (4.75, 5.25),
    "Batt":      (10.5, 14.6),
    "MainPwr":   (10.5, 14.6),
    "Temp":      (-20,  85  ),   # °C
    "VRefInt":   None,           # do not use
}

SENSE_NAMES = [
    "CM4", "Lidar1", "Lidar2", "CanBus", "Accessory",
    "Input5v", "Batt", "MainPwr", "Temp", "VRefInt",
]
# ───────────────────────────────────────────────────────────────────────────────


def read_registers(bus: SMBus):
    """Return (sense_raw[10], control_raw[8])."""
    raw = bus.read_i2c_block_data(I2C_ADDR, 0x00, 28)
    sense   = list(struct.unpack_from("<10H", bytes(raw),  0))   # 10 × uint16
    control = list(struct.unpack_from("<8B",  bytes(raw), 20))   #  8 × uint8
    return sense, control


def decode_sense(index: int, raw: int):
    """Return (value, unit) for a sense register."""
    if index == 8:                          # Temperature: signed, ×100 °C
        value = struct.unpack("<h", struct.pack("<H", raw))[0] / 100.0
        return value, "°C"
    if index == 9:                          # VRefInt: skip
        return raw / 100.0, "V (do not use)"
    value = (raw & 0x0FFF) / 100.0         # Voltage: bits[11:0], ×100 V
    return value, "V"


def check(name: str, value: float) -> str:
    limits = VOLTAGE_LIMITS.get(name)
    if limits is None:
        return "  —  "
    lo, hi = limits
    return "  OK " if lo <= value <= hi else " FAIL"


def main() -> None:
    try:
        with SMBus(I2C_BUS) as bus:
            sense, control = read_registers(bus)
    except FileNotFoundError:
        sys.exit(f"I2C bus {I2C_BUS} not found – is I2C enabled? (raspi-config)")
    except OSError as exc:
        sys.exit(f"I2C error talking to 0x{I2C_ADDR:02X}: {exc}")

    print(f"\n{'Reg':<5} {'Name':<12} {'Raw':>6}  {'Value':>9}  {'Status'}")
    print("─" * 46)
    all_ok = True
    for i, name in enumerate(SENSE_NAMES):
        raw = sense[i]
        value, unit = decode_sense(i, raw)
        status = check(name, value)
        if "FAIL" in status:
            all_ok = False
        print(f"0x{i:02X}  {name:<12} 0x{raw:04X}  {value:>7.2f} {unit:<3}  {status}")

    print("─" * 46)
    print(f"\nControl registers (0x0A–0x11):")
    for i, val in enumerate(control):
        print(f"  0x{0x0A + i:02X}  0x{val:02X}  ({val:08b}b)")

    print()
    if all_ok:
        print(">>> ALL VOLTAGES OK <<<")
    else:
        print(">>> FAULT DETECTED – check FAIL lines above <<<")
        sys.exit(1)


if __name__ == "__main__":
    main()
