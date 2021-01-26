source /opt/Xilinx/petalinux/2018.2/settings.sh
export CROSS_COMPILE=aarch64-linux-gnu-
export ARCH=aarch64

#make xilinx_zynqmp_edpu_revB_defconfig
make xilinx_zynqmp_m9000_defconfig
make -j8
