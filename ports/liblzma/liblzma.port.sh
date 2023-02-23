version=5.2.5
src_url=https://tukaani.org/xz/xz-$version.tar.xz
src_type=tar.xz
src_sha256=3e1e518ffc912f86608a8cb35e4bd41ad1aec210df2a47aaa1f95e7f5576ef56
patches=0001-liblzma.patch
buildsystem=configure
# We only want the library.
extra_configure_flags="--disable-xz --disable-xzdec
--disable-lzmadec --disable-lzmainfo --disable-scripts --disable-nls"
purge=lib/liblzma.la

install_license() {
    # liblzma already installs its license into /share/doc/xz.
    mkdir -p "$SYSROOT/share/licenses/liblzma"
    ln -f "$SYSROOT/share/doc/xz/COPYING" "$SYSROOT/share/licenses/liblzma"
}
