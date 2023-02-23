version=2.6.2
src_url=https://github.com/klange/bim
src_type=git
src_commit=575811f5e0a441643d6f3bf677020db4ae849ca4
patches=0001-Port-to-Dennix.patch
buildsystem=make
install_targets=install
license_files=LICENSE

post_install() {
    cat > "$SYSROOT/home/user/.bimrc" << EOF
rundir /share/bim/themes
theme ansi
theme sunsmoke
smartcomplete 1
EOF
}
