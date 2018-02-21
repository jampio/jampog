# jamp _*OG*_
An engine that patches basejka at runtime to add new features or bug fixes as needed.

Only supports linux i386.

### building
```shell
git clone https://github.com/jampio/jampog
cd jampog
sudo make build-deps && make && make install
```

### setup
```shell
sudo adduser jampog
sudo make game-deps USER=jampog
make assets USER=jampog
git clone https://github.com/jampio/jampog-service
cd jampog-service
sudo make install USER=jampog
sudo loginctl enable-linger jampog
sudo su - jampog
systemctl --user enable jampog
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
