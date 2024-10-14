# ECS 150 programming projects

These projects are meant to get you warmed up with programming in the C++/UNIX
environment. None are meant to be particularly hard, but should be enough so
that you can get more comfortable programming. 

Realize the best thing you can do to learn to program in any environment is to
program **a lot**. These small projects are only the beginning of that
journey; you'll have to do more on your own to truly become proficient.

* [Unix Utilities](project1)
* [Unix Shell](project2)
* [Concurrent Web Server](project3)

Tip of the hat to Remzi and Andrea Arpaci-Dusseau for the amazing
[textbook](https://pages.cs.wisc.edu/~remzi/OSTEP/) and
[projects](https://github.com/remzi-arpacidusseau/ostep-projects),
which we borrow from heavily.

# Using Visual Studio Code and Docker

In this class, the supported configuration is to install Docker Desktop and Visual
Studio Code on your local computer, or to use the Visual Studio Code instances
on the CSIF lab machines.

## Installation
To install these packages, follow the installation instructions [here](https://code.visualstudio.com/docs/devcontainers/containers).
We already have your devcontainer and other settings configured, and Visual Studio
Code will manage Docker for you, but the instructions have links to instructions for
installing Docker on Windows, Mac, and Linux machines.

## Concepts
Conceptually, Visual Studio Code is an application running on your computer. It creates a virtual machine,
using Docker, that has the right version of Linux for this class. Visual Studio Code
handles synchronization between files on your local computer and the virtual machine
and exposes a terminal where you can use the command line to interact with your code
running within the virtual machine, which is also called a dev container.

In a typical configuration you will edit your code using Visual Studio Code, and
compile and run your code using the terminal.

To debug your code using the Visual Studio Code debugger, you need to run your
program using the terminal and then use one of our pre-set configurations to
connect
