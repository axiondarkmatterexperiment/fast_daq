ARG base_image_repo=debian
ARG base_image_tag=9

FROM ${base_image_repo}:${base_image_tag}

# Most dependencies

RUN apt-get update && \
    apt-get clean && \
    apt-get --fix-missing  -y install \
        build-essential \
        cmake \
        libfftw3-3 \
        libfftw3-dev \
        gdb \
        libboost-all-dev \
        #libhdf5-dev \
        librabbitmq-dev \
        wget && \
    rm -rf /var/lib/apt/lists/*

# note that the build dir is *not* in source, this is so that the source can me mounted onto the container without covering the build target

# build hdf5 from source because we need > 1.10.1
ARG hdf5_version=hdf5-1.10.5
RUN cd /tmp &&\
    wget https://support.hdfgroup.org/ftp/HDF5/releases/hdf5-1.10/${hdf5_version}/src/${hdf5_version}.tar.gz
RUN cd /tmp &&\
    tar -xvzf ${hdf5_version}.tar.gz &&\
    cd ${hdf5_version} &&\
    ./configure --prefix=$COMMON_BUILD_PREFIX --enable-cxx --enable-shared &&\
    make install &&\
    /bin/true

# actually build the local project(s)

COPY cmake /usr/local/src/cmake
COPY monarch /usr/local/src/monarch
COPY sandfly /usr/local/src/sandfly
COPY source /usr/local/src/source
COPY .gitignore /usr/local/src/.gitignore
COPY .gitmodules /usr/local/src/.gitmodules
COPY CMakeLists.txt /usr/local/src/CMakeLists.txt

ARG build_type=RELEASE
# need to build dripline separately
RUN mkdir -p /tmp/dl_build && \
    cd /tmp/dl_build && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
          -DCMAKE_BUILD_TYPE=${build_type} \
          /usr/local/src/fast_daq/sandfly/dripline-cpp && \
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
          -DCMAKE_BUILD_TYPE=${build_type} \
          /usr/local/src/fast_daq/sandfly/dripline-cpp && \
    make install && \
    /bin/true

ARG enable_ats=FALSE
RUN cd /usr/local/src && \
    mkdir -p build && \
    cd build && \
    cmake .. && \
    /bin/true
RUN cd /usr/local/src/build && \
    cmake \
          -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
          -DDripline_ENABLE_EXECUTABLES=FALSE \
          -DPsyllid_ENABLE_TESTING=FALSE \
          -DPsyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT=TRUE \
          -DFastDAQ_ENABLE_ATS:BOOL=${enable_ats} \
          -DCMAKE_BUILD_TYPE=${build_type} \
          .. && \
     /bin/true
RUN cd /usr/local/src/build && \
    make install && \
    /bin/true

# this is probalby not a good choice of default config
RUN cp /usr/local/src/fast_daq/examples/str_1ch_fpa.yaml /etc/fast_daq_config.yaml
