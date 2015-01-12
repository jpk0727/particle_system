/*Zoe Junghans and Jess Karol
 *CS40 Final Project
 *Description: This project models a particle system which begins to approximate the movement of water.
 *The particle system allows for particle-particle collisions as well as particle-wall collisions with the static environment (a cube) we created.
 *It incorporates the use of an octree which allows for large performance benefits and the ability to introduce a much larger number of particles into the system.
 *Finally, the particles have been made to respond correctly to transformations applied to the static environment.
 */

#ifndef MYPANELOPENGL_H
#define MYPANELOPENGL_H

#include <QtOpenGL>
#include <QKeyEvent>
#include <QGLWidget>
#include <QMatrix4x4>
#include <QTimer>
#include "common/sphere.h"
#include "particle.h"
#include "octree.h"

typedef QVector4D point4;
typedef QVector4D color4;
typedef QVector3D vec3;
typedef QMatrix4x4 mat4;

#define CS40_NUM_PROGS 2
#define numparts 2000  //This system can hold a maximum of two thousand particles

class MyPanelOpenGL : public QGLWidget
{
    Q_OBJECT

protected:

    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    void keyPressEvent(QKeyEvent* event);

public:
    explicit MyPanelOpenGL(QWidget *parent = 0);
    virtual ~MyPanelOpenGL();

private:

    /* simple test shapes */
    cs40::Sphere* m_sphere;
    int m_sphereSize;

    /* particle fountain VBO */
    QOpenGLBuffer* m_fountain;

    /* information about the number of particles */
    int m_nparticles;
    int orignum;

    /* time variable */
    QTimer* m_timer;
    float m_time;

    GLuint m_texture;

    cs40::Particle* allParticles[numparts]; /* list of all particle objects */
    Octree* list; /* octree that contains all of the particles */

    int m_polyMode;

    vec3 m_angles; /* Euler angles for rotation */

    /* camera matricies */
    mat4 m_model;
    mat4 model;
    mat4 m_camera;
    mat4 m_projection;

    /* cube information */
    unsigned int numVertices;
    point4* points; /* for vbo */
    color4* colors; /* for vbo */
    point4 *vertices; /* unique cube vertices */
    color4 *vertex_colors; /* unique cube colors */
    int index;
    vec3 theta; /* for cube rotations */

    float* partColors; /* for vbo */
    float* data; /* for VBO */

    /* GPU data for shaders */
    QOpenGLBuffer *vboData;

    /* Shaders and programs */
    QOpenGLShader *m_vertexShaders[CS40_NUM_PROGS];
    QOpenGLShader *m_fragmentShaders[CS40_NUM_PROGS];
    QOpenGLShaderProgram *m_shaderPrograms[CS40_NUM_PROGS];

    int m_curr_prog; /*current shader program ID */

    /* Build a model of a unit cube centered at the origin*/
    void makeCube();

    /* Construct all six faces of the cube */
    void constructFaces();

    /* Construct geometry/colors for one face of cube,
     * given vertex indices in CCW order */
    void quad(int a, int b, int c, int d );

    /* Rotates the cube based on index passed in
     * 1 = X rotation
     * 2 = Y rotation
     * 3 = Z rotation
     */
    void rotateCube(int ind);

    /* VBO */
    void createVBO();
    void destroyVBO();

    /* computes a random float between 0 and 1 */
    float randFloat();

    /* makes a particle system that initializes all of the particles to fall in a cube formation into the static scene
     * the index parameter signifies which index the particles should start at in the allParticles array since this method
     * is also called when the "r" key is pressed and more particles are added to the system */
    void makeFountainSmall(int index);

    /* makes a particle system that initializes all of the particles to fall from a single source at a certain time offset */
    void makeFountainLarge();

    /* draws the particle system */
    void drawFountain();

    /* updtaes the time by a constant time step */
    void updateTime();

    /* Initializes the octree by passing in all of the particles' positions */
    void makeOctree();

    /* update Euler angle at index idx by amt
     * idx=0,1,2 -> x,y,z */
    void updateAngles(int idx, qreal amt);

    /* update model matrix based on angle */
    void updateModel();

    /* wrap a angle value to the range 0..360*/
    qreal wrap(qreal amt);

    /*Performs wall-particle collision detection
     *For each particle and for each surface of the cube, checks if that particle is approaching and very close to the surface.
     *If so, the particle's velocity/direction is changed accordingly
     */
    void intersect();

    /* Returns the normal vector to the plane constructed by the three points passed in as parameters */
    vec3 normal(vec3 ll, vec3 ur, vec3 lr);

    /* Returns true if "check" vector is within a certain distance to the plane constructed by the three points passed in
     * and if the "vel" velocity is moving towards the plane
     */
    bool onPlane(const vec3& p1, const vec3& p2, const vec3& p3, const vec3& check, const vec3& vel);

    /* Changes the ith particle's velocity in the case where it has hit a plane */
    void changeVelocity(const vec3& norm, int i);

    /* Changes the ith particle's position in the case where it has hit a plane */
    void changePosition(const vec3& norm, int i);

    /* update Polygon draw mode based
     * 0 : point
     * 1 : line
     * 2 : polygon */
    void updatePolyMode(int val);

    /* For shaders */
    void createShaders(int i, QString vertName, QString fragName);
    void destroyShaders(int i);

    /* paints the particles' scene */
    void paintParticles();

    /* paints the scene involving the cube */
    void paintStatic();

    /*Resets the scene information */
    void reset();

signals:
    
public slots:

    void step();

    /* Called when the "Small" radio button on the GUI is pushed.
     * Makes a new scene with small particles falling in a cube formation.
     */
    void paintSmall();

    /*Called when the "Large" radio button on the GUI is pushed
     *Makes a new scene with large particles falling from a single source with a time delay
     */
    void paintLarge();

};

#endif // MYPANELOPENGL_H
