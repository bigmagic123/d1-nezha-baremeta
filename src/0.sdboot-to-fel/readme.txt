# configure
set cross info

# build
meson setup _build --cross-file cross.txt
cd _build
ninja