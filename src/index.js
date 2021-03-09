import vtkFullScreenRenderWindow from 'vtk.js/Sources/Rendering/Misc/FullScreenRenderWindow';

import vtkActor           from 'vtk.js/Sources/Rendering/Core/Actor';
import vtkColorTransferFunction from 'vtk.js/Sources/Rendering/Core/ColorTransferFunction';
//import vtkCalculator      from 'vtk.js/Sources/Filters/General/Calculator';
//import vtkConeSource      from 'vtk.js/Sources/Filters/Sources/ConeSource';
import vtkMapper          from 'vtk.js/Sources/Rendering/Core/Mapper';
//import { AttributeTypes } from 'vtk.js/Sources/Common/DataModel/DataSetAttributes/Constants';
//import { FieldDataTypes } from 'vtk.js/Sources/Common/DataModel/DataSet/Constants';

import controlPanel from './controller.html';
import vtkHttpDataAccessHelper from 'vtk.js/Sources/IO/Core/DataAccessHelper/HttpDataAccessHelper';
const { fetchBinary } = vtkHttpDataAccessHelper;
//FG
import vtkHttpDataSetReader from 'vtk.js/Sources/IO/Core/HttpDataSetReader';
import vtkXMLPolyDataReader from 'vtk.js/Sources/IO/XML/XMLPolyDataReader';
import vtkGlyph3DMapper from 'vtk.js/Sources/Rendering/Core/Glyph3DMapper';
import vtkSphereSource from 'vtk.js/Sources/Filters/Sources/SphereSource';
//=============================
const server="localhost:54321"
//=============================

// ----------------------------------------------------------------------------
// Standard rendering code setup
// ----------------------------------------------------------------------------

const fullScreenRenderer = vtkFullScreenRenderWindow.newInstance();
const renderer = fullScreenRenderer.getRenderer();
const renderWindow = fullScreenRenderer.getRenderWindow();

// ----------------------------------------------------------------------------
// Example code
// ----------------------------------------------------------------------------

const mapper = vtkGlyph3DMapper.newInstance();
const sphereSource = vtkSphereSource.newInstance();
const writerReader = vtkXMLPolyDataReader.newInstance();
const vtkColorMaps = vtkColorTransferFunction.vtkColorMaps;
const preset = vtkColorMaps.getPresetByName('Inferno (matplotlib)');
const lookupTable = vtkColorTransferFunction.newInstance();
//sphereSource.setResolution(12);
mapper.setInputConnection(sphereSource.getOutputPort(), 1);
//mapper.setOrientationArray('pressure');
mapper.setScaleArray('Radius') ; 
mapper.setScaleMode(1); 
mapper.setScaleFactor(2) ; 
mapper.setScalarMode(1) ; 
lookupTable.setVectorModeToMagnitude(); 
lookupTable.applyColorMap(preset);
lookupTable.setMappingRange(-0.0001,0.0001);
lookupTable.updateRange();
mapper.setLookupTable(lookupTable);
mapper.setColorByArrayName('Radius');
mapper.setColorModeToMapScalars();
mapper.setInterpolateScalarsBeforeMapping();
mapper.setScalarModeToUsePointFieldData();
mapper.setScalarVisibility(true);
//mapper.setScalarRange(0.009, 0.011);
const actor = vtkActor.newInstance();
actor.setMapper(mapper);

renderer.addActor(actor);

// fetch('res/protectiveWallParticle_100.vtu').then(data => {console.log(data);}) ; 
// writerReader.setUrl(`res/protectiveWallParticle_100.vtu`, { loadData: true }).then(() => {
// mapper.setInputConnection(writerReader.getOutputPort(), 0);
// 
// renderer.resetCamera();
// renderWindow.render();
// 
// });




// -----------------------------------------------------
// Time handling
//------------------------------------------------------
let filelist = [] ; 

function downloadTimeSeries() {
  return Promise.all(
    filelist.map((filename) => {
        /*{
        const reader = vtkXMLPolyDataReader.newInstance();
        reader.setUrl(filename, { loadData: true }).then(() => {
            return reader.getOutputPort();
        })
        }*/
      //fetchBinary(`${filename}`).then((binary) => {
        const reader = vtkXMLPolyDataReader.newInstance();
        reader.setUrl(filename, { loadData: true })/*.then( () => {
        //reader.parseAsArrayBuffer(binary);
        return reader;}) ;*/
        return reader; }
      //})
    )
  );
}

function getwall(username) {
    const mapper2 = vtkMapper.newInstance();
    const actor2 = vtkActor.newInstance();
    const reader = vtkXMLPolyDataReader.newInstance();
    reader.setUrl(username+'/protectiveWallWall_0.vtu', { loadData: true });
    reader.loadData().then( () => {
    mapper2.setInputConnection(reader.getOutputPort(),0) ; 
    actor2.setMapper(mapper2);
    renderer.addActor(actor2);
    
    })
}

function setVisibleDataset(ds) {
  console.log(ds) ; 
  mapper.setInputConnection(ds.getOutputPort(), 0);
  //renderer.resetCamera();
  //console.log(mapper)
  renderWindow.render();
}

function checkstatus () {
 console.log("Hello") ;    
}

