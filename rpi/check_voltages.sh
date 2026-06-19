#!/bin/bash
# Boot voltage check — I2C 0x16 power management chip.
# Exits 0 if all rails OK, exits 1 on any fault.

I2C_BUS=0
I2C_ADDR=0x16

NAMES=("CM4" "Lidar1" "Lidar2" "CanBus" "Accessory" "Input5v" "Batt" "MainPwr" "Temp" "VRefInt")

# "min max" per sense channel (V or °C). Empty string = skip check.
LIMITS=(
    "4.75 5.25"  # CM4
    "4.50 5.50"  # Lidar1
    "4.50 5.50"  # Lidar2
    "4.50 5.50"  # CanBus
    "4.50 6.00"  # Accessory
    "4.75 5.25"  # Input5v
    "10.5 17.0"  # Batt
    "10.5 17.0"  # MainPwr
    "-20  85"    # Temp (°C)
    ""           # VRefInt – do not use
)

to_bin() {
    local n=$1 b=""
    for bit in 7 6 5 4 3 2 1 0; do b+=$(( (n >> bit) & 1 )); done
    echo "$b"
}

I2CTRANSFER=$(command -v i2ctransfer || echo /usr/sbin/i2ctransfer)
[ -x "$I2CTRANSFER" ] || { echo "ERROR: i2ctransfer not found – install i2c-tools"; exit 1; }

bytes=($("$I2CTRANSFER" -y "$I2C_BUS" w1@"$I2C_ADDR" 0x00 r28@"$I2C_ADDR" 2>&1))
[ ${#bytes[@]} -eq 28 ] || {
    echo "ERROR: I2C 0x${I2C_ADDR} read failed (got ${#bytes[@]} bytes)"
    exit 1
}

SEP=$(printf '%.0s-' {1..48})
printf "%-5s %-12s %6s  %9s  %s\n" "Reg" "Name" "Raw" "Value" "Status"
echo "$SEP"

all_ok=1
for i in $(seq 0 9); do
    lo=$(( ${bytes[$((i * 2))]} ))
    hi=$(( ${bytes[$((i * 2 + 1))]} ))
    raw=$(( (hi << 8) | lo ))
    name="${NAMES[$i]}"
    limit="${LIMITS[$i]}"

    if [ "$i" -eq 8 ]; then
        # Temperature: signed int16, value x100 degC
        [ "$raw" -ge 32768 ] && raw=$(( raw - 65536 ))
        value=$(awk "BEGIN { printf \"%.2f\", $raw / 100.0 }")
        unit="C"
    else
        masked=$(( raw & 0x0FFF ))
        value=$(awk "BEGIN { printf \"%.2f\", $masked / 100.0 }")
        unit="V"
    fi

    if [ -z "$limit" ]; then
        status="  --  "
    else
        read -r lo_lim hi_lim <<< "$limit"
        if awk "BEGIN { exit !($value >= $lo_lim && $value <= $hi_lim) }"; then
            status="  OK  "
        else
            status=" FAIL "
            all_ok=0
        fi
    fi

    printf "0x%02X  %-12s 0x%04X  %7s %-2s   %s\n" \
        "$i" "$name" "$(( (hi << 8) | lo ))" "$value" "$unit" "$status"
done

echo "$SEP"
echo ""
echo "Control registers (0x0A-0x11):"
for i in $(seq 0 7); do
    val=$(( ${bytes[$((20 + i))]} ))
    printf "  0x%02X  0x%02X  (%sb)\n" "$((0x0A + i))" "$val" "$(to_bin "$val")"
done

echo ""
if [ "$all_ok" -eq 1 ]; then
    echo ">>> ALL VOLTAGES OK <<<"
    exit 0
else
    echo ">>> FAULT DETECTED - check FAIL lines above <<<"
    exit 1
fi
