# Use an Ubuntu base image
FROM ubuntu:latest

# Install necessary dependencies
RUN apt-get update

# Copy the source code to the container
COPY .. /app

# Set the working directory
WORKDIR /app
ENV DISPLAY=:0

RUN ./AppRun
