# This is not a real port and just exists so that the grub source code is
# distributed in port dumps because the release images contain grub.
version=2.06
src_url=https://ftp.gnu.org/gnu/grub/grub-$version.tar.xz
src_type=tar.xz
src_sha256=b79ea44af91b93d17cd3fe80bdae6ed43770678a9a5ae192ccea803ebb657ee1

prepare_source() {
    # On UEFI systems GRUB needs a font.
    unifont_version=13.0.06
    unifont_sha256=b7668a5d498972dc4981250c49f83601babce797be19b4fdd0f2f1c6cfbd0fc5
    wget "https://ftp.gnu.org/gnu/unifont/unifont-$unifont_version/unifont-$unifont_version.bdf.gz" -O "$srcdir/unifont.bdf.gz"
    printf "%s\t%s\n" "$unifont_sha256" "$srcdir/unifont.bdf.gz" | sha256sum -c || exit 1
    gunzip "$srcdir/unifont.bdf.gz"
}

build() {
    echo "Cannot build grub: This is not a real port." >&2
    return 1
}

# To build a GRUB that can be booted on both BIOS and UEFI systems GRUB needs to
# be built twice.
# First configure with --target=i386 --with-platform=pc
# Build and install GRUB, then clean the tree.
# Secondly configure with --target=x86_64 --with-platform=efi
# Build and install again.
# Note that GRUB should be configured within the source tree. Otherwise it will
# not find unifont.bdf.