// -----------------------------------------------------------
// UI control handling
// -----------------------------------------------------------

fullScreenRenderer.addController(controlPanel);
const runsimu = document.querySelector('.runsimu') ; 
const resetcam = document.querySelector('.resetcam') ; 
const timeslider = document.querySelector('.timeslider');
//const representationSelector = document.querySelector('.representations');
//const resolutionChange = document.querySelector('.resolution');
let timeSeriesData = [];

// Run a simulation
runsimu.addEventListener('click', (e) => {
//runsimu.disabled = true; 
runsimu.value = "wait ..." ; 
filelist=[] ; 
const maxtime = document.getElementById("t").value ; 
username=document.getElementById("username").value ;
if (username=="")
{
    alert("A username is required (typically your unikey)") ; 
    runsimu.disabled = false; runsimu.value="Run"; 
    return; 
}

const url='http://'+server+'/run?username='+username+'&Np=' + document.getElementById("Np").value + "&r=" + document.getElementById("R").value + "&h=" + document.getElementById("h").value + "&w=" + document.getElementById("w").value + "&l=" + document.getElementById("l").value + "&s=" + document.getElementById("s").value + "&t=" + maxtime
mapper.setScalarRange(0.9*document.getElementById("R").value, 1.1*document.getElementById("R").value);

document.getElementById("progression").hidden=false ; 
document.getElementById("progression").value = 0 ; 
var intervalID = window.setInterval(() => {
 fetch('http://'+server+'/status?username='+username)
 .then(response => response.json())
 .then(data => {document.getElementById("progression").value=Math.round(data.completion);})}, 1000);


fetch(url).then(data => {
    // Once the simulation is finished, create filelist
    console.log(data.status) ; 
    
    if (data.status== 200) // Everything worked properly
    {
        for (var i=0; i<maxtime*1000 ; i+=5)
            filelist.push(username+"/protectiveWallParticle_"+i+".vtu") ; 
        console.log(filelist) ; 
        getwall(username) ; 
        
        downloadTimeSeries().then((downloadedData) => {
            timeSeriesData = downloadedData.filter((ds) => true);
            //timeSeriesData.sort((a, b) => getDataTimeStep(a) - getDataTimeStep(b));

            console.log(timeSeriesData[10]) ; 
            timeslider.max = filelist.length - 1 ; 
            timeslider.value = 10;

            timeSeriesData[10].loadData().then ( () =>
            {renderer.resetCamera(); setVisibleDataset(timeSeriesData[10]); })
            });
        runsimu.disabled = false; runsimu.value="Done. Run again?"; 
    }
    else if (data.status==400)
    {
        alert("Error 400: An unknown error occured.") ; 
        runsimu.disabled = false; runsimu.value="Run"; 
    }
    else if (data.status==403)
    {
        alert("Error 403: you are not authorised to run simulations. Mispelled username?") ;
        runsimu.disabled = false; runsimu.value="Run"; 
    }
    else if (data.status==408)
    {
        alert("Error 408: your simulation timed out, you probably have too many grains/timestep, or not enough authorisation") ; 
        runsimu.disabled = false; runsimu.value="Run"; 
    }
    else if (data.status==409)
    {
        alert("Error 409: you already seem to have a simulation running. You need to wait.") ; 
        runsimu.disabled = false; runsimu.value="Run"; 
    }
    else if (data.status==503)
    {
        alert("Error 503: Too many simulations running on the server, you need to wait for some of them to finish.") ; 
        runsimu.disabled = false; runsimu.value="Run"; 
    }
    else
    {
        alert("Error "+data.status+": unknown error.") ; 
        runsimu.disabled = false; runsimu.value="Run";  
    }
    window.clearInterval(intervalID);
    document.getElementById("progression").hidden=true ; 
    }) ;
    
}) ;  

resetcam.addEventListener('click', (e) => {renderer.resetCamera(); renderWindow.render();}) ; 

timeslider.addEventListener('input', (e) => {
  const curdata = timeSeriesData[Number(e.target.value)] ;
  curdata.loadData().then( () => 
  { setVisibleDataset(curdata);} )
});

console.log(filelist) ; 



/*
resolutionChange.addEventListener('input', (e) => {
  const resolution = Number(e.target.value);
  coneSource.setResolution(resolution);
  renderWindow.render();
});*/





/*reader.setUrl(`protectiveWallParticle_780.vtu`, { loadData: true }).then(() => {
  //const fileContents = writer.write(reader.getOutputData());
    console.log(reader.getOutputData()); 
    
  // Try to read it back.
  const textEncoder = new TextEncoder();
  writerReader.parseAsArrayBuffer(textEncoder.encode(fileContents));
  renderer.resetCamera();
  renderWindow.render();

  const blob = new Blob([fileContents], { type: 'text/plain' });
  const a = window.document.createElement('a');
  a.href = window.URL.createObjectURL(blob, { type: 'text/plain' });
  a.download = 'cow.vtp';
  a.text = 'Download';
  a.style.position = 'absolute';
  a.style.left = '50%';
  a.style.bottom = '10px';
  document.body.appendChild(a);
  a.style.background = 'white';
  a.style.padding = '5px';
});*/
