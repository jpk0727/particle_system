/*Zoe Junghans and Jess Karol
 *CS40 Final Project
 *Description: This project models a particle system which begins to approximate the movement of water.
 *The particle system allows for particle-particle collisions as well as particle-wall collisions with the static environment (a cube) we created.
 *It incorporates the use of an octree which allows for large performance benefits and the ability to introduce a much larger number of particles into the system.
 *Finally, the particles have been made to respond correctly to transformations applied to the static environment.
 */

#include "mypanelopengl.h"
#include "common/matrixstack.h"
#include <cmath>
#include <iostream>
#include <math.h>

using std::cout;
using std::endl;
using cs40::Sphere;
using cs40::MatrixStack;

//Constructor
MyPanelOpenGL::MyPanelOpenGL(QWidget *parent) :
  QGLWidget(parent), m_angles(0,0,0){

    //Sets shaders to null
    for(int i=0; i<CS40_NUM_PROGS; i++){
      m_shaderPrograms[i]=NULL;
      m_vertexShaders[i]=NULL;
      m_fragmentShaders[i]=NULL;
    }

    m_sphere = NULL;
    m_sphereSize = 10;
    m_polyMode = 0;
    m_curr_prog = 0;

    //Initializes number of particles
    m_nparticles = 1000;
    orignum = m_nparticles;

    //Initializes time and timer
    m_timer = NULL;
    m_time = 0;
    srand(time(NULL));

    vboData = NULL;
    m_fountain = NULL;

    //Initializes cube variables
    numVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
    points = new point4[numVertices];
    colors = new color4[numVertices];
    index = 0;
    vertices = new point4[8];
    vertex_colors = new color4[8];

    theta = vec3(0,0,0);

    //Initializes octree
    list = new Octree(Vector3(0,0,0), 1.1, 0);
  }

//Deconstructor
MyPanelOpenGL::~MyPanelOpenGL(){

  delete m_sphere; m_sphere=NULL;
  delete m_timer; m_timer=NULL;

  if(m_fountain){
    m_fountain->release();
    delete m_fountain; m_fountain=NULL;
  }

  //Deletes all particle objects
  for(int i = 0; i < m_nparticles; i++){
    delete allParticles[i];
  }

  for(int i=0; i<CS40_NUM_PROGS; i++){
    destroyShaders(i);
  }

  delete list;
  delete [] vertices; vertices = NULL;
  delete [] vertex_colors; vertex_colors = NULL;
  delete [] points; points=NULL;
  delete [] colors; colors=NULL;

  destroyVBO();
}

//Initializes shaders, textures, timer, and camera matricies
void MyPanelOpenGL::initializeGL()
{

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  //Constructs the cube
  makeCube();

  //Creates the two shaders
  //The first shader is for the particles, the second is for the cube
  createShaders(0, "vshader2.glsl", "fshader2.glsl");
  createShaders(1,"vshader4.glsl", "fshader4.glsl");

  //Sets the particles' texture
  QPixmap img("data/ball.png");
  m_texture = bindTexture(img, GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_texture);

  //Calls makeFountain to initialize all of the particles
  m_sphere = new Sphere(0.5,30,30);
  makeFountainSmall(0);

  //Sets camera matricies
  m_projection.perspective(40,1,1,8);
  m_camera.lookAt(vec3(0,0,3),vec3(0,0,0),vec3(0,1.,0.));
  updateModel();

  //Sets timer information
  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), this, SLOT(step()));
  m_timer->start(10);

  cout << "Keyboard commands: " << endl;
  cout << "  x: rotates camera in the x direction" << endl;
  cout << "  y: rotates camera in the y direction" << endl;
  cout << "  z: rotates camera in the z direction" << endl;
  cout << "  a: rotates cube in the x direction" << endl;
  cout << "  s: rotates cube in the y direction" << endl;
  cout << "  d: rotates cube in the z direction" << endl;
  cout << "  Shift + any of the above commmands performs the opposite rotation" << endl;
  cout << "  r: drops another set of particles into the scene (will not exceed 2000 particles)" << endl;
}

