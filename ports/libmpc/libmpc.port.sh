depends=libmpfr
version=1.1.0
src_url="https://ftp.gnu.org/gnu/mpc/mpc-$version.tar.gz"
src_type=tar.gz
src_sha256=6985c538143c1208dcb1ac42cedad6ff52e267b47e5f970183a3e75125b43c2e
patches=0001-config.sub.patch
buildsystem=configure
license_files="COPYING.LESSER"
purge=lib/libmpc.la

prepare_source() {
    # For some reason write permissions are not set in the archive.
    chmod -R +w "$srcdir"
}
