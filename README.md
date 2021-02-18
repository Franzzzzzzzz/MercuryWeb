Web interface for Mercury DPM. Allows to run the ProtectiveWall Advanced Tutorial from a browser, running the simulation remotely and displaying the results locally using [vtk.js](https://kitware.github.io/vtk-js/index.html).

# Installing 

There are quite a few steps:

- Download the repository: 

```
git clone https://github.com/Franzzzzzzzz/MercuryWeb.git
cd MercuryWeb
```
## Installing the back-end server (Mercury DPM server)

- Download [Mercury](http://mercurydpm.org/downloads/Trunk), saving the sources in a subfolder `Mercury_src` (for the patches to be applied correctly).

`svn checkout https://svn.MercuryDPM.org/SourceCode/Trunk --username guest --password '' Mercury_src`

- A few adjustments are required for Mercury to write vtk file in a format understood by [vtk.js](https://kitware.github.io/vtk-js/index.html). Let's apply the patches:

```
cd Mercury_modif
./applypatches.sh
cd ..
```

- We also need a couple of files to build the server version of the ProtectiveWall:

```
cp CppServer/ProtectiveWall_Server.cpp CppServer/Server.h Mercury_src/Drivers/Tutorials/Advanced/
```

- Now we can compile (here using 4 threads):

```
mkdir Mercury_build
cmake ../Mercury_src 
make fullTest -j 4
```

- Let's get the server in a more convenient location:

`cp ./Drivers/Tutorials/Advanced/ProtectiveWall_Server ../Software`

## Installing the front-end server (webkit server)

- A 'few' node modules are required. Hopefully this will install them directly:

```
sudo apt install npm
npm install .
```





