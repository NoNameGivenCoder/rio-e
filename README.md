# RIO(e)

RIO(e) is a integrated editor created to make game development using the engine, [rio](https://github.com/aboood40091/rio) easier. RIO is an engine that provides compilation options for both PC and Wii U, RIO(e) expands on this, adding a full integrated development environment, with texture viewing and manipulation, model creation and viewing, audio previewing and playback, saveable layouts, customizable properties, and much more.

Please refer to [rio](https://github.com/aboood40091/rio) for any guidence with engine specific questions.

**Note, RIO(e) is still in development, it is currently not in a usable state. Use at your own discretion.**

## Features

### Nodes

Nodes are the basic building blocks to RIO(e). They are able to be moved around the scene, and allow for properties to be attatched to them.

### Properties

Properties are the main way to control nodes. They can be attatched to a node and allow for control over a node. An example of this could be controlling a nodes position, via the Wii U gamepad's touch control.

As of now, creating custom properties is a bit tedious, so a solution for that will be created eventually. For now, the base properties RIO(e) has so far are:

- `AudioProperty` - Used to playback BGM and SFX sounds. Essentially a wrapper for rio::AudioMgr. (Which is a wrapper for SDL2_mixer)
  
- `MeshProperty` - Used to display meshes throughout your scene.
  
- `PrimitiveProperty` - Used to display primitives. (Spheres, cubes, quads, etc).
  
- `CameraProperty` - Used to position the camera within the scene. (There should only be one per scene at a time).
  
- `MiiHeadProperty` - Used to display a Mii Head. Refer to FFLMgr for more info.
  

Examples of how to utilize properties and nodes can be found in `src/helpers/properties/examples` and `include/helpers/properties/examples` respectively.

- `void Property::Load(YAML::Node node)` - Allows loading from a YAML::Node. Called when a layout file is being loaded.
  
- `YAML::Node Property::Save()` - Allows for saving. Used in the editor only, do **not** expect to use in normal gameplay.
  
- `void Property::Start()` - Called at the beginning of the scene. Useful to initializing values and preparing.
  
- `void Property::Update()` - Called every frame.
  

### Tasks

Tasks are the same as RIO, although with a few differences. Tasks are the way for a developer to orchestrate a scene. Think of a central manager for the scene. Here is an example of a basic task.

```cpp
#include <RootTask.h>
#include <rio.h>
#include <helpers/common/NodeMgr.h>

RootTask::RootTask() : ITask("Example Task"), mInitialized(false)
{
}

void RootTask::prepare_()
{
  mInitialized = false;

  NodeMgr::instance()->LoadFromFile("ExampleLayout.yaml");
  NodeMgr::instance()->Start();

  mInitialized = true;
}

void RootTask::calc_()
{
  if (!mInitialized)
    return;

  NodeMgr::instance()->Update();
}

void RootTask::exit_()
{
  if (!mInitialized)
    return;

  mInitialized = false;
}
```

Loading a layout file is as easy as calling `NodeMgr::instance()->LoadFromFile("")`. Note, layout files are located in the **native content folder's** `map` directory. For PC it's `fs/content` and on Wii U it is `content`.

### FFLMgr

FFLMgr is a global class that has different helper functions related to the library FFL.

- `void FFLMgr::InitializeFFL()` - Called to initialize FFL. Usually should be called during a task's `prepare_()` method.
  
- `void FFLMgr::CreateRandomMiddleDB(u16 pMiiLength)` - Called to create a MiddleDB filled with `pMiiLength` miis.
  
- `FFLStoreData FFLMgr::GetStoreDataFromFile(std::string fileName, rio::RawErrorCode *errCode)` - Called to grab mii data from a .ffsd file located in the **native content folder's** `mii` directory.
  
- `FFLResolution FFLMgr::GetGlobalResolution()` - Returns the global resolution
  

### Editor

The editor currently is being worked on the most, and is a very early work in progress. Here is a photo for reference.

![Photo of editor](https://raw.githubusercontent.com/NoNameGivenCoder/rio-e/master/readme/Editor_01.png)
(A Mii with a chaos emerald? Woah, crazy technology!)

The editor UI is made with [Dear ImGui](https://github.com/ocornut/imgui). Properties that are selected within the editor can use ImGui within the `CreatePropertiesMenu()` method, to allow changing of certain default values if necessary.

### Notes

- As stated above, this is a **work in progress** and does not represent what the final product of this project is. Please use at your discretion.
  
- This is one of my first big C++ projects. If any of my code looks wrong or incorrect, don't be afraid to shoot me a PR or let me know within an issue, I'm always open to taking constructive criticism.
  

### Dependencies

These are the ones that I can currently name, though if you cannot build for whatever reason, please let me know.

- OpenGL (Minimum v4.0 core profile)
  
- GLEW
  
- GLFW3
  
- SDL2_mixer
  
- [rio](https://github.com/aboood40091/rio)
  
- [ninTexUtils](https://github.com/aboood40091/ninTexUtils/tree/cpp)
  
- [ffl](https://github.com/aboood40091/ffl)
  
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
  

## Credits

- Thank you to @aboood40091 for the creation of [rio](https://github.com/aboood40091/rio), [ninTexUtils](https://github.com/aboood40091/ninTexUtils/tree/cpp), decompilation of [ffl](https://github.com/aboood40091/ffl), and overall being a great help in general.
  
- Thank you to @ariankordi for porting rio to [linux](https://github.com/ariankordi/rio), general guidence when it came to FFL, and also being a great help in general.
