# jampog
The project aims to support vanilla basejka without recompiling by means of runtime patching and/or intervening of syscalls.
Only `linux i386` will be supported for the time being.

### building
```shell
# install build deps (only for host os == Ubuntu/Debian amd64)
sudo apt build-dep .
# install build deps (only for host os == Ubuntu/Debian i386)
sudo apt install cmake
# clone repo
git clone https://github.com/jampio/jampog
cd jampog
# build
make
# will install to /usr/local/bin/jampog
sudo make install
```

### included fixes
* gc (out of bounds index) (Cmd_GameCommand_f)
* invalid forcepowers userinfo string crash (BG_LegalizedForcePowers)
* team follow1/follow2/score/scoreboard patched (SetTeam) (should prevent follow1 cycle crash when 0 clients)
* q3msgboom (SV_SendServerCommand) (via openjk)
* client commands filtered from control characters (sv_filterCommands) (via openjk)

### new features
* black names are allowed
* multiduel
