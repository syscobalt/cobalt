depends=gettext-runtime
version=3.7
src_url=https://ftp.gnu.org/gnu/grep/grep-$version.tar.xz
src_type=tar.xz
src_sha256=5c10da312460aec721984d5d83246d24520ec438dd48d7ab5a05dbc0d6d6823c
patches=0001-grep.patch
buildsystem=configure
extra_configure_flags="--enable-cross-guesses=risky --with-included-regex
gl_cv_func_fnmatch_posix=yes"
license_files=COPYING
