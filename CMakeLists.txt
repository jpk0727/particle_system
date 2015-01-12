INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} .)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5OpenGL REQUIRED)

#QT headers requiring MOC (meta object compiler)
set(QTHDRS 
  mainwindow.h 
  mypanelopengl.h
  particle.h
  octree.h
  ISpatialStructure.h
  Vector3.h
  #ISpacialCell.h
  #ISpacialObject.h
  #base.h

  )

#window design files
set(QTFORMS mainwindow.ui)

#Run qt tools to auto generate C++ compatible files
QT5_WRAP_CPP(QTHDRS_MOC ${QTHDRS})
QT5_WRAP_UI(QTFORMS_HDRS ${QTFORMS})
SET(MYQTFILES ${QTHDRS_MOC} ${QTFORMS_HDRS})

#normal source files
set(SRC
  main.cpp 
  mainwindow.cpp 
  mypanelopengl.cpp
  particle.cpp
  octree.cpp

)

set(SHADERS
  vshader2.glsl vshader4.glsl
  fshader2.glsl fshader4.glsl
)

SET(MYLIBS ${EXT_LIBS} libcommon common_rgb common_noise)

ADD_EXECUTABLE(particle ${SRC} ${SHDRS} ${MYQTFILES} ${SHADERS})

TARGET_LINK_LIBRARIES(particle ${EXT_LIBS} libcommon
  Qt5::Core Qt5::Gui Qt5::OpenGL)

symlink_data(particle_shader *.glsl)



