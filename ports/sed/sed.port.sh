depends=gettext-runtime
version=4.8
src_url=https://ftp.gnu.org/gnu/sed/sed-$version.tar.xz
src_type=tar.xz
src_sha256=f79b0cfea71b37a8eeec8490db6c5f7ae7719c35587f21edb0617f370eeff633
patches=0001-sed.patch
buildsystem=configure
extra_configure_flags="--enable-cross-guesses=risky --with-included-regex"
license_files=COPYING
