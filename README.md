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
attach to your running program. Conceptually, there is a server running in your
container that manages the Linux side of debugging, and Visual Studio Code connects
to this server to issue debugging commands and display the state of your program
within the editor.

## Debugging
To debug a program, you run it as follows:

_First compile it without address sanitizer_
* ```make clean; DEBUGGER=true make```

_In the Visual Studio Code terminal_
* [Linux and Windows host] ```gdbserver localhost:1234 ./gunrock_web```
* [Mac host] ```ROSETTA_DEBUGSERVER_PORT=1234 ./gunrock_web```

_Then in Visual Studio Code editor_
* Select "Run and Debug" on the left hand control pane
* Select the "GDB Gunrock Web" configuration and hit the play button

This will connect your Visual Studio Code debugger to your program running in your
container.

## Hints
Here are a few hints to help, but Visual Studio Code is widely used software so checking
online for help when you get stuck will be useful:
* Select "Auto Save" in Visual Studio Code to ensure that it saves your files as you edit.
* In our examples we use port `1234` for the debugger, but you can change this port if needed, just make sure that you update your `launch.json` file to match.
* To get started with a project, you will typically clone our [ECS 150 repo on GitHub](https://github.com/kingst/ecs150-projects) using Visual Studio Code "Clone git repository" option and Visual Studio Code will set up a Dev Container for you based on the confiugrations we included in that repo.
* If we push an update in the middle of a project, you can use `git pull` in your host terminal to fetch the latest updates. There is probably a way to do this in Visual Studio Code as well.