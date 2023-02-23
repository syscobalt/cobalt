depends="binutils gettext-runtime libmpc libstdc++"
version=9.3.0
src_url=https://github.com/syscobalt/cobalt-gcc
src_type=git
# We always build the latest commit.
buildsystem=configure
extra_configure_flags="--target=$host --with-sysroot=/
--with-build-sysroot=$SYSROOT --with-system-zlib --enable-languages=c,c++"
make_targets="all-gcc all-target-libgcc"
install_targets="install-strip-gcc install-strip-target-libgcc"
license_files="COPYING COPYING.LIB COPYING.RUNTIME COPYING3 COPYING3.LIB"
purge="libexec/gcc/$host/$version/liblto_plugin.la"

install_port() {
    make $install_targets DESTDIR="$SYSROOT"
    ln -fs gcc "$SYSROOT/bin/cc"
}
