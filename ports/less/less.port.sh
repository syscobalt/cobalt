depends=ncurses
version=590
src_url=https://www.greenwoodsoftware.com/less/less-$version.tar.gz
src_type=tar.gz
src_sha256=6aadf54be8bf57d0e2999a3c5d67b1de63808bb90deb8f77b028eafae3a08e10
patches=0001-less.patch
buildsystem=configure
license_files="COPYING LICENSE"

prepare_source() {
    # Write permissions are not set in the archive.
    chmod -R +w "$srcdir"
}

post_install() {
    ln "$SYSROOT/bin/less" "$SYSROOT/bin/more"
}
