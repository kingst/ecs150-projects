
## Getting Started:

In ECS 150 we’re going to use Docker to help make sure that your project development environment matches what you’ll have when we autograde your homework in Gradescope. By ensuring that the environments are the same, we will eliminate a large class of difficult-to-debug issues that come from inconsistencies. This document outlines the basics of how to get started using Docker on your computer.

**Install Docker** using the instructions [here](https://docs.docker.com/get-docker/). There are two installation methods listed, both are viable and will install the Docker CLI.

**Validate Docker installation** by typing ‘docker’ in the command line.


## Setting up a working directory:

**Create a working directory** by creating a folder on your local machine.

**Put the Dockerfile in the working directory**, this is the root of your working directory.




![alt_text](images/image1.png "image_tooltip")



## Building a Docker Image and Running the Container:

**Run the following commands**:



1. **docker  build -t &lt;name of the image> .**

     \
What it does → ‘docker build’ creates a docker image with the name after ‘-t’, using the Dockerfile in the directory ‘.’ (The current directory)


    Confirm that the build was successful, by making sure there were no errors


    


![alt_text](images/image2.png "image_tooltip")


2. **docker run -it &lt;name of the image>**

    What it does → ‘docker run’ will now run the commands specified either in the Dockerfile or in the ‘docker run’ [options](https://docs.docker.com/reference/cli/docker/container/run/), this is currently set to run a bash shell by default to give you access to a terminal on run. The ‘-it’ is a mix of two tags ‘-i’ and ‘-t’ which make the container interactive and specify the tag/name, of the image respectively.


    

![alt_text](images/image3.png "image_tooltip")


3. **docker ps**

    What it does → ‘docker ps’ will show all the currently running containers


    



![alt_text](images/image4.png "image_tooltip")


4. **Ctrl+D**

    What it does → The above keys will exit out of the container shell, any container made using the provided docker file, will also stop immediately. This is because Docker containers stop immediately after all commands have been run.


    



![alt_text](images/image5.png "image_tooltip")


5. **docker stop [container id]**

    What it does → ‘docker stop’ will stop the provided container, and your files will remain in the designated folders in the container. To start the container again, run the abovementioned ‘docker run -it’ command



## Running Programs

Compile and run executables normally as you would outside the container (your local terminal)


## Listing and Stopping a Container


### Listing

**docker ps**

Lists running containers

**docker ps -a** 

Lists all containers


### Stopping

**docker** **stop [container id]** 

This stops the particular container, you can get the container ID from the above commands

**docker stop $(docker ps -a)**

This command stops all containers


### Removals

**docker rm [container id]**

**docker rm $(docker ps -a)**

The above commands remove a container, this is helpful if you plan on making multiple containers to test files parallelly.


## The Dockerfile

The Dockerfile in question is a simple set of commands, it **first** looks for/ pulls the required OS image. In this case, we are using a Ubuntu image.

**Second**, package lists and libraries are installed, if any extra libraries are needed, please add them here in the same format. (You will have to rebuild the image, after any changes to the Dockerfile)

**Third,** the contents of your current directory are copied to the docker container directory. This only happens on the initial build. If you want to add other files/directories you will have to do so in the docker shell or copy the files using the command:

**docker cp &lt;source file name> &lt;container_id>:/&lt;destination file name>**

Or you can build again.


## Troubleshooting

In general, building the image shouldn’t take more than a few seconds. Please let us know if there are inconsistencies here.

It is possible that the PATH variable is not updated on installation, you can update the PATH variable via the terminal (using ‘export’ in mac or ‘set’ in windows) or update it in that shell’s file.
