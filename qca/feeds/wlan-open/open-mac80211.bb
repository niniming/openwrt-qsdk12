SUMMARY = "cfg80211 interface configuration utility"
LICENSE = "GPL-2.0 WITH Linux-syscall-note"
SECTION = "console/network"
LIC_FILES_CHKSUM = "file://${DL_DIR}/mac80211-kernel/COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"
PKG_NAME = "kernel"
SRC_URI = "https://git.kernel.org/pub/scm/linux/kernel/git/firmware/linux-firmware.git/snapshot/linux-firmware-8afadbe.tar.gz"

KERNEL_VERSION = "/5.4.164+yocto"
PKG_KERNEL_SRC_URL = "https://git.codelinaro.org/clo/qsdk/kvalo/ath.git"
PKG_KERNEL_VERSION = "f40abb4788"
SRC_URI[md5sum] = "21113fbd338eed6070c7f9b4c8939b43"
#SRCREV = "8afadbe553017bec1e442b5a9fba859b54fd33fb"
V_mac80211_config =" \
	\nCPTCFG_CFG80211=m \
	\nCPTCFG_MAC80211=m \
	\nCPTCFG_ATH_CARDS=m \
	\nCPTCFG_ATH_COMMON=m \
	\nCPTCFG_ATH11K=m \
	\nCPTCFG_ATH11K_PCI=m \
	\nCPTCFG_ATH11K_AHB=m \
	\nCPTCFG_ATH12K=m \
	\nCPTCFG_WLAN=y \
	\nCPTCFG_NL80211_TESTMODE=y \
	\nCPTCFG_CFG80211_WEXT=y \
	\nCPTCFG_CFG80211_INTERNAL_REGDB=y \
	\nCPTCFG_CFG80211_CERTIFICATION_ONUS=y \
	\nCPTCFG_MAC80211_RC_MINSTREL=y \
	\nCPTCFG_MAC80211_RC_MINSTREL_HT=y \
	\nCPTCFG_MAC80211_RC_DEFAULT_MINSTREL=y \
	\nCPTCFG_MAC80211_MESH=y \
	\nCPTCFG_CFG80211_DEBUGFS=y \
	\nCPTCFG_MAC80211_DEBUGFS=y \
	\nCPTCFG_ATH9K_DEBUGFS=y \
	\nCPTCFG_ATH9K_HTC_DEBUGFS=y \
	\nCPTCFG_ATH10K_DEBUGFS=y \
	\nCPTCFG_ATH11K_DEBUGFS=y \
	\nCPTCFG_ATH12K_DEBUGFS=y \
	\nCPTCFG_ATH10K_PKTLOG=y \
	\nCPTCFG_ATH11K_PKTLOG=y \
	\nCPTCFG_ATH12K_PKTLOG=y \
	\nCPTCFG_ATH_DEBUG=y \
	\nCPTCFG_ATH10K_DEBUG=y \
	\nCPTCFG_ATH11K_DEBUG=y \
	\nCPTCFG_ATH12K_DEBUG=y \
	\nCPTCFG_HWMON=y \
	\nCPTCFG_ATH9K_PCI=y \
	\nCPTCFG_ATH_USER_REGD=y \
	\nCPTCFG_ATH_REG_DYNAMIC_USER_REG_HINTS=y \
	\nCPTCFG_ATH_REG_DYNAMIC_USER_CERT_TESTING=y \
	\nCPTCFG_ATH11K_SPECTRAL=y \
	\nCPTCFG_ATH11K_CFR=y \
	\nCPTCFG_MAC80211_LEDS=y \
"

addtask download before do_unpack after do_fetch
PKG_BACKPORTS_SOURCE_URL = "https://git.kernel.org/pub/scm/linux/kernel/git/backports/backports.git"
PKG_BACKPORTS_VERSION = "83f664bbc583"
PKG_VERSION = "20220404"
SUBDIR = "backports-${PKG_VERSION}-5.4.164-${PKG_KERNEL_VERSION}"
DEPENDS += "linux-ipq libnl pkgconfig-native"
LINUX_SRC = "tmp/work/aarch64-poky-linux/linux-libc-headers/5.4-r0/linux-5.4/"

T_DIR = "${TOPDIR}/target-aarch64_cortex-a53+neon-vfpv4_musl/linux-ipq95xx_64/${SUBDIR}-default/${SUBDIR}"
P_DIR = "${TOPDIR}/../poky/meta-ipq/recipes-openwifi/wlan-open/mac80211/patches"
I_DIR = "${TOPDIR}/../poky/meta-ipq/recipes-openwifi/wlan-open/"
CC_remove = "-fstack-protector-strong  -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security"
KBUILD_CFLAGS += "-Wno-error=implicit-function-declaration"
LC_ALL = "C"


