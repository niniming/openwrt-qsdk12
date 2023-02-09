inherit autotools-brokensep module qperf


# if is TARGET_KERNEL_ARCH is set inherit qtikernel-arch to compile for that arch.
inherit ${@bb.utils.contains('TARGET_KERNEL_ARCH', 'aarch64', 'qtikernel-arch', '', d)}

DESCRIPTION = "Recipe file for Hy-Fi bridging Netfilter Module"
LICENSE          = "ISC & GPLv2"
LIC_FILES_CHKSUM = "file://${TOPDIR}/../poky/meta-qti-ipq/recipes/recipes-hyfi-oss/qca-hyfi-bridge/copyright;md5=4728e7f54c2a01772aaae890803c91d1"

FILESPATH =+ "${TOPDIR}/../src:"

DEPENDS = "glib-2.0 qca-ssdk-nohnat virtual/kernel emesh-sp"

SRC_URI = "file://ipq/qca-ezmesh/qca-hyfi-bridge \
	file://ipq/qca-ezmesh/qca-ezmesh-files/hyfi/qca-hyfi-bridging/qca-hyfi-bridge \
	"
S = "${WORKDIR}/ipq/qca-ezmesh/qca-hyfi-bridge"

FILES_${PN} += "${libdir}"
FILES_${PN} += "/etc*"
FILES_${PN} += "/install/*"
FILES_${PN} += "/usr/lib/*"
FILES_${PN} += "/etc/init.d/hyfi-bridging"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

setup_build_variables() {
	export STAGING_DIR="${STAGING_DIR}"; \
	export STAGING_INCDIR="${STAGING_INCDIR}"; \
	export STAGING_LIBDIR="${STAGING_LIBDIR}"; \
	export STAGING_KERNEL_BUILDDIR="${STAGING_KERNEL_BUILDDIR}"; \
	export LINUX_SRC_DIR="${STAGING_KERNEL_BUILDDIR}/source"; \
	export LINUX_VERSION="${KERNEL_VERSION}"; \
	export LINUX_DIR="${STAGING_KERNEL_BUILDDIR}/source"; \
	export LINUX_SRC_DIR="${STAGING_KERNEL_BUILDDIR}/source"; \
	export CROSS_COMPILE="${KERNEL_CROSS}"; \
	export PKG_BUILD_DIR="${S}"; \
	export TARGET_CROSS="${TARGET_PREFIX}"; \
	export TARGET_CFLAGS="${TOOLCHAIN_OPTIONS} -I${STAGING_INCDIR}/libnl3 \
				-I${STAGING_KERNEL_BUILDDIR} -I${LINUX_SRC_DIR} -I${STAGING_INCDIR} \
				-fpie -I${STAGING_DIR}/usr/include/ -I${STAGING_DIR}/usr/include/libxml2/ \
				-T${STAGING_KERNEL_DIR} -Wno-misleading-indentation"; \
	export TARGET_LDFLAGS="-L${STAGING_LIBDIR} -lc -lpthread -ldl"; \
	export LDFLAGS="-L${STAGING_LIBDIR} -lc -lpthread -ldl"; \
	export ARCH="arm"; \
	export HYBRID_MC_MLD=1; \
	export KERNELRELEASE="${LINUX_RELEASE}"; \
	export LINUX_VERSION="${KERNEL_VERSION}"; \
	export CFLAGS="-I${STAGING_KERNEL_BUILDDIR} -I${STAGING_KERNEL_DIR} -I${STAGING_DIR}/usr/include/ \
				-I${STAGING_INCDIR}/glib-2.0 -I${STAGING_INCDIR} -I${STAGING_LIBDIR}/glib-2.0/include / \
				-I${STAGING_KERNEL_DIR}"; \
	export MDIR="${S}"; \
	export PLC_SUPPORT_NF=0; \
	export DISABLE_SSDK_SUPPORT=1; \
	export HYFI_BRIDGE_EMESH_ENABLE=1; \
	export CONFIG_BUILD_YOCTO="y"; \
	export MODULEPATH="${LINUX_SRC_DIR}"; \
}

do_compile() {
	setup_build_variables
	make -C ${LINUX_SRC_DIR} HYFI_CFLAGS="-I${TOPDIR}/../src/ipq/qca-ezmesh/emesh-sp/" \
		M=${S} ARCH="arm" CROSS_COMPILE=${TARGET_PREFIX} \
		KERNELRELEASE=1 KERNELPATH=${STAGING_KERNEL_DIR} \
		KBUILDPATH=${STAGING_KERNEL_BUILDDIR} \
		KBUILD_EXTRA_SYMBOLS="${STAGING_DIR}/usr/include/emesh-sp/Module.symvers" MDIR=${S}
}

do_install() {
	install -d ${D}/etc/init.d
	install -d ${D}/usr/include/hyfibr
	install -m 0644 ${S}/hyfi-multicast/mc_api.h ${D}/usr/include/hyfibr
	install -m 0644 ${S}/hyfi-netfilter/hyfi_ecm.h ${D}/usr/include/hyfibr
	install -m 0644 ${S}/hyfi-netfilter/hyfi_api.h ${D}/usr/include/hyfibr
	install -m 0644 ${S}/hyfi-netfilter/hyfi_hash.h ${D}/usr/include/hyfibr
	install -m 0755 ${WORKDIR}/ipq/qca-ezmesh/qca-ezmesh-files/hyfi/qca-hyfi-bridging/qca-hyfi-bridge/files/hyfi-bridging.init \
		${D}/etc/init.d/hyfi-bridging
	install -d ${D}${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra
	install -m 0755 ${S}/*.ko ${D}/${nonarch_base_libdir}/modules/${KERNEL_VERSION}/extra/
	install -d ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${PN}
	install -m 0755 ${S}/*.ko ${D}${base_libdir}/modules/${KERNEL_VERSION}/kernel/drivers/${PN}
#To start hyfi-briging and insmoding hyfi-bridging.ko on boot-up
	install -d ${D}/etc/initscripts
	install -m 0755 ${THISDIR}/files/hyfi-bridging.initscripts ${D}/etc/initscripts/start_hyfi-bridging
}
