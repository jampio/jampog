#!/bin/bash
echo "User: " $USER
wget http://media.steampowered.com/client/steamcmd_linux.tar.gz -O /tmp/steamcmd.tar.gz &&
mkdir -p /tmp/steamcmd &&
tar -xvzf /tmp/steamcmd.tar.gz -C /tmp/steamcmd &&
read -s -p "Enter Steam Username: " STEAM_USER &&
echo &&
read -s -p "Enter Steam Password: " STEAM_PASS &&
echo &&
/tmp/steamcmd/steamcmd.sh \
	+@sSteamCmdForcePlatformType windows \
	+login $STEAM_USER $STEAM_PASS \
	+force_install_dir /tmp/SteamJKA \
	 +app_update 6020 validate +quit
STEAM_PATH=/tmp/SteamJKA/GameData/base
INSTALL_PATH=/home/$USER/.local/share/jampog/base/
sudo mkdir -p $INSTALL_PATH
[ -f $STEAM_PATH/assets0.pk3 ] && sudo cp $STEAM_PATH/assets0.pk3 $INSTALL_PATH
[ -f $STEAM_PATH/assets1.pk3 ] && sudo cp $STEAM_PATH/assets1.pk3 $INSTALL_PATH
[ -f $STEAM_PATH/assets2.pk3 ] && sudo cp $STEAM_PATH/assets2.pk3 $INSTALL_PATH
[ -f $STEAM_PATH/assets3.pk3 ] && sudo cp $STEAM_PATH/assets3.pk3 $INSTALL_PATH
