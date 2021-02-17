import vtkFullScreenRenderWindow from 'vtk.js/Sources/Rendering/Misc/FullScreenRenderWindow';

import vtkActor           from 'vtk.js/Sources/Rendering/Core/Actor';
import vtkColorTransferFunction from 'vtk.js/Sources/Rendering/Core/ColorTransferFunction';
//import vtkCalculator      from 'vtk.js/Sources/Filters/General/Calculator';
//import vtkConeSource      from 'vtk.js/Sources/Filters/Sources/ConeSource';
import vtkMapper          from 'vtk.js/Sources/Rendering/Core/Mapper';
//import { AttributeTypes } from 'vtk.js/Sources/Common/DataModel/DataSetAttributes/Constants';
//import { FieldDataTypes } from 'vtk.js/Sources/Common/DataModel/DataSet/Constants';

import controlPanel from './controller.html';

//FG
import vtkHttpDataSetReader from 'vtk.js/Sources/IO/Core/HttpDataSetReader';
import vtkXMLPolyDataReader from 'vtk.js/Sources/IO/XML/XMLPolyDataReader';
import vtkGlyph3DMapper from 'vtk.js/Sources/Rendering/Core/Glyph3DMapper';
import vtkSphereSource from 'vtk.js/Sources/Filters/Sources/SphereSource';
// ----------------------------------------------------------------------------
// Standard rendering code setup
// ----------------------------------------------------------------------------

const fullScreenRenderer = vtkFullScreenRenderWindow.newInstance();
const renderer = fullScreenRenderer.getRenderer();
const renderWindow = fullScreenRenderer.getRenderWindow();

// ----------------------------------------------------------------------------
// Example code
// ----------------------------------------------------------------------------

const writerReader = vtkXMLPolyDataReader.newInstance();
writerReader.setUrl(`protectiveWallParticle_4999.vtu`, { loadData: true }).then(() => {

  //console.log(writerReader.getOutputData())
  //console.log(writerReader.getOutputData().getPoints().get().values[2]) ;   
  const mapper = vtkGlyph3DMapper.newInstance();
mapper.setInputConnection(writerReader.getOutputPort(), 0);
const sphereSource = vtkSphereSource.newInstance();
//sphereSource.setResolution(12);
mapper.setInputConnection(sphereSource.getOutputPort(), 1);
//mapper.setOrientationArray('pressure');
mapper.setScaleArray('Radius') ; 
mapper.setScaleMode(1); 
mapper.setScaleFactor(2) ; 
mapper.setScalarRange(0.0, 0.1);
mapper.setScalarMode(1) ; 
const vtkColorMaps = vtkColorTransferFunction.vtkColorMaps;
console.log(vtkColorMaps)
const preset = vtkColorMaps.getPresetByName('Inferno (matplotlib)');
const lookupTable = vtkColorTransferFunction.newInstance();
lookupTable.setVectorModeToMagnitude();
console.log(preset) ; 
lookupTable.applyColorMap(preset);
lookupTable.setMappingRange(-0.0001,0.0001);
lookupTable.updateRange();
mapper.setLookupTable(lookupTable);
mapper.setColorByArrayName('Velocity');
mapper.setColorModeToMapScalars();
mapper.setInterpolateScalarsBeforeMapping();
mapper.setScalarModeToUsePointFieldData();
mapper.setScalarVisibility(true);
mapper.setScalarRange(0.0,0.0001);
console.log(mapper.getScalarVisibility())
//const mapper = vtkMapper.newInstance();
//mapper.setInputConnection(filter.getOutputPort());
console.log(mapper)

const actor = vtkActor.newInstance();
actor.setMapper(mapper);

renderer.addActor(actor);
renderer.resetCamera();
renderWindow.render();

});

// -----------------------------------------------------------
// UI control handling
// -----------------------------------------------------------

/*fullScreenRenderer.addController(controlPanel);
const representationSelector = document.querySelector('.representations');
const resolutionChange = document.querySelector('.resolution');

representationSelector.addEventListener('change', (e) => {
  const newRepValue = Number(e.target.value);
  actor.getProperty().setRepresentation(newRepValue);
  renderWindow.render();
});

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
