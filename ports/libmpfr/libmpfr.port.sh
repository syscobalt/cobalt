depends=libgmp
version=4.0.2
src_url="https://www.mpfr.org/mpfr-$version/mpfr-$version.tar.xz"
src_type=tar.xz
src_sha256=1d3be708604eae0e42d578ba93b390c2a145f17743a744d8f3f8c2ad5855a38a
patches=0001-config.sub.patch
buildsystem=configure
purge=lib/libmpfr.la

prepare() {
    # The configure script does not properly realize we are cross compiling
    # unless --build is given explicitly.
    extra_configure_flags="--build=$("$srcdir/config.guess")"
}

install_license() {
    # libmpfr already installs its license into /share/doc/mpfr.
    mkdir -p "$SYSROOT/share/licenses/libmpfr"
    ln -f "$SYSROOT/share/doc/mpfr/COPYING" "$SYSROOT/share/licenses/libmpfr"
    ln -f "$SYSROOT/share/doc/mpfr/COPYING.LESSER" "$SYSROOT/share/licenses/libmpfr"
}
