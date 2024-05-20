# Use an Ubuntu base image
FROM ubuntu:20.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update 
RUN apt-get upgrade
RUN apt-get install torcs -y
# for the virtual display required by trackgen
RUN apt-get install xvfb  -y
RUN apt-get clean
# Set the working directory to the default directory
WORKDIR /root

ENTRYPOINT ["/bin/bash"]
