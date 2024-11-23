# Use an Ubuntu base image
FROM ubuntu:20.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV CFLAGS="-fPIC"
ENV CPPFLAGS=$CFLAGS
ENV CXXFLAGS=$CFLAGS
ENV MAKE_DEFAULT=/torcs-code/Make-default.mk
ENV TORCS_BASE=/torcs-code/

# Update and install dependencies
RUN apt-get update && apt-get install -y \
    libglib2.0-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3-dev \
    libplib-dev \
    libopenal-dev \
    libalut-dev \
    libxi-dev \
    libxmu-dev \
    libxrender-dev \
    libxrandr-dev \
    libpng-dev \
    libxxf86vm-dev \
    libvorbis-dev \
    make \
    g++ \
    gdb \
    python3 \
    python3-pip \
    && pip3 install matplotlib numpy scipy \
    && apt-get clean

# Copy Sirianni tools into container
COPY sirianni_tools /usr/local/lib/sirianni_tools

# Add Sirianni tools to both PYTHONPATH (for imports) and PATH (for execution)
ENV PYTHONPATH="${PYTHONPATH}:/usr/local/lib/sirianni_tools"
ENV PATH="${PATH}:/usr/local/lib/sirianni_tools"

# Clone TORCS repository
COPY torcs-1.3.7-master /torcs-code

# Set working directory
WORKDIR /torcs-code

# Configure and build TORCS
RUN ./configure --disable-xrandr --x-libraries=/usr/lib/ \
    && make \
    && make install \
    && make datainstall

# Set the working directory to the default directory
WORKDIR /root

# Entry point
ENTRYPOINT ["/bin/bash"]