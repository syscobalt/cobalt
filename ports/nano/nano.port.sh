depends="gettext-runtime libz ncurses"
version=5.7
src_url=https://nano-editor.org/dist/v5/nano-$version.tar.xz
src_type=tar.xz
src_sha256=d4b181cc2ec11def3711b4649e34f2be7a668e70ab506860514031d069cccafa
patches=0001-nano.patch
buildsystem=configure
extra_configure_flags="--enable-cross-guesses=risky --with-included-regex"
license_files="COPYING COPYING.DOC"

post_install() {
    mkdir -p "$SYSROOT/etc"
    sed '/historylog/s/^# //;/\*.nanorc/s/^# //' "$builddir/doc/sample.nanorc" > "$SYSROOT/etc/nanorc"
}
