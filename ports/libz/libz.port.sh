version=1.2.8.f
src_url=https://gitlab.com/sortix/libz.git
src_type=git
src_commit=752c1630421502d6c837506d810f7918ac8cdd27
buildsystem=configure

install_license() {
    mkdir -p $SYSROOT/share/licenses/libz
    head -n28 "$srcdir/zlib.h" | tail -n25 | sed 's/^  //g' > $SYSROOT/share/licenses/libz/LICENSE
}
