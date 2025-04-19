# Scroll Space
**Scroll Space** is a fast-paced, 2D infinite scroller for the original Game Boy (the "DMG" from 1989) with procedurally generated maps and destructible terrain. The goal is to avoid obstacles and destroy mines in order to collect the most points and beat your high score. The game features:

* two weapons: bullets and bombs
* pickup items: health packs and shields
* randomly generated maps
* destructible terrain
* progressively faster and more challenging maps
* 2 unlockable difficulty levels with different scroll speeds
* a special bonus after beating all difficulty levels

The game was written in C using GDBK with original art and music. The music was created in hUGETracker and runs with the hUGEDriver library. The source code can be found in this GitHub repo.

The background art of the intro scene was inspired by Wario Land: Super Mario Land 3 on the Game Boy.

Note: Due to the fast-paced nature of the game, for best controls we recommend playing on a Game Boy compatible device (including original hardware and the Analogue Pocket), or on an emulator with a controller.

# Controls on Game Boy
* D-pad: move ship
* A: shoot bullets
* B: drop bomb
* START: pause game
* SELECT: switch score between points and time played

# Compilation Instructions
Required tools:
* macOS, Linux, or Windows with WSL
* gbdk-2020 v4.2.0 or higher
* hUGEDriver v6.13 or higher

1. Clone gbdk-2020 and hUGEDriver to /opt/gbdk/ and /opt/hUGEDriver-6/
``` bash
cd /opt
git clone https://github.com/gbdk-2020/gbdk-2020.git gbdk
git clone https://github.com/SuperDisk/hUGEDriver.git hUGEDriver-6
```
2. Clone this repo
``` bash
git clone https://github.com/cailangames/scrollspace.git
cd scrollspace
```
3. Compile using `make`
``` bash
# Make .gb file
make gb

# Make .pocket file
make pocket
```