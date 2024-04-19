#The image to be loaded/downloaded
FROM ubuntu:22.04

#Updating package lists
RUN apt-get update -y

#Installing libraries
RUN apt-get install -y gdb vim emacs libssl-dev lynx build-essential g++ libstdc++6 cmake clang lldb git

#Add extra libraries to install here
# RUN apt-get install -y valgrind

#Setting the work directory in the container
WORKDIR /ECS_150/

#Copying your files here
COPY . /ECS_150/

# Set the default command to run when the container starts
CMD ["/bin/bash"]








