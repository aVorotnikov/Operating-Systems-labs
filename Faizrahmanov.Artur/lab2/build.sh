mkdir build
cd build

apt install qml-module-qtquick-window2
apt install qml-module-qtquick-layouts
apt install qml-module-qtquick-controls2

cmake ..
make -j2
mv host_sock host_mq host_fifo ..
cd ..
rm -r build
