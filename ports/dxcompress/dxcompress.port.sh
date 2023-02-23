depends="liblzma libz"
version=1.0
src_url=https://github.com/dennis95/dxcompress/releases/download/dxcompress-$version/dxcompress-$version.tar.xz
src_type=tar.xz
src_sha256=28ef275fff1ac2c126cd5bc3d7319038266274db8efe6bdc1f33ba6d5e235f4e
buildsystem=configure
extra_configure_flags="--enable-wrappers=gzip,xz"
license_files=LICENSE
