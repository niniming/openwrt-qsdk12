# ###################################################
# # Makefile for the NSS EIP197 driver
# ###################################################

NSS_EIP_MOD_NAME=qca-nss-eip

ccflags-y += -Wall -Werror
ccflags-y += -DNSS_EIP_BUILD_ID=\"'Build_ID - $(shell date +'%m/%d/%y, %H:%M:%S') SoC=$(SoC)'\"

obj-m += $(NSS_EIP_MOD_NAME).o
$(NSS_EIP_MOD_NAME)-objs += eip.o
$(NSS_EIP_MOD_NAME)-objs += eip_ctx.o
$(NSS_EIP_MOD_NAME)-objs += eip_dma.o
$(NSS_EIP_MOD_NAME)-objs += eip_dma_hy.o
$(NSS_EIP_MOD_NAME)-objs += eip_hw.o
$(NSS_EIP_MOD_NAME)-objs += eip_tr.o
$(NSS_EIP_MOD_NAME)-objs += eip_tr_skcipher.o
$(NSS_EIP_MOD_NAME)-objs += eip_tr_ahash.o
$(NSS_EIP_MOD_NAME)-objs += eip_tr_aead.o
$(NSS_EIP_MOD_NAME)-objs += eip_tr_ipsec.o
$(NSS_EIP_MOD_NAME)-objs += eip_tk.o
$(NSS_EIP_MOD_NAME)-objs += eip_flow.o
obj ?= .
