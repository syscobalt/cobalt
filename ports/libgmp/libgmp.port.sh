version=6.1.2
src_url="https://gmplib.org/download/gmp/gmp-$version.tar.xz"
src_type=tar.xz
src_sha256=87b565e89a9a684fe4ebeeddb8399dce2599f9c9049854ca8c0dfbdea0e21912
patches=0001-configfsf.sub.patch
buildsystem=configure
license_files="COPYING.LESSERv3 COPYINGv2 COPYINGv3"
purge=lib/libgmp.la
