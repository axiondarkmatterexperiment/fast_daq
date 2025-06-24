ARG base_image=debian
ARG base_tag=12

# Base image with environment variables set
#FROM ${base_image}:${base_tag} AS base
FROM ${base_image}:${base_tag}

# Set bash as the default shell
SHELL ["/bin/bash", "-c"]

ARG build_type=DEBUG
ARG narg=2


ENV ADMX_ROOT=/usr/local/
ENV FAST_DAQ_INSTALL_PREFIX=${ADMX_ROOT}
ENV NARG=${narg}

ENV PATH="${PATH}:${FAST_DAQ_INSTALL_PREFIX}"

# Build image with dev dependencies

# use quill_checkout to specify a tag or branch name to checkout
ARG quill_checkout=v8.1.1
ENV QUILL_CHECKOUT=${quill_checkout}

RUN apt-get update &&\
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential \
        cmake \
        git \
        openssl \
	curl \
        libfftw3-dev \
        libboost-chrono-dev \
        libboost-filesystem-dev \
        libboost-system-dev \
        libhdf5-dev \
        librabbitmq-dev \
        libyaml-cpp-dev \
        rapidjson-dev \
	python3 \
	python3-pip \
        &&\
    apt-get clean &&\
    rm -rf /var/lib/apt/lists/* &&\
    cd /usr/local &&\
    git clone https://github.com/odygrd/quill.git &&\
    cd quill &&\
    git checkout ${QUILL_CHECKOUT} &&\
    mkdir build &&\
    cd build &&\
    cmake .. &&\
    make -j${narg} install &&\
    cd / &&\
    rm -rf /usr/local/quill &&\
    /bin/true

# Build fast_daq in the deps image
RUN curl -O https://raw.githubusercontent.com/rabbitmq/rabbitmq-management/v3.7.8/bin/rabbitmqadmin && \
   chmod +x rabbitmqadmin && mv rabbitmqadmin /usr/local/bin/
RUN mkdir -p /usr/include && mkdir -p /usr/lib && mkdir -p /tmp_source
COPY . /tmp_source
COPY ./ATS_local/usr /usr

## store cmake args because we'll need to run twice (known package_builder issue)
## use `extra_cmake_args` to add or replace options at build time; CMAKE_CONFIG_ARGS_LIST are defaults
ARG extra_cmake_args=""
ENV CMAKE_CONFIG_ARGS_LIST="\
      -D CMAKE_BUILD_TYPE=$build_type \
      -D CMAKE_INSTALL_PREFIX:PATH=$FAST_DAQ_INSTALL_PREFIX \
      -D FastDAQ_ENABLE_ATS=TRUE \
      ${extra_cmake_args} \
      "

RUN mkdir -p /build &&\
    cd /build &&\
    cmake ${CMAKE_CONFIG_ARGS_LIST} /tmp_source &&\
    make -j${NARG} install &&\
    /bin/true



COPY ./entrypoint.sh /root/entrypoint.sh
RUN rm -rf /tmp_source
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$FAST_DAQ_INSTALL_PREFIX
WORKDIR /root
