version=1.5.2
src_url=https://github.com/klange/nyancat
src_type=git
src_commit=d594203cc2c7a69c13ffe760a9a2b3dbaf33a9a6
patches=0001-Port-to-Dennix.patch
buildsystem=make

install_license() {
    mkdir -p "$SYSROOT/share/licenses/nyancat"
    head -n49 "$srcdir/src/nyancat.c" | tail -n48 | sed -E 's/^ \* ?//' > "$SYSROOT/share/licenses/nyancat/LICENSE"
}
