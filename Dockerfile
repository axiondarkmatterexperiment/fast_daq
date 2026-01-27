ARG base_image=debian
ARG base_tag=13

# Base image with environment variables set
FROM ${base_image}:${base_tag} AS base

# Set bash as the default shell
SHELL ["/bin/bash", "-c"]

ARG build_type=DEBUG
ARG narg=2
ARG enable_ats=FALSE

ENV NARG=${narg}
ENV ENABLE_ATS=${enable_ats}

ENV INSTALL_PREFIX=/usr/local
ENV LD_LIBRARY_PATH=${INSTALL_PREFIX}
ENV PATH="${PATH}:${INSTALL_PREFIX}"


# Build image with dev dependencies
FROM base AS deps

RUN apt-get update &&\
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential \
        cmake \
        git \
        openssl \
        curl \
        libfftw3-dev \
        libboost-atomic-dev \
        libboost-chrono-dev \
        libboost-filesystem-dev \
        libboost-system-dev \
        libhdf5-dev \
        librabbitmq-dev \
        libyaml-cpp-dev \
        rapidjson-dev \
#        python3 \
#        python3-pip \
        &&\
    apt-get clean &&\
    rm -rf /var/lib/apt/lists/* &&\
    curl -O https://raw.githubusercontent.com/rabbitmq/rabbitmq-management/v3.7.8/bin/rabbitmqadmin && \
    chmod +x rabbitmqadmin && \
    mv rabbitmqadmin /usr/local/bin/ && \
    /bin/true

# ATS installation, if present
COPY ./ATS_local/usr /usr

# Build fast_daq in the deps image
FROM deps AS build

COPY . /tmp_source

## store cmake args because we'll need to run twice (known package_builder issue)
## use `extra_cmake_args` to add or replace options at build time; CMAKE_CONFIG_ARGS_LIST are defaults
ARG extra_cmake_args=""
ENV CMAKE_CONFIG_ARGS_LIST="\
      -D CMAKE_BUILD_TYPE=$build_type \
      -D CMAKE_INSTALL_PREFIX:PATH=$INSTALL_PREFIX \
      -D FastDAQ_ENABLE_ATS=$ENABLE_ATS \
      ${extra_cmake_args} \
      "

RUN mkdir -p /build &&\
    cd /build &&\
    cmake ${CMAKE_CONFIG_ARGS_LIST} /tmp_source &&\
    make -j${NARG} install &&\
    /bin/true

# Final production image
FROM base

RUN apt-get update &&\
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
        build-essential \
        libssl3 \
        libfftw3-single3 \
        libboost-atomic1.83.0 \
        libboost-chrono1.83.0t64 \
        libboost-filesystem1.83.0 \
        libboost-system1.83.0 \
        libhdf5-cpp-310 \
        librabbitmq4 \
        libyaml-cpp0.8 \
        rapidjson-dev \
        &&\
    apt-get clean &&\
    rm -rf /var/lib/apt/lists/* &&\
    /bin/true

COPY ./entrypoint.sh /root/entrypoint.sh

COPY --from=build /usr/local /usr/local

WORKDIR /root
