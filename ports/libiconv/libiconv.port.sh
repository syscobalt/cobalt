version=1.17
src_url=https://ftp.gnu.org/gnu/libiconv/libiconv-$version.tar.gz
src_type=tar.gz
src_sha256=8f74213b56238c85a50a5329f77e06198771e70dd9a739779f4c02f65d971313
patches=0001-config.sub.patch
buildsystem=configure
extra_configure_flags="--enable-cross-guesses=risky"
purge="lib/libcharset.la lib/libiconv.la"
license_files="COPYING COPYING.LIB"
