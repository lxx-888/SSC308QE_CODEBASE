CONFIG=config # Add new build config in ./config/
SOURCE=source # Put source files of luajit to ./source/

if [ ! -d "${SOURCE}" ]; then
    echo "${SOURCE} is not exist"
    exit 1
fi

if [ ! -d "${CONFIG}" ]; then
    echo "${CONFIG} is not exist"
    exit 1
fi

if [ ! -d "${SOURCE}/luajit" ]; then
    echo "${SOURCE} is not exist"
    exit 1
fi

build() {
    release_mode=${1}
    arch=${ARCH}
    host_cc=${HOST_CC}
    gccflags=""
    if [ -z "${host_cc}" ]; then
        host_cc=gcc
    fi
    if [ -z "${arch}" ]; then
        host_cc=unknown
    fi
    if [[ "${release_mode}" == "debug" ]]; then
        if [[ "${TOOLCHAIN}" == "uclibc" ]]; then
            gccflags="-Wall -g -pipe -fPIC"
        else
            gccflags="-fsanitize=address -fno-omit-frame-pointer -fsanitize-recover=address -Wall -g -pipe -fPIC"
        fi
    fi

    build_root="${PWD}/.build"

    build_out_path="${build_root}/${arch}/${TOOLCHAIN}/${TOOLCHAIN_VERSION}/${release_mode}/"
    static_lib_path="${PWD}/${arch}/lib/${TOOLCHAIN}/${TOOLCHAIN_VERSION}/${release_mode}/static/"
    dynamic_lib_path="${PWD}/${arch}/lib/${TOOLCHAIN}/${TOOLCHAIN_VERSION}/${release_mode}/dynamic/"
    bin_path="${PWD}/${arch}/bin/${TOOLCHAIN}/${TOOLCHAIN_VERSION}/${release_mode}/"
    include_path="${PWD}/include"

    echo ">>>>>>> Start build ${build_out_path} <<<<<<<"

    mkdir -p "${build_out_path}"

    make -C ${SOURCE}/luajit clean

    make -C ${SOURCE}/luajit \
        HOST_CC="${host_cc}" \
        CROSS="${CROSS}" \
        TARGET_CFLAGS="${TARGET_CFLAGS}" \
        TARGET_CFLAGS="${gccflags}" \
        TARGET_LDFLAGS="${gccflags}" \
        amalg -j8 || exit 1

    make -C ${SOURCE}/luajit install PREFIX="${build_out_path}" || exit 1

    mkdir -p "${static_lib_path}"
    mkdir -p "${dynamic_lib_path}"
    mkdir -p "${bin_path}"
    mkdir -p "${include_path}"

    cp "${build_out_path}/lib/libluajit-5.1.a" "${static_lib_path}/libluajit.a"
    cp "${build_out_path}/lib/libluajit-5.1.so" "${dynamic_lib_path}/libluajit.so"
    cp "${build_out_path}/bin/luajit" "${bin_path}"
    cp "${build_out_path}/include/luajit-2.1/" "${include_path}" -rT

    echo ">>>>>>> End build ${build_out_path} <<<<<<<"
}

for cfg in "${CONFIG}"/*.sh; do
    source "${cfg}"
    build debug
    build release
done

