#!/bin/bash
#
systemd_user_dir="$HOME/.local/share/systemd/user"
systemd_user_service_name="ucan-monitor.service"

echo "Starting uninstallation"

echo "Disabling $systemd_user_service_name"
systemctl --user disable "$systemd_user_service_name"
systemctl --user daemon-reload

if [ -f "$systemd_user_dir/$systemd_user_service_name" ]
then 
  rm "$systemd_user_dir/$systemd_user_service_name"
  echo "Removed '$systemd_user_dir/$systemd_user_service_name'"
else
  echo "'$systemd_user_dir/$systemd_user_service_name' does not exist, skipping"
fi

systemd_system_dir="/etc/systemd/system"
systemd_system_service_name="ucan-cansocket.service"

cansocket_system_script_dir="/usr/sbin"
cansocket_system_script="$cansocket_system_script_dir/socketcan_enable.sh"

if [ -f "$cansocket_system_script" ]
then
  sudo rm "$cansocket_system_script"
  echo "Removed '$cansocket_system_script'"
else
  echo "'$cansocket_system_script' does not exist, skipping"
fi

echo "Disabling $systemd_system_service_name"
sudo systemctl disable "$systemd_system_service_name"
sudo systemctl daemon-reload

if [ -f "$systemd_system_dir/$systemd_system_service_name" ]
then
  sudo rm "$systemd_system_dir/$systemd_system_service_name"
  echo "Removed '$systemd_system_dir/$systemd_system_service_name'"
else
  echo "'$systemd_system_dir/$systemd_system_service_name' does not exixst, skipping"
fi

project_dir="$(cd "$(dirname "$0")/../.." && pwd)"
echo "Removing project directory"
cd "$project_dir/.."
rm -rf "$project_dir"

echo "Uninstallation complete!"
exit 0
