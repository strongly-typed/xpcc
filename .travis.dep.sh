# Preparing the cache
if [ ! -f "$HOME/cache/avr-gcc.tar.bz2" ]; then
	echo "Downloading AVR toolchain..."
	(cd $HOME/cache && wget -q https://github.com/strongly-typed/build-atmel-avr-gnu-toolchain/releases/download/v3.6.1/avr-gcc.tar.bz2) &
fi
if [ ! -f "$HOME/cache/cortex-m.tar.bz2" ]; then
	echo "Downloading Cortex-M toolchain..."
	(cd $HOME/cache && wget -q https://developer.arm.com/-/media/Files/downloads/gnu-rm/6-2017q2/gcc-arm-none-eabi-6-2017-q2-update-linux.tar.bz2 -O cortex-m.tar.bz2) &
fi
if [ ! -f "$HOME/cache/boost.tar.bz2" ]; then
	echo "Downloading libboost package..."
	(cd $HOME/cache && wget -q http://box.xpcc.io/boost.tar.bz2) &
fi
# wait for all downloads to finish
wait
echo "Downloads done."

# unzip all toolchains
mkdir $HOME/toolchain
echo "Expanding AVR toolchain..."
(cd $HOME/toolchain && tar -xjf $HOME/cache/avr-gcc.tar.bz2) &
echo "Expanding Cortex-M toolchain..."
(cd $HOME/toolchain && tar -xjf $HOME/cache/cortex-m.tar.bz2) &
echo "Expanding boost package..."
(cd $HOME/toolchain && tar -xjf $HOME/cache/boost.tar.bz2) &
# synchronize
wait
echo "Expanding done."
