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
We can start both server locally and check if everything is ok. We need to run the backend server from a specific folder though:

```
npm start &
cd dist/res
../../Software/ProtectiveWall_Server &
cd ../..
```

Now navigating to `http://localhost:8080/` will connect to the front-end sever, allow to change the simulation parameters, run the simulation, and display the results. When clicking on the run button, the back-end server is contacted (it can also be contacted directly from the browser at `http://localhost:54321/run?Np=50&t=...`). If everything looks good, we can deploy:

## Deploy
First, we need to adapt a variable in the javascript to fit the back-end server to be contacted for the calculation. Modify the `server` variable in the file `src/index.js` accordingly to the IP and port to contact the backend. Default port is 54321, as shown above. The front-end open port is 54322 when using the `startserver` script as shown below. To start everything:

```
npm run startserver && cd dist/res && ../../Software/ProtectiveWall_Server &&
```

but if everything is working well, we probably also want to be able to disconnect, so running all that with `nohup` is probably better. But really, a proper server deamon that would restart this programs on crash is the long term / proper solution...










