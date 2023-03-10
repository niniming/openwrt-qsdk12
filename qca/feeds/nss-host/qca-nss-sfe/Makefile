include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=qca-nss-sfe
PKG_SOURCE_PROTO:=git
PKG_BRANCH:=master
PKG_RELEASE:=1

include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_VERSION:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
endif

include $(INCLUDE_DIR)/package.mk

define KernelPackage/qca-nss-sfe
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=@TARGET_ipq95xx||TARGET_ipq50xx||TARGET_ipq53xx +kmod-pppoe \
	   +TARGET_ipq95xx:kmod-qca-nss-ppe-rule \
	   +TARGET_ipq53xx:kmod-qca-nss-ppe-rule
  TITLE:=Kernel data path for SFE
  FILES:=$(PKG_BUILD_DIR)/qca-nss-sfe.ko
  KCONFIG:=CONFIG_NF_CONNTRACK_EVENTS=y
  AUTOLOAD:=$(call AutoLoad,09,qca-nss-sfe)
endef

define KernelPackage/qca-nss-sfe/Description
Shortcut is an in-Linux-kernel IP packet forwarding engine.
endef

define KernelPackage/qca-nss-sfe/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/sfe_dump $(1)/usr/bin
endef

ifneq (, $(filter $(CONFIG_IPV6),m y))
SFE_MAKE_OPTS:=SFE_SUPPORT_IPV6=y
endif

SFE_MAKE_OPTS+=SFE_PROCESS_LOCAL_OUT=y

EXTRA_CFLAGS+= -I$(PKG_BUILD_DIR)/exports

ifeq ($(CONFIG_KERNEL_IPQ_MEM_PROFILE),256)
EXTRA_CFLAGS+= -DSFE_MEM_PROFILE_LOW
else ifeq ($(CONFIG_LOWMEM_FLASH),y)
EXTRA_CFLAGS+= -DSFE_MEM_PROFILE_LOW
else ifeq ($(CONFIG_KERNEL_IPQ_MEM_PROFILE),512)
EXTRA_CFLAGS+= -DSFE_MEM_PROFILE_MEDIUM
ifneq ($(CONFIG_TARGET_ipq95xx)$(CONFIG_TARGET_ipq53xx),)
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/qca-nss-ppe
SFE_MAKE_OPTS+=SFE_RFS_SUPPORTED=y
endif
else
ifneq ($(CONFIG_TARGET_ipq95xx)$(CONFIG_TARGET_ipq53xx),)
EXTRA_CFLAGS+= -I$(STAGING_DIR)/usr/include/qca-nss-ppe
SFE_MAKE_OPTS+=SFE_RFS_SUPPORTED=y
endif
endif

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)"  $(strip $(SFE_MAKE_OPTS))\
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		M="$(PKG_BUILD_DIR)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		modules
endef

ifneq ($(CONFIG_PACKAGE_kmod-qca-nss-sfe),)
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/qca-nss-sfe
	$(CP) -rf $(PKG_BUILD_DIR)/exports/sfe_api.h $(1)/usr/include/qca-nss-sfe
endef
endif

$(eval $(call KernelPackage,qca-nss-sfe))
