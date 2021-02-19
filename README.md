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

- A few adjustments are required for Mercury to write vtk file in a format understood by [vtk.js](https://kitware.github.io/vtk-js/index.html). Also, the server requires the pthread library to compile, so let's apply a few patches to a few files using the supplied bash script:

```
cd Mercury_modif
./applypatches.sh
cd ..
```

- We also need a couple of files to build the server version of the ProtectiveWall:

```
cp CppServer/ProtectiveWall_Server.cpp CppServer/Server.h Mercury_src/Drivers/Tutorials/Advanced/
```

- Now we can compile. To speed things up we can just focus on the server compilation:

```
mkdir Mercury_build
cmake ../Mercury_src 
cd Drivers/Tutorials/Advanced/
make 
cd ../../../..
```

- Let's get the server in a more convenient location:

`cp Mercury_build/Drivers/Tutorials/Advanced/ProtectiveWall_Server Software`

## Installing the front-end server (webkit server)

- A 'few' node modules are required. Hopefully this will install them directly:

```
sudo apt install npm
npm install .
```

# Running

## Testing
We can start both server locally and check if 