//Updates time and scene
void MyPanelOpenGL::step(){
  updateTime();
  updateGL();
}

//Increments time
void MyPanelOpenGL::updateTime(){
  m_time += 0.01;
}

//Resizes the window
void MyPanelOpenGL::resizeGL(int w, int h)
{
  glViewport(0,0,w, h);
}

//Paints the scene
void MyPanelOpenGL::paintGL(){

  //Clear both color and depth buffer
  glDisable(GL_POINT_SPRITE);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Two calls, one draws static scene one draws particles
  paintStatic();
  paintParticles();

  //swapBuffers(); /* not need in QT see QGLWidget::setAutoBufferSwap */
}

//Draws the static scene (the cube)
void MyPanelOpenGL::paintStatic(){

    //For transparency
    glDepthMask(GL_FALSE);

    //Sets to the correct shader
    m_curr_prog = 1;
    if(!m_shaderPrograms[m_curr_prog]){return; }

    //Binds and checks VBO
    vboData->bind();
    if(!vboData){
        return;
    }

    //Sets all shader attributes
    m_shaderPrograms[m_curr_prog]->bind();
    m_shaderPrograms[m_curr_prog]->enableAttributeArray("vPosition");
    m_shaderPrograms[m_curr_prog]->setAttributeBuffer("vPosition", GL_FLOAT, 0, 4, 0);

    m_shaderPrograms[m_curr_prog]->enableAttributeArray("vColor");
    m_shaderPrograms[m_curr_prog]->setAttributeBuffer("vColor", GL_FLOAT,numVertices*sizeof(point4),4,0);

    mat4 mview = m_camera*m_model;
    m_shaderPrograms[m_curr_prog]->setUniformValue("projection",m_projection);
    m_shaderPrograms[m_curr_prog]->setUniformValue("camera",m_camera);
    m_shaderPrograms[m_curr_prog]->setUniformValue("model", m_model);
    m_shaderPrograms[m_curr_prog]->setUniformValue("modelView",mview);
    m_shaderPrograms[m_curr_prog]->setUniformValue("normalMatrix",mview.normalMatrix());
    m_shaderPrograms[m_curr_prog]->setUniformValue("lightPos",vec4(1.5,0,2,1.)); //in world coordinates

    glDrawArrays(GL_TRIANGLES, 0, numVertices);

    glFlush();
    m_shaderPrograms[m_curr_prog]->release();

    vboData->release();

}

//Draws the particles
void MyPanelOpenGL::paintParticles(){

    //Sets to the correct shader
    m_curr_prog = 0;

    if(!m_shaderPrograms[m_curr_prog]){return;}
    m_shaderPrograms[m_curr_prog]->bind();

    //Sets all shader attributes
    mat4 mview = m_camera*m_model;
    m_shaderPrograms[m_curr_prog]->setUniformValue("projection",m_projection);
    m_shaderPrograms[m_curr_prog]->setUniformValue("camera",m_camera);
    m_shaderPrograms[m_curr_prog]->setUniformValue("model", m_model);
    m_shaderPrograms[m_curr_prog]->setUniformValue("modelView",mview);
    m_shaderPrograms[m_curr_prog]->setUniformValue("normalMatrix",mview.normalMatrix());
    m_shaderPrograms[m_curr_prog]->setUniformValue("lightPos",vec4(1.5,0,2,1.)); //in world coordinates

    //Added these lines to get different colors
    m_shaderPrograms[m_curr_prog]->enableAttributeArray("vColor");
    m_shaderPrograms[m_curr_prog]->setAttributeBuffer("vColor", GL_FLOAT, numVertices*sizeof(point4),4,0);

    updatePolyMode(100);

    //Changes the size of the particles
    glPointSize(m_sphereSize);

    glEnable(GL_POINT_SPRITE);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Draws all of the particles
    drawFountain();

    glFlush();
    m_shaderPrograms[m_curr_prog]->release();
}

//This slot is called when the "Small" radio button on the GUI is pushed
//Resets some variables and makes a new scene with small particles falling in a cube formation
void MyPanelOpenGL::paintSmall(){

    reset();

    m_nparticles = orignum;
    m_sphereSize = 10;

    //partColors = new vec4[m_nparticles];
    makeFountainSmall(0);
}

