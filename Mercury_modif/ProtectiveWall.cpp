//Copyright (c) 2013-2020, The MercuryDPM Developers Team. All rights reserved.
//For the list of developers, see <http://www.MercuryDPM.org/Team>.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name MercuryDPM nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED. IN NO EVENT SHALL THE MERCURYDPM DEVELOPERS TEAM BE LIABLE FOR ANY
//DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/* Particles are released from a specific height, roll through a slope and then a protective wall retains them. This is an interactive tutorial which the uset can modify the input parameters. For full documentation of this code, go to http://docs.mercurydpm.org/
*/

//! [AT_PW:headers]
#include <Mercury3D.h>
#include "Species/LinearViscoelasticSlidingFrictionSpecies.h"
#include "Walls/InfiniteWall.h"
#include "Walls/IntersectionOfWalls.h"
#include "Boundaries/CubeInsertionBoundary.h"
#include "Boundaries/CubeDeletionBoundary.h"
#include "Boundaries/PeriodicBoundary.h"
//! [AT_PW:headers]

//! [AT_PW:MainClass]
class protectiveWall : public Mercury3D
{
public:

    //! [AT_PW:Constructor]
    protectiveWall(int Nump, Mdouble pRadius, Mdouble height, Mdouble width,Mdouble Length, Mdouble slopeAngle)
    {
        //! [AT_PW:globalConditions]
        setNumParticles = Nump; //Number of particles
        setGlobalRadius = pRadius; //Particle radius
        //setParticleHeight = height; //Height of release
        setWallHeight = height ; 
        setSlopeAngle = constants::pi / 180.0 * slopeAngle; //Slope angle

        setName("protectiveWall"); //Output file name
        setWallsWriteVTK(FileType::ONE_FILE); //For visualization
        setParticlesWriteVTK(true); //For visualization
        setInteractionsWriteVTK(true);
        setGravity(Vec3D(0.0, 0.0, -9.81)); //Set gravity

        Mdouble Hipo = Length/cos(setSlopeAngle); //Slope length
        setXMax(Length); //Boundary length
        setYMax(width); //Boundary width
        
        Mdouble  Zmaxslope = Hipo*sin(setSlopeAngle) ;
        
        setZMax(Zmaxslope+20); //Boundary height using slope length

        
        //! [AT_PW:globalConditions]

        //! [AT_PW:particleSpecies]//Particles have their own species
        particleSpecies = speciesHandler.copyAndAddObject(LinearViscoelasticSlidingFrictionSpecies());
        particleSpecies->setDensity(2500.0); //set the species density
        particleSpecies->setStiffness(150000);//set the spring stiffness.
        particleSpecies->setDissipation(100); //set the dissipation.
        particleSpecies->setSlidingFrictionCoefficient(0.5) ;
        //! [AT_PW:particleSpecies]

        //! [AT_PW:particles] //To insert the particles
        insb = new CubeInsertionBoundary;
        SphericalParticle particles;
        particles.setSpecies(particleSpecies);

        insb->set(
                &particles,
                1,
                Vec3D(0.9*getXMax(),  getYMin(), Zmaxslope ), //Minimum position
                Vec3D(getXMax(), getYMax(), Zmaxslope + 2), //Maximum position
                Vec3D(0, 0, 0), //Minimum velocity
                Vec3D(0, 0, 0), //Maximum velocity
                pRadius*0.98, //Minimum particle radius
                pRadius*1.1 //Maximum particle radius
        );
        insb = boundaryHandler.copyAndAddObject(insb);
        //! [AT_PW:particles]

        // Periodic in y
        PeriodicBoundary b;
        b.set(Vec3D(0.0, 1.0, 0.0), getYMin(), getYMax());
        boundaryHandler.copyAndAddObject(b);
        
        //To delete particles
        delb = new DeletionBoundary;
        delb->set(Vec3D(-1,0,0), getXMin());
        delb =  boundaryHandler.copyAndAddObject(delb);
        
    }
    //! [AT_PW:Constructor]

    //! [AT_PW:MemberFunctions]

    void setupInitialConditions() override
    {
        //! [AT_PW:walls]
        InfiniteWall slope,lateralwall1, lateralwall2, lateralwall3;

        //! [AT_PW:slope] //Defining slope
        slope.setSpecies(particleSpecies);
        slope.setPosition(Vec3D(getXMin(),getYMin(),getZMin()));
        slope.setOrientation(Vec3D(sin(setSlopeAngle),0,-cos(setSlopeAngle)));
        wallHandler.copyAndAddObject(slope);
        
        
        //! [AT_PW:slope]

        //! [AT_PW:lateralWall] //Defining walls
        /*lateralwall1.setSpecies(particleSpecies);
        lateralwall1.set(Vec3D(0.0, 1.0, 0.0), Vec3D(0.0,getYMax()+10,0.0));
        wallHandler.copyAndAddObject(lateralwall1);

        lateralwall2.setSpecies(particleSpecies);
        lateralwall2.set(Vec3D(0.0, -1.0, 0.0), Vec3D(0.0,getYMin()-10,0.0));
        wallHandler.copyAndAddObject(lateralwall2);
        */
        IntersectionOfWalls roarWall, proctWall;
        //Rear wall.
        roarWall.setSpecies(particleSpecies);
        roarWall.addObject(Vec3D(1.0, 0.0, 0.0), Vec3D(getXMax(), 0.0, 0.0));
        roarWall.addObject(Vec3D(0.0, 0.0, -1.0), Vec3D(getXMax(), 0.0, getZMax()));
        wallHandler.copyAndAddObject(roarWall);

        //! [AT_PW:lateralWall]

        //! [AT_PW:protectiveWall]
        //Protective wall
        //IntersectionOfWalls proctWall ; 
        heightProtWall = setWallHeight;
        proctWall.setSpecies(particleSpecies);
        proctWall.addObject(Vec3D(-1.0, 0.0, 0.0), Vec3D(getXMin(), 0.0, 0.0));
        proctWall.addObject(Vec3D(0.0, 0.0, -1.0), Vec3D(getXMin(), 0.0, heightProtWall));
        wallHandler.copyAndAddObject(proctWall);
        //! [AT_PW:protectiveWall]

        //! [AT_PW:walls]
    }

