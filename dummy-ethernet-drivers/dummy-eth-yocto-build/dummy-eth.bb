SUMMARY = "Dummy Ethernet Driver"
LICENSE = "GPL-2.0-only"

LIC_FILES_CHKSUM = "file://dummy-eth-src.c;beginline=1;endline=10;md5=1bcb91b045cb578ea4f55439672ef238"

SRC_URI = "file://dummy-eth-src.c \
           file://Makefile"

S = "${WORKDIR}"

inherit module