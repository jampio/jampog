#!/bin/bash
mkdir -p /home/$USER/.local/share/jampog/base/ &&
apt install steamcmd &&
read -s -p "Enter Steam Username: " STEAM_USER &&
echo &&
read -s -p "Enter Steam Password: " STEAM_PASS &&
echo &&
/usr/games/steamcmd \
	+@sSteamCmdForcePlatformType windows \
	+login $STEAM_USER $STEAM_PASS \
	+force_install_dir /tmp/SteamJKA \
	 +app_update 6020 validate +quit &&
mv /tmp/SteamJKA/GameData/base/assets0.pk3 /home/$USER/.local/share/jampog/base/ &&
mv /tmp/SteamJKA/GameData/base/assets1.pk3 /home/$USER/.local/share/jampog/base/ &&
mv /tmp/SteamJKA/GameData/base/assets2.pk3 /home/$USER/.local/share/jampog/base/ &&
mv /tmp/SteamJKA/GameData/base/assets3.pk3 /home/$USER/.local/share/jampog/base/ &&
rm -rf /tmp/SteamJKA
