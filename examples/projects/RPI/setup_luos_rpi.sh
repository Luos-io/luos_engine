#!/bin/bash
# setup_luos_rpi.sh - Prepare a fresh Raspberry Pi OS for Luos/Robus development
# Run this script on the RPI itself (e.g., via SSH)
set -e

echo "=== Luos RPI Setup ==="

# Install build dependencies
echo "[1/5] Installing build dependencies..."
sudo apt-get update
sudo apt-get install -y build-essential libgpiod-dev python3-pip python3-venv

# Install PlatformIO Core CLI
echo "[2/5] Installing PlatformIO..."
if ! command -v pio &> /dev/null; then
    curl -fsSL -o /tmp/get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
    python3 /tmp/get-platformio.py
    export PATH="$HOME/.platformio/penv/bin:$PATH"
    if ! grep -q '.platformio/penv/bin' ~/.bashrc; then
        echo 'export PATH="$HOME/.platformio/penv/bin:$PATH"' >> ~/.bashrc
    fi
    echo "PlatformIO installed: $(pio --version)"
else
    echo "PlatformIO already installed: $(pio --version)"
fi

# Disable serial console
echo "[3/5] Disabling serial console..."
CMDLINE="/boot/firmware/cmdline.txt"
if grep -q 'console=serial0' "$CMDLINE"; then
    sudo sed -i 's/console=serial0,[0-9]* //g' "$CMDLINE"
    echo "Serial console disabled in $CMDLINE"
    REBOOT_NEEDED=1
else
    echo "Serial console already disabled"
fi

# Enable PL011 UART on GPIO14/15 (disable Bluetooth UART)
echo "[4/5] Configuring PL011 UART..."
CONFIG="/boot/firmware/config.txt"
if ! grep -q 'dtoverlay=disable-bt' "$CONFIG"; then
    echo "" | sudo tee -a "$CONFIG"
    echo "# Disable Bluetooth to free PL011 UART for Luos RS485" | sudo tee -a "$CONFIG"
    echo "dtoverlay=disable-bt" | sudo tee -a "$CONFIG"
    echo "PL011 UART enabled on GPIO14/15"
    REBOOT_NEEDED=1
else
    echo "PL011 UART already configured"
fi

# Disable bluetooth service
echo "[5/5] Disabling bluetooth service..."
sudo systemctl disable hciuart.service 2>/dev/null || true
sudo systemctl disable bluetooth.service 2>/dev/null || true

echo ""
echo "=== Setup Complete ==="
if [ "${REBOOT_NEEDED:-0}" = "1" ]; then
    echo "*** REBOOT REQUIRED for UART changes to take effect ***"
    echo "Run: sudo reboot"
else
    echo "No reboot needed. Ready to build."
fi
