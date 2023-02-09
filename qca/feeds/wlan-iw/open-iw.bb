SUMMARY = "cfg80211 interface configuration utility"
HOMEPAGE = "http://wireless.kernel.org/en/users/Documentation/iw"
LICENSE = "ISC"
SECTION = "console/network"
LIC_FILES_CHKSUM = "file://COPYING;md5=878618a5c4af25e9b93ef0be1a93f774"

PKG_NAME = "iw"
PKG_VERSION = "5.16"


FILESEXTRAPATHS_append := "${THISDIR}/package/network/utils/iw/patches:"
SRC_URI = "https://www.kernel.org/pub/software/network/iw/${PKG_NAME}-${PKG_VERSION}.tar.xz"
SRC_URI +=" \
	file://001-nl80211_h_sync.patch \
	file://120-antenna_gain.patch \
	file://200-reduce_size.patch \
	file://502-Add-channel-attribure-support-for-nl80211-message.patch \
	file://505-Add-user-command-for-tid-specific-retry-count.patch \
	file://506-Add-user-command-for-tid-specific-aggr-conf.patch \
	file://507-Add-peer-address-in-noack-map-command \
	file://510-iw-wifi-config-vendor.patch \
	file://512-iw-add-wide-band-scan-support.patch \
	file://517-iw-support-optional-argument-to-specify-6Ghz-channel.patch \
	file://519-iw-Add-HE-UL-MU-fixed-rate-setting.patch \
	file://521-iw-Add-aggregation-count-and-management-rtscts-control-support.patch \
	file://522-iw-Add-MLO-support-in-interface-creation.patch \
	file://523-iw-fix-compile-issues.patch \
	file://601-iw-update-nl80211.h.patch \
	file://602-iw-Support-EHT-rate-configuration.patch \
	file://603-iw-Add-320-MHz-support.patch \
	file://603-iw-Add-EHT-Capability-Information.patch \
	file://604-iw-Add-support-to-set-static-puncturing-pattern.patch \
	file://604-iw-Add-the-support-for-green-ap.patch \
	file://605-iw-add-support-for-6-GHz-in-iw-reg-get-command.patch \
	file://606-iw-fix-wrong-information-display-for-EHT-PHY-CAPABIL.patch \
	file://607-Changes-needed-for-yocto.patch \
"

SRC_URI[md5sum] = "782a3460da2854bd2e5b8f96845a62f8"
S = "${WORKDIR}/${PKG_NAME}-${PKG_VERSION}"
inherit pkgconfig

DEPENDS += "libnl"
TARGET_CFLAGS = "-fpie -Wall -Werror -O2 "
TARGET_LDFLAGS = "-pie"
EXTRA_OEMAKE = ""

do_compile() {
	echo "patch is done"
	make -C ${S} ARCH="arm64" V=1
}

do_configure() {
	echo "const char iw_version[] = \"${PKG_VERSION}\";" > ${WORKDIR}/${PKG_NAME}-${PKG_VERSION}/version.c
	rm -f ${WORKDIR}/${PKG_NAME}-${PKG_VERSION}/version.sh
	touch ${WORKDIR}/${PKG_NAME}-${PKG_VERSION}/version.sh
	chmod +x ${WORKDIR}/${PKG_NAME}-${PKG_VERSION}/version.sh

}
FILES_${PN} += "/usr/sbin/*"

do_install() {
	echo "after compile"
	install -m 0755 -d ${D}/usr/sbin

	cp ${S}/iw ${D}/usr/sbin/
}
