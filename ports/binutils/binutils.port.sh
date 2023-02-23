depends="gettext-runtime libz"
version=2.34
src_url=https://github.com/syscobalt/cobalt-binutils
src_type=git
# We always build the latest commit.
buildsystem=configure
extra_configure_flags="--with-system-zlib --disable-werror"
license_files="COPYING COPYING.LIB COPYING3 COPYING3.LIB"
purge="lib/libbfd.la lib/libopcodes.la"