//This slot is called when the "Large" radio button on the GUI is pushed
//Resets some variables and makes a new scene with large particles falling from a single source with a time delay
void MyPanelOpenGL::paintLarge(){

    reset();

    m_nparticles = 64;
    m_sphereSize = 30;

    //partColors = new vec4[m_nparticles];
    makeFountainLarge();
}

//Resets the scene information
void MyPanelOpenGL::reset(){

    //Resets all particle information
    for(int i = 0; i < m_nparticles; i++){
        delete allParticles[i];
    }

    //Resets angles
    m_angles = vec3(0,0,0);
    updateAngles(0,0);
    index = 0;
    makeCube();

    //Resets octree
    delete list;
    list = new Octree(Vector3(0,0,0), 1.1, 0);

    m_time = 0;
}

/*Handles cases of various key strokes
 *  x: rotates camera in the x direction
 *  y: rotates camera in the y direction
 *  z: rotates camera in the z direction
 *  a: rotates cube in the x direction
 *  s: rotates cube in the y direction
 *  d: rotates cube in the z direction
 *  Shift + any of the above commmands performs the opposite rotation
 *  r: drops another set of particles into the scene (will not exceed 2000 particles)
 */
void MyPanelOpenGL::keyPressEvent(QKeyEvent *event)
{
  qreal step=1;
  /*Enable strong Focus on GL Widget to process key events*/
  switch(event->key()){
    case Qt::Key_X:
      if (event->text()=="x"){updateAngles(0,step);}
      else{updateAngles(0,-step);}
      break;

    case Qt::Key_Y:
      if (event->text()=="y"){ updateAngles(1,step);}
      else{ updateAngles(1,-step);}
      break;

    case Qt::Key_Z:
      if (event->text()=="z"){updateAngles(2,step);}
      else{updateAngles(2,-step);}
      break;

    case Qt::Key_C:
      if(glIsEnabled(GL_CULL_FACE)){glDisable(GL_CULL_FACE);}
      else{glEnable(GL_CULL_FACE);}
      break;

    case Qt::Key_P:
      m_polyMode = (m_polyMode+1)%3;
      updatePolyMode(m_polyMode);
      break;

    case Qt::Key_A:
      if (event->text()=="a"){rotateCube(-1);}
      else{rotateCube(1);}
      break;

    case Qt::Key_S:
      if (event->text()=="s"){rotateCube(-2);}
      else{rotateCube(2);}
      break;

    case Qt::Key_D:
      if (event->text()=="d"){rotateCube(-3);}
      else{rotateCube(3);}
      break;

    case Qt::Key_R:
      if(m_nparticles + orignum > 2000){
          break;
      }
      delete list;
      list = new Octree(Vector3(0,0,0), 1.1, 0);
      m_nparticles += orignum;
      makeFountainSmall(m_nparticles-orignum-1);
      break;

    default:
      QWidget::keyPressEvent(event); /* pass to base class */
  }
  updateGL();
}

//Generates a random float
float MyPanelOpenGL::randFloat(){
  static int digits = 10000;
  return (rand() % digits)/(1.*digits);
}

