# jamp _*OG*_
An engine that patches basejka at runtime to add new features or bug fixes as needed.

Only supports linux i386.

### building
```shell
# For APT users: install build deps
sudo apt install $(make apt-build-deps)
# build & install to /usr/local/bin/jampog
make && sudo make install
```

### installing assets (using jkpak)
```shell
jkpak set-install-path ~/.local/share/jampog/base/
# install jampgamei386.so & libcxa.so.1
jkpak install basejka
# install assets pk3s via steam
jkpak install steam-assets
```

### setup as a systemd user service
```shell
git clone https://github.com/jampio/jampog-service
cd jampog-service && make install
sudo loginctl enable-linger
systemctl enable jampog --user
systemctl start jampog --user
```

### included fixes
* gc (out of bounds index) (Cmd_GameCommand_f)
* invalid forcepowers userinfo string crash (BG_LegalizedForcePowers)
* team follow1/follow2/score/scoreboard patched (SetTeam) (should prevent follow1 cycle crash when 0 clients)
* q3msgboom (SV_SendServerCommand) (via openjk)
* client commands filtered from control characters (sv_filterCommands) (via openjk)

### new features
* black names are allowed
* improved dueling (multiduel, removed distance limit, improved duel messages)
* duel interference like in JA+
* Admin

### new server cvars
* g_duelHealth (default: 100)
* g_duelArmor (default: 100)
