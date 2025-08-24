#!/bin/bash
#
set -euo pipefail

# Function to detect Raspberry Pi
is_raspberry_pi() {
  # Check /proc/device-tree/model (newer RPi)
  if [ -f /proc/device-tree/model ]; then
    if grep -qi "raspberry" /proc/device-tree/model; then
      return 0
    fi
  fi

  # Fallback: check /proc/cpuinfo for older RPi models
  if grep -qE "BCM|Raspberry Pi" /proc/cpuinfo 2>/dev/null; then
    return 0
  fi

  return 1
}

project_dir="$(cd "$(dirname "$0")/../.." && pwd)"
build_dir="$project_dir/build"
bin_target="$project_dir/bin/ucan-monitor"

echo "Starting installation"
echo "Project root is '$project_dir'"

# --- Build ---
if [ -f "$bin_target" ]
then
    echo "Binary already exists, build skipped"
else
  mkdir -p "$build_dir"

  echo "Starting build..."
  (
    cd "$build_dir"
    cmake .. -DMULTI_SERVER=off -DSERVER=moyka-dashboard-2 && make -j$(nproc)
  ) > build.log 2>&1 &

  build_pid=$!
  spinner="/-\|"
  i=0
  while kill -0 $build_pid 2>/dev/null
  do
    i=$(( (i+1) %4 ))
    printf "\rBuilding... ${spinner:$i:1}"
    sleep 0.2
  done

  wait $build_pid || { echo "Build failed. See $build_dir/build.log"; exit 1; }
  printf "\rBuild finished successfully!    \n"
fi

# --- Sources cleanup ---
pushd "$project_dir" > /dev/null
echo "Cleaning project tree..."
shopt -s extglob
rm -rf !(bin|assets|scripts)
rm -rf .[^.] .??*             # Delete all hidden files and dirs
echo "Cleanup done"
popd > /dev/null

# --- User systemd service setup ---
systemd_user_dir="$HOME/.local/share/systemd/user"
systemd_user_service_name="ucan-monitor.service"
ucan_monitor_service_template="$(dirname "$0")/ucan-monitor.service.in"
ucan_monitor_service="$(dirname "$0")/$systemd_user_service_name"

sed "s|__project_dir__|$project_dir|g" "$ucan_monitor_service_template" > "$ucan_monitor_service"
echo "Generated $systemd_user_service_name unit file: '$ucan_monitor_service'"

echo "Checking '$systemd_user_dir'..."
if [ ! -d "$systemd_user_dir" ]
then
  mkdir -p "$systemd_user_dir"
  echo "Created '$systemd_user_dir'"
else
  echo "'$systemd_user_dir' already exists"
fi

cp "$ucan_monitor_service" "$systemd_user_dir"
echo "Copied $systemd_user_service_name unit file to '$systemd_user_dir'"

echo "Enabling $systemd_user_service_name..."
systemctl --user daemon-reload
systemctl --user enable "$systemd_user_service_name"

# --- System systemd service setup ---
systemd_system_dir="/etc/systemd/system"
systemd_system_service_name="ucan-cansocket.service"
ucan_cansocket_service="$(dirname "$0")/$systemd_system_service_name"

cansocket_system_script_dir="/usr/sbin"
cansocket_enable_script="$(dirname "$0")/../socketcan_enable.sh"
cansocket_system_script="$cansocket_system_script_dir/socketcan_enable.sh"

echo "Installing CAN socket script to '$cansocket_system_script_dir'..."
sudo cp "$cansocket_enable_script" "$cansocket_system_script_dir"
sudo chmod +x "$cansocket_system_script"
echo "Installed '$cansocket_system_script' and made it executable"

sudo cp "$ucan_cansocket_service" "$systemd_system_dir"
echo "Copied $systemd_system_service_name unit file to '$systemd_system_dir'"

echo "Enabling $systemd_system_service_name..."
sudo systemctl daemon-reload
sudo systemctl enable "$systemd_system_service_name"

# --- SPI-CAN converter setup for Raspberry Pi ---
if is_raspberry_pi
then
  firmware_config_file="/boot/firmware/config.txt"
  firmware_configuration_marker="Custom config for SPI-CAN converter"
  firmware_custom_configuration=$(cat <<EOF
#
# $firmware_configuration_marker
dtoverlay=mcp2515-can0,oscillator=12000000,interrupt=12
dtoverlay=spi-bcm2835-overlay
EOF
)

  echo "Configuring SPI-CAN..."

  # Create file if it doesn't exist
  if [ ! -f "$firmware_config_file" ]
  then
    sudo mkdir -p "$(dirname "$firmware_config_file")"
    sudo touch "$firmware_config_file"
    echo "Created '$firmware_config_file'"
  fi

  # Append custom configuration if not already present
  if ! grep -Fq "$firmware_configuration_marker" "$firmware_config_file"
  then
    echo "$firmware_custom_configuration" | sudo tee -a "$firmware_config_file" > /dev/null
    echo "Added $firmware_configuration_marker to '$firmware_config_file'"
  else
    echo "$firmware_configuration_marker already present in '$firmware_config_file'"
  fi

else
  echo "Raspberry Pi is not detected, skipping SPI-CAN configuration"
fi

echo "Installation complete!"
exit 0