//Constructs the "Small" scene option, forms a cube of particles with random velocities that fall into the scene
void MyPanelOpenGL::makeFountainSmall(int index){

  //Chapter 9 OpenGL 4.0 GLSL cookbook

  //Attempts to create the VBO
  if(m_fountain){
      m_fountain->release();
      delete m_fountain; m_fountain=NULL;
  }
  m_fountain = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  bool ok = m_fountain->create();
  if(!ok){
    cout << "Unable to create VBO" << endl;
  }

  m_fountain->setUsagePattern(QOpenGLBuffer::DynamicDraw);

  vec3 v(0,0,0);
  float velocity, theta, phi;
  data = new float[m_nparticles*3]; //Initial velocities of particles
  partColors = new float[m_nparticles * 4];
  int ind = index;
  float b = 0.45; //For the blue component of the color

  //Initializes particle objects and sets their initial positions to make a cube formation
  for (int i=0; i<cbrt(m_nparticles); i++){
    for (int j = 0; j<cbrt(m_nparticles); j++){
      for (int k = 0; k<cbrt(m_nparticles);k++){
        if (ind < m_nparticles){

          theta = randFloat()*M_PI/8.+0.5;
          //Gives a 360 circle
          phi = randFloat()*2*M_PI;
          v.setX(-fabs(sin(theta) * cos(phi)) * 3);
          v.setY(cos(theta));
          v.setZ(sin(theta)*sin(phi));
          //Radius of circle
          velocity = .25 + 0.1*randFloat();
          v *= velocity;

          //Data to be written to VBO
          data[3*(ind)] = v.x();
          data[3*(ind)+1] = v.y();
          data[3*(ind)+2] = v.z();

          //partColors[ind] = vec4(.3,.4, b, 1.);
          //partColors[ind] = vec4(i/3,j/3, 0.8,1.);
          partColors[3*(ind)] = i/3;
          partColors[3*(ind)+1] = i/3;
          partColors[3*(ind)+2] = 0.8;
          partColors[3*(ind)+3] = 1;

          vec3 offset = vec3(.25,-1,.25);
          allParticles[ind] = new cs40::Particle(vec3(i,j,k)/20 - offset, v);
          ind++;
        }
      }
      b+=0.05;
    }
    b = 0.45;
  }

  //Sets initial times, radii, and collision status for all the particles
  float time_now = 0, time_step=0.0;
  for (int i=index; i<m_nparticles; i++){
    allParticles[i]->setStartTime(time_now); //add start times as a variable to particle
    time_now += time_step;
    allParticles[i]->VSetRadius(0.014);
    allParticles[i]->VCollisionOff();

  }

  makeOctree();

  //Writes information to the VBO
  m_fountain->bind();
  m_fountain->allocate(m_nparticles*(sizeof(vec3)) + m_nparticles*(sizeof(vec4)));
  m_fountain->write(0, data, m_nparticles*sizeof(vec3));
  m_fountain->write((m_nparticles*sizeof(vec3)), partColors, m_nparticles*sizeof(vec4));
  m_fountain->release();
}

//Constructs the "Large" scene option, particles fall from a source at a time offset
void MyPanelOpenGL::makeFountainLarge(){

  //Chapter 9 OpenGL 4.0 GLSL cookbook

  //Attempts to create the VBO
  if(m_fountain){
     m_fountain->release();
     delete m_fountain; m_fountain=NULL;
  }
  m_fountain = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  bool ok = m_fountain->create();
  if(!ok){
    cout << "Unable to create VBO" << endl;
  }
  m_fountain->setUsagePattern(QOpenGLBuffer::DynamicDraw);

  vec3 v(0,0,0);
  float velocity, theta, phi;
  data = new float[m_nparticles*3]; //Initial velocities of particles
  partColors = new float[m_nparticles * 4]; //Particle color information
  int ind = 0;

  //Initializes the velocity and positions of all particles
  for (int i=0; i<m_nparticles; i++){
      theta = randFloat()*M_PI/8.+0.5;
      //Gives a 360 circle
      phi = randFloat()*M_PI;
      v.setX(-50);
      v.setY(cos(theta));
      v.setZ(sin(theta)*sin(phi));
      //Radius of circle particles
      velocity = .25 + 0.1*randFloat();
      v *= velocity;

      //Data to be written to the VBO
      data[3*(i)] = v.x();
      data[3*(i)+1] = v.y();
      data[3*(i)+2] = v.z();

      //partColors[ind] = vec4(0.52,0.81,0.98,1.);
      partColors[3*(ind)] = 0.52;
      partColors[3*(ind)+1] = 0.81;
      partColors[3*(ind)+2] = 0.98;
      partColors[3*(ind)+3] = 1;

      allParticles[i] = new cs40::Particle(vec3(0.4,1,0), v);
  }

  //Sets initial times, radii, and collision status for all the particles
  float time_now = 0, time_step=0.2;
  for (int i=0; i<m_nparticles; i++){
    allParticles[i]->setStartTime(time_now); //add start times as a variable to particle
    time_now += time_step;
    allParticles[i]->VSetRadius(0.08);
    allParticles[i]->VCollisionOff();
  }

  makeOctree();

  //Writes information to the VBO
  m_fountain->bind();
  m_fountain->allocate(m_nparticles*(sizeof(vec3)) + m_nparticles*(sizeof(vec4)));
  m_fountain->write(0,data,m_nparticles*sizeof(vec3));
  m_fountain->write((m_nparticles*sizeof(vec3)) , partColors, m_nparticles*sizeof(vec4));
  m_fountain->release();
}

