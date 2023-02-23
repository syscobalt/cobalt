depends="liblzma libz"
version=3.4.2
src_url=https://www.libarchive.org/downloads/libarchive-$version.tar.xz
src_type=tar.xz
src_sha256=d8e10494b4d3a15ae9d67a130d3ab869200cfd60b2ab533b391b0a0d5500ada1
patches=0001-libarchive.patch
buildsystem=configure
extra_configure_flags=--disable-maintainer-mode
license_files=COPYING
purge=lib/libarchive.la

post_install() {
    ln -f "$SYSROOT/bin/bsdcpio" "$SYSROOT/bin/cpio"
    ln -f "$SYSROOT/bin/bsdtar" "$SYSROOT/bin/tar"
}
