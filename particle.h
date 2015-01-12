/*Zoe Junghans and Jess Karol
 *Description:  This particle class defines the behavior of a single particle to act like water.
 * The particle's position and velocity are maintained and changed according to time, gravity, collisions,
 * and other factors.
 */

#ifndef CS40_PARTICLE_H
#define CS40_PARTICLE_H

#include "ISpatialObject.h"
#include <QtOpenGL>
#include "Vector3.h"

typedef QVector2D vec2;
typedef QVector3D vec3;
typedef QVector4D vec4;

using namespace SpatialTest;

namespace cs40{

  class Particle : public SpatialTest::ISpatialObject{

    public:
     Particle(vec3 pos, vec3 vel);
     ~Particle(){ /*do nothing*/};

     void updatePosition(float time);

     /* gets and sets position of particle */
     vec3 getPos();
     void setPos(vec3 p);

     /* gets and sets velocity of particle */
     vec3 getVel();
     void setVel(vec3 v);

     /* gets and sets previous position of particle */
     vec3 getPrevPos();
     void setPrevPos(vec3 pp);

     /* gets and sets friction component of particle */
     float getFriction();
     void setFriction(float f);

     /* gets and sets start time of particle */
     float getStartTime();
     void setStartTime(float st);


     /*Methods inherited from ISpatialObject abstract class: */

     /*Returns one if the current particle has collided with the passed in object, zero otherwise
      *In the case of a collision, both colliding particles' velocities are changed accordingly
      */
     int VCheckCollision(ISpatialObject* object, bool compute) ;

     /*Returns true if there has been a collision between the two passed in objects, false otherwise
      */
     bool testParticleCollision(ISpatialObject* curr, ISpatialObject* obj);

     /* sets the collision boolean to true or false respectively */
     void VCollisionOn();
     void VCollisionOff();

     /* returns true if the particle is currently in collision with another particle, false otherwise */
     int VGetCollisionStatus() const;

     /* gets and sets the Vector3 version of the position of the particle */
     void VSetPosition(const Vector3 &refPosition);
     const Vector3& VGetPosition() const;

     /* gets and sets the Vector3 version of the velocity of the particle */
     void VSetDirection(const Vector3 &refDirection);
     const Vector3& VGetDirection() const;

     /* gets and sets the radius of the particle */
     void VSetRadius(float f32Radius);
     float VGetRadius() const;

     /* gets and sets the current ISpatialCell the particle is located in */
     void VSetCell(ISpatialCell* pCell);
     ISpatialCell* VGetCell();

     /* gets and sets the next particle in a linked list of ISpatialObjects */
     ISpatialObject* VGetNext();
     void VSetNext(ISpatialObject* pObject);

     /* gets and sets the level of the octree the particle is located in */
     void VSetLevel(int i32Level);
     int VGetLevel() const;

     /* applies a pressure feature of water behavior to the objects by adjusting their velocites
      * depending on how many other particles they are in collision with */
     void VApplyPressure(int weight, ISpatialObject* obj);

    protected:

     float startTime;
     float friction;
     vec3 previousPos;
     vec3 vel;
     vec3 pos;

     float m_radius;
     SpatialTest::ISpatialCell* m_spatialCell;
     int m_collision;
     int level;
     float dt;
     float m_time;

     SpatialTest::Vector3 m_position;
     SpatialTest::Vector3 m_direction;

     SpatialTest::ISpatialObject* next;

  };
}
#endif // PARTICLE_H