do_download() {
	if [ ! -f ${DL_DIR}/${SUBDIR}.tar.bz2 ];then
	git clone ${PKG_KERNEL_SRC_URL} ${DL_DIR}/mac80211-kernel || (rm -rf ${DL_DIR}/mac80211-kernel &&  git clone ${PKG_KERNEL_SRC_URL} ${DL_DIR}/mac80211-kernel)
	(cd ${DL_DIR}/mac80211-kernel;git remote add src ${PKG_KERNEL_SRC_URL}; git fetch src)

	(cd  ${DL_DIR}/mac80211-kernel&& git checkout ${PKG_KERNEL_VERSION} && \
	git revert --no-edit db3bdcb9 && \
	git revert --no-edit c8ffcd122 && git revert --no-edit 48a54f6bc && \
	git revert --no-edit b55379e34 && git revert --no-edit 47c662486 && \
	git revert --no-edit 36b0cefe9 && git revert --no-edit dfcb63ce && \
	git revert --no-edit e72aeb9e && git revert --no-edit de1352ead8 && \
	git revert --no-edit fb5f6a0e && git revert --no-edit 74624944)

	git clone ${PKG_BACKPORTS_SOURCE_URL} ${DL_DIR}/mac80211-source  ||
        (rm -rf ${DL_DIR}/mac80211-source && git  clone ${PKG_BACKPORTS_SOURCE_URL} ${DL_DIR}/mac80211-source)
	cp ${TOPDIR}/../poky/meta-ipq/recipes-openwifi/wlan-open/mac80211/files/copy-list.ath ${DL_DIR}/mac80211-source
	(cd  ${DL_DIR}/mac80211-source && git checkout ${PKG_BACKPORTS_VERSION} && git revert --no-edit 49432781 &&./gentree.py --clean --copy-list ./copy-list.ath ${DL_DIR}/mac80211-kernel ${DL_DIR}/${SUBDIR})
	echo "before making tar"
	(cd ${DL_DIR}; if [ -z " tar --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name ${TAR_TIMESTAMP:+--mtime="$TAR_TIMESTAMP"} -c ${SUBDIR} |  bzip2 -c > ${DL_DIR}/${SUBDIR}.tar.bz2" ]; then bzip2 -c > ${DL_DIR}/${SUBDIR}.tar.bz2; else : ; fi; tar --numeric-owner --owner=0 --group=0 --mode=a-s --sort=name ${TAR_TIMESTAMP:+--mtime="$TAR_TIMESTAMP"} -c ${SUBDIR} | bzip2 -c > ${DL_DIR}/${SUBDIR}.tar.bz2)

	else
	echo "Skipping backports download"
	fi;

	if [ -d ${T_DIR} ]; then
		rm -rf ${T_DIR}/
	fi;
	mkdir -p ${T_DIR}/
	bzcat ${DL_DIR}/${SUBDIR}.tar.bz2 | tar -C ${T_DIR}/.. -xf -

}

do_patch() {
	ls ${I_DIR}/mac80211/patches | xargs -I % sh -c 'patch -d${T_DIR} -f -p1 < ${I_DIR}/mac80211/patches/%'
	find ${T_DIR}/ '(' -name '*.orig' -o -name '.*.orig' ')' -exec rm -f {} \;
	tar -C ${T_DIR} -xf ${DL_DIR}/linux-firmware-8afadbe.tar.gz
	rm -rf ${T_DIR}/include/linux/ssb ${T_DIR}/include/linux/bcma ${T_DIR}/include/net/bluetooth
	rm -rf ${T_DIR}/include/linux/cordic.h  ${T_DIR}/include/linux/crc8.h ${T_DIR}/include/linux/eeprom_93cx6.h ${T_DIR}/include/linux/wl12xx.h ${T_DIR}/include/linux/spi/libertas_spi.h ${T_DIR}/include/net/ieee80211.h

}

do_compile() {
	echo "${V_mac80211_config}" > ${T_DIR}/.config
	cat ${T_DIR}/.config
	make  -C ${T_DIR} CC="gcc" LD="${LD}"  ARCH="arm64" EXTRA_CFLAGS="-I${T_DIR}/include/ -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Wno-int-conversion -Wno-implicit-function-declaration" MODPROBE=true     KLIB=/lib/modules/5.4.164  KLIB_BUILD=${TOPDIR}/tmp/work/ipq95xx_64-poky-linux/linux-ipq/5.4-r0/build KERNEL_SUBLEVEL=4 KBUILD_LDFLAGS_MODULE_PREREQ= olddefconfig
	rm -rf ${T_DIR}/modules
make -C ${T_DIR} CC="${CC}" LD="${LD}"  ARCH="arm64" EXTRA_CFLAGS="-I${T_DIR}/include -Wall -Werror -Wno-incompatible-pointer-types -Wno-unused-variable -Wno-discarded-qualifiers -Wno-int-conversion -Wno-implicit-fallthrough" KLIB=/lib/modules/5.4.164  KLIB_BUILD=${TOPDIR}/tmp/work/ipq95xx_64-poky-linux/linux-ipq/5.4-r0/build KERNEL_SUBLEVEL=4 KBUILD_LDFLAGS_MODULE_PREREQ= modules
	cp ${T_DIR}/compat/compat.ko ${T_DIR}/drivers/net/wireless/ath/ath12k/ath12k.ko  ${T_DIR}/drivers/net/wireless/ath/ath11k/ath11k.ko ${T_DIR}/drivers/net/wireless/ath/ath11k/ath11k_ahb.ko ${T_DIR}/drivers/net/wireless/ath/ath11k/ath11k_pci.ko ${T_DIR}/drivers/net/wireless/ath/ath.ko ${T_DIR}/net/mac80211/mac80211.ko ${T_DIR}/net/wireless/cfg80211.ko ./


}

do_install() {
	install -m 0755 -d ${D}${base_libdir}/modules${KERNEL_VERSION}/kernel/drivers/mac80211
	install -m 0644 *.ko ${D}${base_libdir}/modules${KERNEL_VERSION}/kernel/drivers/mac80211
	install -d ${D}${includedir}/open-mac80211
}

FILES_${PN} += "/lib \
/lib/modules \
/lib/modules/5.4.164+yocto \
/lib/modules/5.4.164+yocto/kernel \
/lib/modules/5.4.164+yocto/kernel/drivers \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211 \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/ath.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/ath11k.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/ath11k_ahb.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/ath11k_pci.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/ath12k.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/cfg80211.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/compat.ko \
/lib/modules/5.4.164+yocto/kernel/drivers/mac80211/mac80211.ko \
"
S = "${WORKDIR}/git/${PKG_NAME}"
inherit pkgconfig