//Creates the octree by adding all of the particles into it
void MyPanelOpenGL::makeOctree(){

    std::vector <ISpatialObject*> toAdd;

    for(int i = 0; i <m_nparticles; i++){
         toAdd.push_back(allParticles[i]);
    }

    list->VAddObjects(toAdd);
}

/*Draws the particle system
 *  -Updates each particle position
 *  -Checks for wall-particle collisions
 *  -Updates the particles' locations in the octree which in turn checks particle-particle collisions
 *  -Writes all data to VBO
 */
void MyPanelOpenGL::drawFountain(){

  m_fountain->bind();

  //Updates all of the particles
  for(int i = 0; i < m_nparticles; i++){
      allParticles[i]->updatePosition(m_time);
      //allParticles[i]->VCollisionOff();
  }

  //Checks for wall-particle collisions
  intersect();

  //Updates particles' positions in octree which takes care of particle-particle collisions
  list->VUpdate();

  //Overwrites position information to the VBO
  for(int i = 0; i < m_nparticles; i++){
      data[3*i] = allParticles[i]->getPos().x();
      data[3*i+1] = allParticles[i]->getPos().y();
      data[3*i+2] = allParticles[i]->getPos().z();
  }

  m_fountain->write(0,data,m_nparticles*sizeof(vec3));

  m_shaderPrograms[m_curr_prog]->setUniformValue("vColor",vec4(0.52,0.81,0.98,1.));
  m_shaderPrograms[m_curr_prog]->setUniformValue("vSColor",vec4(1.,1.,1.,1.));
  m_shaderPrograms[m_curr_prog]->setUniformValue("Tex0",0);
  m_shaderPrograms[m_curr_prog]->enableAttributeArray("vPosition");
  m_shaderPrograms[m_curr_prog]->setAttributeBuffer("vPosition",GL_FLOAT,0,3,0);
  m_shaderPrograms[m_curr_prog]->enableAttributeArray("colors");
  m_shaderPrograms[m_curr_prog]->setAttributeBuffer("colors", GL_FLOAT,m_nparticles*sizeof(vec3), 4, 0);
  m_shaderPrograms[m_curr_prog]->setUniformValue("global_time",m_time);
  glDrawArrays(GL_POINTS, 0, m_nparticles);
  glFlush();

  m_fountain->release();
}

//Updates the camera angles
void MyPanelOpenGL::updateAngles(int idx, qreal amt){
  if(idx == 0){
    m_angles.setX(wrap(m_angles.x()+amt));
  }else if(idx == 1){
    m_angles.setY(wrap(m_angles.y()+amt));
  }else if(idx == 2){
    m_angles.setZ(wrap(m_angles.z()+amt));
  }
  updateModel();
}

//Rotates camera
void MyPanelOpenGL::updateModel(){
  m_model.setToIdentity();
  m_model.rotate(m_angles.z(), vec3(0,0,1.));
  m_model.rotate(m_angles.y(), vec3(0,1,0.));
  m_model.rotate(m_angles.x(), vec3(1,0,0.));
}

//Maps the input into the range of 0 to 360
qreal MyPanelOpenGL::wrap(qreal amt){
  if (amt > 360.){ return amt - 360.; }
  else if (amt < 0.){ return amt + 360.; }
  return amt;
}

