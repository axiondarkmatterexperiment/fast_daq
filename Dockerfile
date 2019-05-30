FROM debian:9

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
    make install -j3 &&\
    /bin/true

COPY psyllid /usr/local/src/psyllid
COPY source /usr/local/src/source
COPY cmake /usr/local/src/cmake
COPY CMakeLists.txt /usr/local/src/CMakeLists.txt

RUN cd /usr/local/src && \
    mkdir -p build && \
    cd build && \
    cmake .. && \
    /bin/true
RUN cd /usr/local/src/build && \
    cmake -DPsyllid_ENABLE_TESTING=FALSE \
          #-DMonarch_ENABLE_EXECUTABLES=FALSE \
          -DCMAKE_INSTALL_PREFIX:PATH=/usr/local \
          -DPsyllid_ENABLE_STREAMED_FREQUENCY_OUTPUT=TRUE \
          .. && \
     /bin/true
RUN cd /usr/local/src/build && \
    make install && \
    /bin/true

# this is probalby not a good choice of default config
RUN cp /usr/local/src/psyllid/examples/str_1ch_fpa.yaml /etc/psyllid_config.yaml
