#!/bin/bash
#
systemd_user_dir="$HOME/.local/share/systemd/user"
systemd_user_service_name="ucan-monitor.service"

echo "Disabling $systemd_user_service_name..."
systemctl --user disable "$systemd_user_service_name"
systemctl --user daemon-reload

rm "$systemd_user_dir/$systemd_user_service_name"
echo "Removed '$systemd_user_dir/$systemd_user_service_name'"

systemd_system_dir="/etc/systemd/system"
systemd_system_service_name="ucan-cansocket.service"

cansocket_system_script_dir="/usr/sbin"
cansocket_system_script="$cansocket_system_script_dir/socketcan_enable.sh"

echo "Uninstalling CAN socket script..."
sudo rm "$cansocket_system_script"
echo "Removed '$cansocket_system_script'"

echo "Disabling $systemd_system_service_name..."
sudo systemctl disable "$systemd_system_service_name"
sudo systemctl daemon-reload

sudo rm "$systemd_system_dir/$systemd_system_service_name"
echo "Removed '$systemd_system_dir/$systemd_system_service_name'"

echo "Done"
exit 0
