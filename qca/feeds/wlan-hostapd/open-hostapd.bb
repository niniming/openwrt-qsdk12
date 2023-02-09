SUMMARY = "hostapd daemon"
LICENSE = "BSD-3-Clause"
SECTION = "network"
LIC_FILES_CHKSUM = "file://COPYING;md5=279b4f5abb9c153c285221855ddb78cc"
PKG_NAME = "hostapd"
PV = "2_10"

FILESEXTRAPATHS_append := "${THISDIR}/package/network/services/hostapd/patches:"
FILESEXTRAPATHS_append := "${THISDIR}/package/network/services/hostapd/files:"

SRC_URI = "git://w1.fi/hostap.git;nobranch=1 \
	file://hostapd-mesh.config \
	file://wpa_supplicant-mesh.config \
"

SRCREV = "b26f5c0fe35cd0472ea43f533b981ac2d91cdf1f"

FILES_${PN} += "/usr/sbin/*"

S = "${WORKDIR}/git"
inherit pkgconfig

DEPENDS = "libnl pkgconfig-native"
DEPENDS += "libubox"
DEPENDS += "openssl"

LC_ALL = "C"

do_patch() {
        ls ${THISDIR}/package/network/services/hostapd/patches/ | xargs -I % sh -c 'echo "applying patch "%;patch -d${S} -f -p1 < ${THISDIR}/package/network/services/hostapd/patches/%'
}

do_compile() {
	cp ${WORKDIR}/hostapd-mesh.config ${S}/${PKG_NAME}/.config
	sed -i '/CONFIG_UBUS=y/d' ${S}/${PKG_NAME}/.config
	sed -i '/CONFIG_INTERNAL_AES=y/d' ${S}/${PKG_NAME}/.config
	sed -i 's/CONFIG_TLS=internal/CONFIG_TLS=openssl/g' ${S}/${PKG_NAME}/.config
	cp ${WORKDIR}/wpa_supplicant-mesh.config ${S}/wpa_supplicant/.config
	sed -i '/CONFIG_TLS=internal/d' ${S}/wpa_supplicant/.config
	echo 'CONFIG_CTRL_IFACE_MIB=y' >> ${S}/wpa_supplicant/.config
	echo 'CONFIG_IEEE80211AX=y' >> ${S}/wpa_supplicant/.config
	make V=s  -C ${S}/${PKG_NAME}
	make V=s  -C ${S}/wpa_supplicant
}

do_install() {
	install -m 0755 -d ${D}/usr/sbin
	cp ${S}/${PKG_NAME}/hostapd ${D}/usr/sbin/
	cp ${S}/${PKG_NAME}/hostapd_cli ${D}/usr/sbin/
	cp ${S}/wpa_supplicant/wpa_supplicant ${D}/usr/sbin/
	cp ${S}/wpa_supplicant/wpa_cli ${D}/usr/sbin/
}
