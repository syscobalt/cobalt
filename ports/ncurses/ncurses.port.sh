version=6.2
src_url=https://ftp.gnu.org/gnu/ncurses/ncurses-$version.tar.gz
src_type=tar.gz
src_sha256=30306e0c76e0f9f1f0de987cf1c82a5c21e1ce6568b9227f7da5b71cbea86c9d
patches="0001-ncurses.patch 0002-terminfo.patch"
buildsystem=configure
extra_configure_flags="--enable-overwrite --enable-pc-files --enable-widec
--with-pkg-config-libdir=/lib/pkgconfig --without-cxx-binding --without-debug
--without-tests cf_cv_working_poll=yes"
license_files=COPYING
purge=bin/ncursesw6-config

post_install() {
    ln -fs libncursesw.a "$SYSROOT/lib/libcurses.a"
}