//Updates the GLPlogonMode
void MyPanelOpenGL::updatePolyMode(int val){
  GLenum mode=GL_NONE;
  if(val==0){mode=GL_POINT;}
  else if(val==1){mode=GL_LINE;}
  else if(val==2){mode=GL_FILL;}

  if(mode != GL_NONE){
    glPolygonMode(GL_FRONT_AND_BACK, mode);
  }
  //glPolygonMode(GL_BACK,GL_POINT);
}

//Creates the vertex and fragment shaders whose names are passed in and stores them in the array of shaders
void MyPanelOpenGL::createShaders(int i, QString vertName, QString fragName){

  cout << "building shader " << i << endl;

  //Makes a new vertex shader
  m_vertexShaders[i] = new QOpenGLShader(QOpenGLShader::Vertex);
  if (!m_vertexShaders[i]->compileSourceFile(vertName)){
    qWarning() << m_vertexShaders[i]->log();
  }

  //Makes a new fragment shader
  m_fragmentShaders[i] = new QOpenGLShader(QOpenGLShader::Fragment);
  if(!m_fragmentShaders[i]->compileSourceFile(fragName)){
    qWarning() << m_fragmentShaders[i]->log();
  }

  m_shaderPrograms[i] = new QOpenGLShaderProgram();
  m_shaderPrograms[i]->addShader(m_vertexShaders[i]);
  m_shaderPrograms[i]->addShader(m_fragmentShaders[i]);

  if(!m_shaderPrograms[i]->link()){
    qWarning() << m_shaderPrograms[i]->log() << endl;
  }
}

//Destroys the shader and the index passed in
void MyPanelOpenGL::destroyShaders(int i){

  delete m_vertexShaders[i]; m_vertexShaders[i]=NULL;
  delete m_fragmentShaders[i]; m_fragmentShaders[i]=NULL;

  if(m_shaderPrograms[i]){
    m_shaderPrograms[i]->release();
    delete m_shaderPrograms[i]; m_shaderPrograms[i]=NULL;
  }
}

//Makes the cube
void MyPanelOpenGL::makeCube(){

    /* setup 8 corners of cube */
    vertices[0] = point4( -0.5, -0.5,  0.5, 1.0 );
    vertices[1] = point4( -0.5,  0.5,  0.5, 1.0 );
    vertices[2] = point4(  0.5,  0.5,  0.5, 1.0 );
    vertices[3] = point4(  0.5, -0.5,  0.5, 1.0 );
    vertices[4] = point4( -0.5, -0.5, -0.5, 1.0 );
    vertices[5] = point4( -0.5,  0.5, -0.5, 1.0 );
    vertices[6] = point4(  0.5,  0.5, -0.5, 1.0 );
    vertices[7] = point4(  0.5, -0.5, -0.5, 1.0 );

    /* assign one color to each corner */
    vertex_colors[0] = color4( 0.1, 0.1, 0.1, 1.0 );
    vertex_colors[1] = color4( 0.2, 0.2, 0.2, 1.0 );
    vertex_colors[2] = color4( 0.3, 0.3, 0.3, 1.0 );
    vertex_colors[3] = color4( 0.4, 0.4, 0.4, 1.0 );
    vertex_colors[4] = color4( 0.5, 0.5, 0.5, 1.0 );
    vertex_colors[5] = color4( 0.6, 0.6, 0.6, 1.0 );
    vertex_colors[6] = color4( 0.8, 0.8, 0.8, 1.0 );
    vertex_colors[7] = color4( 0.7, 0.7, 0.7, 1.0 );

    constructFaces();

    createVBO();
}

//Assigns colors and points that represent a single square side of the cube
void MyPanelOpenGL::quad(int a, int b, int c, int d){
    /* index is a static variable initialized once. same var (though different value)
     * used on all function calls. a 'mini global' variable */

    /* first triangle */
    colors[index] = vertex_colors[a]; points[index] = vertices[a]; index++;
    colors[index] = vertex_colors[a]; points[index] = vertices[b]; index++;
    colors[index] = vertex_colors[a]; points[index] = vertices[c]; index++;
    /* second triangle */
    colors[index] = vertex_colors[a]; points[index] = vertices[a]; index++;
    colors[index] = vertex_colors[a]; points[index] = vertices[c]; index++;
    colors[index] = vertex_colors[a]; points[index] = vertices[d]; index++;
}

