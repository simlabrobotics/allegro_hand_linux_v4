Allegro Hand Linux Project
==========================
Note: This project works only for PEAK System CAN interface (chardev) for USB: PCAN-USB

Download
========
File:Allegro hand linux.zip

Git Repo
https://github.com/simlabrobotics/allegro_hand_linux


Build and Run: "grasp"
======================

1. Download, build, and install PCAN-USB driver for Linux: libpcan

tar -xzvf peak-linux-driver-x.x.tar.gz
cd peak-linux-driver-x.x
make NET=NO
sudo make install

2. Download, build, and install PCAN-Basic API for Linux: libpcanbasic

tar -xzvf PCAN_Basic_Linux-x.x.x.tar.gz
cd PCAN_Basic_Linux-x.x.x/pcanbasic
make
sudo make install

3. Download, build, and install Grasping Library for Linux, "libBHand": Grasping_Library_for_Linux

4. Build Allegro Hand Project using cmake "out of source build" style.

unzip AllegroHand.zip
cd AllegroHand
mkdir build
cd build
cmake ..
make
make install

Note: You will need to replace the encoder offsets and directions and the motor directions in the array at the top of the main.cpp file. These offsets can be found on the offsets and directions table on your Allegro Hand Wiki page (front page - scroll down): Allegro_Hand_DML_Info

Note: Using cmake "out of source build" style, the entire build tree is created under "build" directory so that you can delete "build" directory without worrying about the sources.

5. Connect PCAN-USB and Allegro Hand (make sure to power off Allegro Hand)

6. Start the grasping program: "grasp"

bin/grasp

7. Power on Allegro Hand.

8. Use keyboard command to move Allegro Hand!!!!

================================================
If you have any questions, feel free to ask.

support@simlab.co.kr
