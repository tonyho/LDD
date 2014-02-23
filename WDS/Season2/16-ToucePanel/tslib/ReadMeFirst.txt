sudo apt-get install autoconf
sudo apt-get install automake
sudo apt-get install libtool




编译：
tar xzf tslib-1.4.tar.gz
cd tslib
./autogen.sh 

mkdir tmp
echo "ac_cv_func_malloc_0_nonnull=yes" >arm-linux.cache
./configure --host=arm-linux --cache-file=arm-linux.cache --prefix=$(pwd)/tmp ##For TQ2440 here is not the arm-none-linux-gnueabi but the arm-linux, Ref the busybox compile
make
make install

安装：
cd tmp
cp * -rf /nfsroot

使用：
先安装s3c_ts.ko, lcd.ko

1．
修改 /etc/ts.conf第1行(去掉#号和第一个空格)：
# module_raw input
改为：
module_raw input

2．In serial Console
export TSLIB_TSDEVICE=/dev/event0 ##eventN is detected by insmod the input.ko like tq_ts.ko
export TSLIB_CALIBFILE=/etc/pointercal
export TSLIB_CONFFILE=/etc/ts.conf
export TSLIB_PLUGINDIR=/lib/ts
export TSLIB_CONSOLEDEVICE=none
export TSLIB_FBDEVICE=/dev/fb0

ts_calibrate

ts_test