    void actionsAfterTimeStep() override {

        vol_inserted = insb->getVolumeOfParticlesInserted();
        partDel = delb->getNumberOfParticlesDeleted();

        //Specific number of particles
        setVolume = 4.0/3.0 * constants::pi * (setGlobalRadius*setGlobalRadius*setGlobalRadius);
        setVolume *= setNumParticles;

        //Stop inserting particles
        if(vol_inserted  >= setVolume && !removed_insb){
            boundaryHandler.removeObject(insb->getIndex());
            removed_insb = true;
            setVolume = vol_inserted;
        }

        //Wall force and pressure
        wallForceX = fabs(wallHandler.getObject(2)->getForce().X);
        wallForceY = fabs(wallHandler.getObject(2)->getForce().Y);
        wallForceZ = fabs(wallHandler.getObject(2)->getForce().Z);
        wallPressure = wallForceX / (getYMax()*heightProtWall);
    }

    //Criterion to stop the simulation, otherwise the simulation stops at maxTime.
    bool continueSolve() const override
    {
        static unsigned int counter = 0;
        if (++counter>100)
        {
            counter=0;
            if (getKineticEnergy()<0.001*getElasticEnergy())
                return false;
        }
        return true;
    }

    //Print varaibles in the console/terminal
    void printTime()const override
    {
        std::cout << "t=" << std::setprecision(3) << std::left<< getTime()
                  << ", tmax=" << std::setprecision(3) << std::left<< getTimeMax()
                  << ", # Particles inserted=" << std::setprecision(3) << std::left << setNumParticles - partDel
                  << ", # Particles deleted=" << std::setprecision(3) << std::left << partDel
                  << ", Volume inserted=" << std::setprecision(3) << std::left << std::setw(6)<< vol_inserted
                  << ", WallForce=" << std::setprecision(3) << std::left << std::setw(6) << wallForceX
                  << ", WallPressure=" << std::setprecision(3) << std::left << std::setw(6) << wallPressure
                  << std::endl;
    }

    //Write variables in the fstat file
    void writeFstatHeader(std::ostream &os) const override {
        os<< getTime() //Current time
          << " " << getTimeMax() //MaxTime
          << " " << setNumParticles - partDel //Particles inserted
          << " " << partDel //Partilcles deleted
          << " " << wallForceX //Wall force
          << " " << wallForceY //Wall force
          << " " << wallForceZ //Wall force
          << std::endl;
    }

    //! [AT_PW:MemberFunctions]

    //! [AT_PW:PrivateVariables]
private:
    Mdouble setGlobalRadius = 1.0; //By default
    Mdouble setWallHeight = 1.; //By default
    Mdouble setSlopeAngle = 20.0; //By default
    Mdouble heightProtWall = 1.0; //By default

    CubeInsertionBoundary* insb;
    DeletionBoundary* delb;
    int partDel = 0;
    int setNumParticles;
    Mdouble wallForceX = 0.0;
    Mdouble wallForceY = 0.0;
    Mdouble wallForceZ = 0.0;
    Mdouble wallPressure = 0.0;
    Mdouble vol_inserted = 0.0;
    Mdouble setVolume = 0.0;
    bool removed_insb = false;

    LinearViscoelasticSlidingFrictionSpecies* slopeSpecies;
    LinearViscoelasticSlidingFrictionSpecies* particleSpecies;
    //! [AT_PW:PrivateVariables]
};

//! [AT_PW:MainClass]

//! [AT_PW:Mainfunction]
int main(int argc, char* argv[])
{
    //Helper
    std::cout << "Write in the terminal after the compilation'./protectiveWall -Np 500 -r 0.01 -h 0.1 -w 0.25 -l 1.0 -s 15.0 -t 20.0' to run the program" << std::endl;

    //! [AT_PW:setUp]
    int Nump = helpers::readFromCommandLine(argc,argv,"-Np",500); //50 particles
    Mdouble pRadius = helpers::readFromCommandLine(argc,argv,"-r",0.15); //0.01 [m]
    Mdouble height = helpers::readFromCommandLine(argc,argv,"-h",1.5); //0.1 [m]
    Mdouble width = helpers::readFromCommandLine(argc,argv,"-w",1.);  //0.25 [m]
    Mdouble length = helpers::readFromCommandLine(argc,argv,"-l",10.); //1.0 [m]
    Mdouble slopeAngle = helpers::readFromCommandLine(argc,argv,"-s",10.0); //15 grades

    printf("%d %g %g %g %g %g \n", Nump, pRadius, height, width, length, slopeAngle) ; fflush(stdout);
    protectiveWall problem(Nump,pRadius,height,width,length,slopeAngle); //Object

    Mdouble simTime = helpers::readFromCommandLine(argc,argv,"-t",5.0); // 5.0 [s]
    problem.setTimeMax(simTime);
    //! [AT_PW:setUp]
    problem.setSaveCount(1000);
    problem.setTimeStep(0.00001); // (collision time)/50.0
    problem.removeOldFiles();
    problem.solve();
    return 0;
}
//! [AT_PW:Mainfunction]