//Constructs all six faces of the cube
void MyPanelOpenGL::constructFaces(){

    /* construct faces of cube */
    //front
    quad( 1, 0, 3, 2 );
    //right
    quad( 2, 3, 7, 6 );
    //bottom
    quad( 3, 0, 4, 7 );
    //top
    quad( 6, 5, 1, 2 );
    //back
    quad( 4, 5, 6, 7 );
    //left
    quad( 5, 4, 0, 1 );
}

/*Rotates the cube based on the index passed in:
 *  1 = x rotation
 *  2 = y rotation
 *  3 = z rotation
 */
void MyPanelOpenGL::rotateCube(int ind){

    //Rotates by a constant amount each time this function is called
    theta = (abs(ind)/ind) *vec3(M_PI/2,M_PI/2,M_PI/2);

    mat4 XRotation = mat4(1, 0, 0, 0,
                          0, cos(theta.x() * (M_PI/180.)), -sin(theta.x()* (M_PI/180.)), 0,
                          0, sin(theta.x()* (M_PI/180.)), cos(theta.x()* (M_PI/180.)), 0,
                          0, 0, 0, 1);
    mat4 YRotation = mat4(cos(theta.y()* (M_PI/180.)), 0, sin(theta.y()* (M_PI/180.)), 0,
                          0, 1, 0, 0,
                          -sin(theta.y()* (M_PI/180.)), 0, cos(theta.y()* (M_PI/180.)), 0,
                          0, 0, 0, 1);
    mat4 ZRotation = mat4(cos(theta.z()* (M_PI/180.)), -sin(theta.z()* (M_PI/180.)), 0, 0,
                          sin(theta.z()* (M_PI/180.)), cos(theta.z()* (M_PI/180.)), 0, 0,
                          0, 0, 1, 0,
                          0, 0, 0, 1);
    mat4 apply;

    //Assigns the correct rotation
    if(abs(ind) == 1){
      apply = XRotation;
    }
    else if(abs(ind) == 2){
      apply = YRotation;
    }
    else if(abs(ind) == 3){
      apply = ZRotation;
    }

    //Applies rotation to each vertex of the cube
    for(int i = 0; i < 8; i++){
        vertices[i] = vertices[i]*apply;
    }

    //Constructs a new cube
    index = 0;
    constructFaces();

    createVBO();

    //Applies all the rotation to the particles and has them respond accordingly
    for(int i =0; i < m_nparticles; i++){
        allParticles[i]->setPos(allParticles[i]->getPos() * apply);
        allParticles[i]->VCollisionOff();
        allParticles[i]->updatePosition(m_time);
    }

    //list->VUpdate();
    updateGL();
}

/*Performs wall-particle collision detection
 *For each particle and for each surface of the cube, checks if that particle is approaching and very close to the surface.
 *If so, the particle's velocity/direction is changed accordingly
 */
