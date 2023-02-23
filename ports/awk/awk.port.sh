depends="gettext-runtime libmpfr"
version=5.1.1
src_url=https://ftp.gnu.org/gnu/gawk/gawk-$version.tar.xz
src_type=tar.xz
src_sha256=d87629386e894bbea11a5e00515fc909dc9b7249529dad9e6a3a2c77085f7ea2
patches=0001-awk.patch
buildsystem=configure
extra_configure_flags="ac_cv_func_connect=no"
license_files=COPYING
