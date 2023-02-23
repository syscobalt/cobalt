version=9.3.0
src_url=https://github.com/syscobalt/cobalt-gcc
src_type=git
# We always build the latest commit.
buildsystem=configure
extra_configure_flags="--disable-nls"
license_files="COPYING.RUNTIME COPYING3"
purge="lib/libstdc++.la lib/libsupc++.la"

prepare_source() {
    # The following commands separate libstdc++ from the gcc source tree.
    cp  "$srcdir/install-sh" "$srcdir/config.sub" "$srcdir/config.guess" \
        "$srcdir/missing" "$srcdir/config-ml.in" "$srcdir/ltmain.sh" \
        "$srcdir/mkinstalldirs" "$srcdir/libtool-ldflags" \
        "$srcdir/COPYING.RUNTIME" "$srcdir/COPYING3" "$srcdir/libstdc++-v3"

    # Fix references to the top directory.
    sed -i '/multi_basedir=/ s/\.\."$/"/
/toplevel_builddir=/ s|/\.\.||
/toplevel_srcdir=/ s|/\.\.||' "$srcdir/libstdc++-v3/configure"
    sed -i 's|$(top_srcdir)/../|$(top_srcdir)/|g' \
        $(find "$srcdir/libstdc++-v3" -name Makefile.in)

    # Copy files from other gcc components used by libstdc++.
    mkdir -p "$srcdir/libstdc++-v3/gcc"
    cp "$srcdir/gcc/BASE-VER" "$srcdir/gcc/DATESTAMP" "$srcdir/libstdc++-v3/gcc"

    mkdir -p "$srcdir/libstdc++-v3/libiberty"
    cp "$srcdir/libiberty/cp-demangle.c" "$srcdir/libiberty/cp-demangle.h" \
        "$srcdir/libstdc++-v3/libiberty"

    mkdir -p "$srcdir/libstdc++-v3/libgcc"
    cp "$srcdir/libgcc/"*.h "$srcdir/libstdc++-v3/libgcc"
    ln -s "gthr-posix.h" "$srcdir/libstdc++-v3/libgcc/gthr-default.h"
    cp "$srcdir/include/ansidecl.h" "$srcdir/include/libiberty.h" \
        "$srcdir/include/demangle.h" "$srcdir/libstdc++-v3/include"

    srcdir="$srcdir/libstdc++-v3"
}

prepare() {
    # We need to override CXX because otherwise some configure tests fail when
    # trying to link with the not yet built libstdc++.
    export CXX="$host-gcc -nostdinc++" CXXFLAGS="-O2 -g"
}