void MyPanelOpenGL::intersect(){

    //For every particle
     for(int i = 0; i < m_nparticles; i++){

       vec3 toExamine = allParticles[i]->getPos();
       vec3 velocity = allParticles[i]->getVel();


            //For every surface
            //bottom
            vec3 lr = vertices[3].toVector3D();
            vec3 ll = vertices[0].toVector3D();
            vec3 ur = vertices[7].toVector3D();

            //right
            vec3 lr1 = vertices[2].toVector3D();
            vec3 ll1 = vertices[3].toVector3D();
            vec3 ur1 = vertices[6].toVector3D();

            //left
            vec3 lr2 = vertices[5].toVector3D();
            vec3 ll2 = vertices[4].toVector3D();
            vec3 ur2 = vertices[1].toVector3D();

            //back
            vec3 lr3 = vertices[4].toVector3D();
            vec3 ll3 = vertices[5].toVector3D();
            vec3 ur3 = vertices[7].toVector3D();

            //front
            vec3 lr4 = vertices[1].toVector3D();
            vec3 ll4 = vertices[0].toVector3D();
            vec3 ur4 = vertices[2].toVector3D();

            //top
            vec3 lr5 = vertices[6].toVector3D();
            vec3 ll5 = vertices[5].toVector3D();
            vec3 ur5 = vertices[2].toVector3D();          

            vec3 norm;

            //If the particle is on any of the planes, its velocity and position are changed accordingly
            if(onPlane(ll,ur,lr,toExamine, velocity)){
                norm = normal(ll,ur,lr);
                changeVelocity(norm,i);
                changePosition(norm,i);
            }
            if(onPlane(ll1,ur1,lr1,toExamine, velocity)){
                norm = normal(ll1,ur1,lr1);
                changeVelocity(norm,i);
                changePosition(norm,i);
            }
            if(onPlane(ll2,ur2,lr2,toExamine, velocity)){
                norm = normal(ll2,ur2,lr2);
                changeVelocity(norm,i);
                changePosition(norm,i);
            }
            if(onPlane(ll3,ur3,lr3,toExamine, velocity)){
                norm = normal(ll3,ur3,lr3);
                changeVelocity(norm,i);
                changePosition(norm,i);
            }
            if(onPlane(ll4,ur4,lr4,toExamine, velocity)){
                norm = normal(ll4,ur4,lr4);
                changeVelocity(norm,i);
                changePosition(norm,i);
            }
            if(onPlane(ll5,ur5,lr5,toExamine, velocity)){
                norm = normal(ll5,ur5,lr5);
                changeVelocity(norm,i);
                changePosition(norm,i);
            }
       }
}

//Changes the position of the specified particle in the case that it has hit a surface
void MyPanelOpenGL::changePosition(const vec3& norm, int i){

    vec3 p;

    if(fabs(norm.x())>0){
        p = allParticles[i]->getPos();
        p.setX(allParticles[i]->getPrevPos().x());
        allParticles[i]->setPos(p);
    }
    if(norm.y()>0){
        p = allParticles[i]->getPos();
        p.setY(allParticles[i]->getPrevPos().y());
        allParticles[i]->setPos(p);
    }
    if(fabs(norm.z())>0){
        p = allParticles[i]->getPos();
        p.setZ(allParticles[i]->getPrevPos().z());
        allParticles[i]->setPos(p);
    }
}

//Changes the velocity of the specified particle in the case that it has hit a surface
void MyPanelOpenGL::changeVelocity(const vec3& norm, int i){

    allParticles[i]->setVel(allParticles[i]->getVel() - 2*-norm*QVector3D::dotProduct(-norm, allParticles[i]->getVel()));
    allParticles[i]->setVel(allParticles[i]->getVel() * allParticles[i]->getFriction());
}

//Returns true if the passed in position (check) is on the plane constructed by the three points and if the the velocity is moving towards the plane
//Returns false otherwise
bool MyPanelOpenGL::onPlane(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& check, const vec3& vel){

    vec3 dir = -normal(p1,p2,p3);
    return QVector3D::dotProduct(dir,check) + allParticles[0]->VGetRadius() > 0.5 && QVector3D::dotProduct(vel, dir) > 0;
}

//Computes and returns the normal to the rectangle
vec3 MyPanelOpenGL::normal(vec3 ll, vec3 ur, vec3 lr){

    vec3 vector1 = -ll+lr;
    vec3 vector2 = ur - lr;
    vec3 toReturn = QVector3D::crossProduct(vector1, vector2);
    toReturn.normalize();
    return toReturn;
}

//Creates the VBO
void MyPanelOpenGL::createVBO(){

    destroyVBO(); //get rid of any old buffers

    vboData = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vboData->create();
    vboData->bind();
    vboData->setUsagePattern(QOpenGLBuffer::StaticDraw);

    vboData->allocate(numVertices*(sizeof(point4)+sizeof(color4)) + m_nparticles*(sizeof(color4)));
    vboData->write(0,points,numVertices*sizeof(point4));
    vboData->write(numVertices*sizeof(point4),colors,numVertices*sizeof(color4));
}

//Destroys the VBO
void MyPanelOpenGL::destroyVBO(){
    if (vboData){
        vboData->release();
        delete vboData; vboData=NULL;
    }
}
