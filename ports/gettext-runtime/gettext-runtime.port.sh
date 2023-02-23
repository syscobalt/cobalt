depends=libiconv
version=0.21
src_url=https://ftp.gnu.org/gnu/gettext/gettext-$version.tar.xz
src_type=tar.xz
src_sha256=d20fcbb537e02dcf1383197ba05bd0734ef7bf5db06bdb241eb69b7d16b73192
patches=0001-config.sub.patch
buildsystem=configure
purge=lib/libintl.la
license_files="../COPYING intl/COPYING.LIB"

prepare() {
    srcdir=$srcdir/gettext-runtime
}
