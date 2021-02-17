import 'https://unpkg.com/vtk.js/Sources/favicon';

import vtkActor from 'https://unpkg.com/vtk.js/vtk.js/Sources/Rendering/Core/Actor';
import vtkFullScreenRenderWindow from 'https://unpkg.com/vtk.js/Sources/Rendering/Misc/FullScreenRenderWindow';
import vtkMapper from 'https://unpkg.com/vtk.js/Sources/Rendering/Core/Mapper';
import vtkPolyDataReader from 'https://unpkg.com/vtk.js/Sources/IO/Legacy/PolyDataReader';

const fileName = 'sphere.vtk'; // 'uh60.vtk'; // 'luggaBody.vtk';

// ----------------------------------------------------------------------------
// Standard rendering code setup
// ----------------------------------------------------------------------------

const fullScreenRenderer = vtkFullScreenRenderWindow.newInstance();
const renderer = fullScreenRenderer.getRenderer();
const renderWindow = fullScreenRenderer.getRenderWindow();

const resetCamera = renderer.resetCamera;
const render = renderWindow.render;

// ----------------------------------------------------------------------------
// Example code
// ----------------------------------------------------------------------------

const reader = vtkPolyDataReader.newInstance();
reader.setUrl(`${__BASE_PATH__}/data/legacy/${fileName}`).then(() => {
  const polydata = reader.getOutputData(0);
  const mapper = vtkMapper.newInstance();
  const actor = vtkActor.newInstance();

  actor.setMapper(mapper);
  mapper.setInputData(polydata);

  renderer.addActor(actor);

  resetCamera();
  render();
});

// -----------------------------------------------------------
// Make some variables global so that you can inspect and
// modify objects in your browser's developer console:
// -----------------------------------------------------------

global.reader = reader;
global.fullScreenRenderer = fullScreenRenderer;
