inherit autotools-brokensep module qperf

# if is TARGET_KERNEL_ARCH is set inherit qtikernel-arch to compile for that arch.
inherit ${@bb.utils.contains('TARGET_KERNEL_ARCH', 'aarch64', 'qtikernel-arch', '', d)}

DESCRIPTION = "Recipe file for the kernel module for Service Prioritization in EasyMesh"
LICENSE = "ISC"
LIC_FILES_CHKSUM = "file://${TOPDIR}/../poky/meta/files/common-licenses/ISC;md5=f3b90e78ea0cffb20bf5cca7947a896d"

S = "${WORKDIR}/ipq/qca-ezmesh/emesh-sp"

FILESPATH =+ "${TOPDIR}/../src:"

DEPENDS = "glib-2.0 virtual/kernel"

SRC_URI = "file://ipq/qca-ezmesh/emesh-sp"

FILES_SOLIBSDEV = ""
#FILES_${PN} += "${S}/emesh-sp.ko"
FILES_${PN} += "/etc*"

setup_build_variables() {
	export TARGET_CROSS="${TARGET_PREFIX}"; \
	export TOOLPREFIX="${TARGET_CROSS}"; \
	export EXTRA_CFLAGS="${TARGET_CFLAGS}"; \
	export EXTRA_LDFLAGS="${TARGET_LDFLAGS}"; \
	export STAGING_INCDIR="${STAGING_INCDIR}"; \
	export STAGING_LIBDIR="${STAGING_LIBDIR}"; \
	export STAGING_KERNEL_BUILDDIR="${STAGING_KERNEL_BUILDDIR}"; \
	export LINUX_SRC_DIR="${STAGING_KERNEL_BUILDDIR}/source"; \
	export LINUX_DIR="${STAGING_KERNEL_BUILDDIR}/source"; \
	export LINUX_KARCH="arm"; \
	export CONFIG_BUILD_YOCTO="y"; \
	export CONFIG_NETFILTER="y"; \
	export CONFIG_BRIDGE_NETFILTER="y"; \
}

do_compile() {
	setup_build_variables
	make -C "${LINUX_DIR}" CROSS_COMPILE="${TARGET_CROSS}" ARCH="${LINUX_KARCH}" M="${S}" modules
}

do_install() {
	install -d ${D}/usr/include/emesh-sp
	install -m 0644 ${S}/sp_api.h ${D}/usr/include/emesh-sp
	mkdir -p ${STAGING_DIR}/usr/include/emesh-sp
	cp ${TOPDIR}/../src/ipq/qca-ezmesh/emesh-sp/*.h  ${STAGING_DIR}/usr/include/
	install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
	install -m 0755 ${S}/*.ko ${D}/${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/
	install -m 0755 ${S}/Module.symvers ${STAGING_DIR}/usr/include/emesh-sp/
	install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${PN}
	install -m 0755 ${S}/*.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${PN}
	#To load emesh-sp.ko on boot-up
	install -d ${D}/etc/systemd/system/
	install -m 0644 ${THISDIR}/files/emesh-sp.service -D ${D}/etc/systemd/system/emesh-sp.service
	install -d ${D}/etc/systemd/system/multi-user.target.wants/
	ln -sf /etc/systemd/system/emesh-sp.service \
		${D}/etc/systemd/system/multi-user.target.wants/emesh-sp.service
	install -d ${D}/etc/initscripts
	install -m 0755 ${THISDIR}/files/emesh-sp.initscripts ${D}/etc/initscripts/start_emesh-sp
}
