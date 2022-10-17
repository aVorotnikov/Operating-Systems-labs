PID_FILE_PATH="/var/run/lab1_10.pid"
BUILD_DIR="build"
EXECUTABLE_NAME="lab1_10"

# create pid file
sudo touch $PID_FILE_PATH
sudo chmod 666 $PID_FILE_PATH

# build proj
mkdir $BUILD_DIR
cd $BUILD_DIR
cmake ..
make

cd ..
cp $BUILD_DIR/$EXECUTABLE_NAME .
rm -r $BUILD_DIR
