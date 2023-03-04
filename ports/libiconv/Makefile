VERSION = 1.17
SOURCE_URL = https://ftp.gnu.org/gnu/libiconv/libiconv-$(VERSION).tar.gz

default:
	wget $(SOURCE_URL)
	tar -xz -f libiconv-$(VERSION).tar.gz
	cp 0001-config.sub.patch libiconv-$(VERSION)
	cd libiconv-$(VERSION)
	patch -p1 < 0001-config.sub.patch
	libiconv-$(VERSION)/configure --enable-cross-guesses=risky