#The image to be loaded/downloaded
FROM --platform=amd64 ubuntu:22.04

#Updating package lists
RUN apt-get update -y
RUN apt-get update --fix-missing -y

#Installing libraries
RUN apt-get install -y gdb gdbserver vim emacs libssl-dev lynx build-essential g++ libstdc++6 cmake clang lldb git
