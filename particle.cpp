/*Zoe Junghans and Jess Karol
 *Description:  This particle class implements the behavior of a single particle to act like water.
 * The particle's position and velocity are maintained and changed according to time, gravity, collisions,
 * and other factors.
 */

#include "particle.h"
#include <iostream>
#include <math.h>
#include <math_constants.h>

using namespace cs40;

//Constructor
Particle::Particle(vec3 p, vec3 v){
    pos = p;
    vel = v;
    dt = .0005;
    m_collision = 0;
    m_radius = 0;
    level = 0;
    m_position = Vector3(pos.x(), pos.y(), pos.z());
    previousPos = vec3(0,0,0);
    m_direction = Vector3(vel.x(), vel.y(), vel.z());
    friction = 0.8;
    startTime = 0;
}

/* Updates the partice's position
 * Particle's position is changed by its velocity times a time step, taking into consideration
 * the force of gravity.  Takes in a time parameter for an error check.
 */
void Particle::updatePosition(float global_time){

    m_time = global_time;
    vec3 gravity=vec3(0,-1.2,0);

    if(global_time > startTime){

      //Does not apply gravity if the particle is in constant collision with others.  This
      //creates a stacking effect.
      if(!VGetCollisionStatus()){
         vel += 0.4*gravity;
      }

      previousPos = pos;
      pos = pos+vel*dt;
    }

    VSetPosition(Vector3(pos.x(),pos.y(),pos.z()));
    VSetDirection(Vector3(vel.x(),vel.y(),vel.z()));
}

//Returns the position of the particle
vec3 Particle::getPos(){
    return pos;
}

//Sets the position of the particle
void Particle::setPos(vec3 p){
    pos = p;
    VSetPosition(Vector3(pos.x(),pos.y(),pos.z()));
}

//Returns the velocity of the particle
vec3 Particle::getVel(){
    return vel;
}

//Sets the velocity of the particle
void Particle::setVel(vec3 v){
    vel = v;
    VSetDirection(Vector3(vel.x(),vel.y(),vel.z()));
}

//Returns the previous position of the particle
vec3 Particle::getPrevPos(){
    return previousPos;
}

//Sets the previous position of the particle
void Particle::setPrevPos(vec3 pp){
    previousPos = pp;
}

//Returns the friction component
float Particle::getFriction(){
    return friction;
}

//Sets the friction component
void Particle::setFriction(float f){
    friction = f;
}

//Returns the start time of the particle
float Particle::getStartTime(){
    return startTime;
}

//Sets the start time of the particle
void Particle::setStartTime(float st){
    startTime = st;
}

/*Methods inherited from the ISpatialObject abstract class:*/

/*Returns one if the current particle has collided with the passed in object, zero otherwise
 *In the case of a collision, both colliding particles' velocities are changed accordingly
 */
int Particle::VCheckCollision(ISpatialObject* obj, bool compute) {

    //Checks if there is a collision
    if(testParticleCollision(this, obj)){

        if(compute){
            Vector3 sub = this->VGetPosition() - obj->VGetPosition();
            Vector3 displacement = sub*(1/sub.Length());

            //Applies viscosity
            this->VSetDirection(this->VGetDirection() + dt*100*(obj->VGetDirection()-this->VGetDirection()));
            obj->VSetDirection(obj->VGetDirection() + dt*100*(this->VGetDirection()-obj->VGetDirection()));

            //Applies equal and opposite reaction that conserves energy and momentum
            this->VSetDirection(friction*(this->VGetDirection() - (2*displacement*this->VGetDirection().Dot(displacement))));
            obj->VSetDirection(friction*(obj->VGetDirection() - (2*displacement*obj->VGetDirection().Dot(displacement))));

        }

        return 1;
    }

    return 0;
}

//Applies a pressure component to the particles based on how many other particles they are colliding with
void Particle::VApplyPressure(int weight, ISpatialObject* obj){

    float pressure = fmax(0, 0.5*(weight));

    Vector3 sub = this->VGetPosition() - obj->VGetPosition();
    Vector3 displacement = sub*(1/sub.Length());

    //Changes velocities accordingly
    this->VSetDirection(this->VGetDirection() + (pressure*2*weight*displacement));
    this->VSetDirection(this->VGetDirection()*friction);

}

//Returns true if the current particle is in close proximity and moving towards the other particle, false otherwise
bool Particle::testParticleCollision(ISpatialObject* curr, ISpatialObject* obj){

    //Does not check collisions until the particles are in the cube
    if(curr->VGetPosition().y > 0.5){
        return false;
    }

    if(m_time > 0.1 && pow((VGetPosition()-obj->VGetPosition()).Length(),2) < 4*m_radius*m_radius){

        Vector3 netV = curr->VGetDirection() - obj->VGetDirection();
        Vector3 displacement = curr->VGetPosition() - obj->VGetPosition();

        return netV.Dot(displacement) < 0;
    }

    //No collision
    else{
        return false;
    }
}

//Turns the collision status on
void Particle::VCollisionOn(){
    m_collision = 1;
}

//Turns the collision status off
void Particle::VCollisionOff(){
    m_collision = 0;
}

//Returns the collision status
int Particle::VGetCollisionStatus() const{
    return m_collision;
}

//Sets the Vector3 version of the particle's position
void  Particle::VSetPosition(const Vector3 &refPosition){
    m_position = refPosition;
    pos = vec3(refPosition.x,refPosition.y,refPosition.z);
}

//Returns the Vector3 version of the particle's position
const Vector3& Particle::VGetPosition() const{
    Vector3 *v = new Vector3(pos.x(), pos.y(), pos.z());
    return *v;
}

//Sets the Vector3 version of the particle's velocity
void  Particle::VSetDirection(const Vector3 &refDirection){
    m_direction = refDirection;
    vel = vec3(refDirection.x,refDirection.y,refDirection.z);
}

//Returns the Vector3 version of the particle's velocity
const Vector3& Particle::VGetDirection() const{
    Vector3 *v = new Vector3(vel.x(), vel.y(), vel.z());
    return *v;
}

//Sets the particle's radius
void Particle::VSetRadius(float f32Radius){
    m_radius = f32Radius;
}

//Returns the particle's radius
float Particle::VGetRadius() const{
    return m_radius;
}

//Sets the current cell the particle is contained in
void  Particle::VSetCell(ISpatialCell* pCell){
    m_spatialCell = pCell;
}

//Returns the current cell the particle is contained in
ISpatialCell*  Particle::VGetCell(){
    return m_spatialCell;
}

//Returns the next particle in the linked list of particles
ISpatialObject*  Particle::VGetNext(){
    return next;
}

//Sets the next particle in the linked list of particles
void  Particle::VSetNext(ISpatialObject* pObject){
    next = pObject;
}

//Sets the current level of the octree that the particle is contained in
void  Particle::VSetLevel(int i32Level){
    level = i32Level;
}

//Returns the current level of the octree that the particle is contained in
int  Particle::VGetLevel() const{
    return level;
}

