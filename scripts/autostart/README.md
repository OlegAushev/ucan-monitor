cp ucan-cansocket.service /etc/systemd/system
cp socketcan_enable.sh /usr/sbin
cd /usr/sbin
chmod a+x socketcan_enable.sh
cd-
#
systemctl daemon-reload
systemctl enable ucan-cansocket.service
#
mkdir -p $HOME/.local/share/systemd/user
cp ucan-monitor.service $HOME/.local/share/systemd/user
cp start-ucan-monitor.sh $HOME
cd $HOME
chmod a+x start-ucan-monitor.sh
cd -
#
systemctl --user daemon-reload
systemctl --user enable ucan-cansocket.service


- edit start-ucan-monitor.sh and ucan-monitor.desktop
- place start-ucan-monitor.sh in home directory
- place ucan-monitor.desktop in ~/.config/autostart
