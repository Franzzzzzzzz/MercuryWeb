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
#define __STDC_WANT_LIB_EXT1__ 1
#include <cstdlib>
#include <cstdio>
#include <string>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <atomic>
#include <pugixml.hpp>
#include <iostream>
#include <sstream>
#include "Server.h"

#define MAXPROC 4

struct Userspace {
    enum Policy {admin=0, student=1, none=2} ; 
    enum ProcessStatus {stopped=0, running=1} ; 
    static const std::map <Policy, int> maxTime ;
    Policy userpolicy = none ; 
    ProcessStatus processstatus = stopped ; 
    std::chrono::steady_clock::time_point starttime ; 
    double percentcompleted=0.0 ; 
    bool killsignal = false ;
} ; 
const std::map <Userspace::Policy, int> Userspace::maxTime = {{Userspace::admin, 86400}, {Userspace::student, 10}, {Userspace::none, 0}} ; 
std::atomic<int> running = 0 ; 


template <class T>  T sanitize (const std::map <std::string, std::string> & params, const std::string p, const T min, const T max, const T defaut)
{
    std::string valuestr; 
    try {
        valuestr=params.at(p) ;
    }
    catch (const std::out_of_range& oor) {
        return (defaut) ; 
    }
    T value =static_cast<T>(std::stod(valuestr)) ; 
    if (value<min) return min ; 
    if (value>max) return max ; 
    return value ;
}


//! [AT_PW:Mainfunction]
int main(int argc, char* argv[])
{
    //Helper
    // std::cout << "Write in the terminal after the compilation'./protectiveWall -Np 500 -r 0.01 -h 0.1 -w 0.25 -l 1.0 -s 15.0 -t 20.0' to run the program" << std::endl;
    
    using namespace httplib;
    std::map <std::string, Userspace> user ;
    Server svr;
    
    // Get userlist
    FILE * userfile ; 
    userfile = fopen("../userlist.txt", "r") ; 
    if (userfile == NULL) 
    {
        printf("Userfile not found. You will not be able to run anything\n") ;
    }
    else
    {
        char username[255] ; int p ;
        while (!feof(userfile))
        {
            fscanf(userfile, "%s %d\n", username, &p);
            user[username] ; // Using the operator[] to fill up the user with default constructors
            user[username].userpolicy = static_cast<Userspace::Policy>(p) ; 
        }
        
    }
    
    // Setup server
    svr.set_base_dir(".");
    
    svr.Get(R"(/run)", [&](const Request& req, Response& res) {
        try 
        {
            std::string username=req.params.at("username") ;
            auto policy = user.at(username).userpolicy ;
            switch (policy) {
                case Userspace::admin : // Falling down
                case Userspace::student : break ; 
                default: res.status=403 ; // Forbiden ; 
                         return ; 
            }
            
            if (user[username].processstatus == Userspace::running)
            {
                printf("[%s] Process already running. Discarding the request.\n", username.c_str()) ; 
                res.status=409 ;
                return ; 
            }
            
            if (running >= MAXPROC)
            {
                printf("[%s] Cannot run new simulations, already %d running.\n", username.c_str(), MAXPROC) ; 
                res.status=503 ;
                return ; 
            }

            std::filesystem::create_directory(username.c_str()) ;
            
            int Nump=sanitize<int>(req.params, "Np", 1,1000, 50) ; 
            double pRadius = sanitize<double>(req.params,"r", 0.01,0.5, 0.2) ; //0.01 [m]
            double height  = sanitize<double>(req.params,"h", 0.05, 5 , 1.); //0.1 [m]
            double width   = sanitize<double>(req.params,"w", 0.5, 20, 1.);  //0.25 [m]
            double length  = sanitize<double>(req.params,"l", 1, 20, 10.); //1.0 [m]
            double slopeAngle = sanitize<double>(req.params,"s", 0, 50, 10.0); //15 grades
            double simTime = sanitize<double>(req.params, "t", 1, 1000., 10.); //15 grades
            
            char commande[1001] ; 
            snprintf(commande,1000, "cd %s ; ../../Software/ProtectiveWall -Np %d -r %g -h %g -w %g -l %g -s %g -t %g", username.c_str(), Nump, pRadius, height, width, length, slopeAngle, simTime);
            
            printf("%s\n", commande) ; 
            
            //------------------------
            FILE * saveinfo ; std::string infopath = username ; 
            infopath += "/LastRun.txt" ; 
            saveinfo = fopen(infopath.c_str(), "w"); 
            if (saveinfo != NULL) 
            {
                fprintf(saveinfo, "%d %g %g %g %g %g %g\n", Nump, pRadius, height, width, length, slopeAngle, simTime) ;
                fclose(saveinfo) ; 
            }
            //=========================
            FILE *fp; int status;
            char line[10000] ; 
            user[username].processstatus = Userspace::running ; 
            user[username].starttime = std::chrono::steady_clock::now();
            user[username].killsignal = false ; 
            running++ ; 
            fp = popen (commande, "r") ; 
            if (fp==NULL) {printf("Error: cannot open pipe\n") ; throw ; }
            float tcur=0, tmax=simTime ; 
            while (!feof(fp) && user[username].killsignal==false)
            {
                fscanf(fp, "%[^\n]%*c", line) ; 
                sscanf(line, "t=%g, tmax=%g", &tcur, &tmax) ; 
                user[username].percentcompleted = tcur/tmax*100. ; 
                auto runningtime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-user[username].starttime).count() ;
                printf("[%s] %lds %g\n",username.c_str(), runningtime, user[username].percentcompleted) ; 
                if (runningtime > Userspace::maxTime.at(user[username].userpolicy)) 
                {
                    printf("[%s] Force stopped after authorised time elapsed.\n", username.c_str()) ; 
                    res.status = 408 ; 
                    break ; 
                }
            }
            if (user[username].killsignal==true) printf("[%s] the simulation was killed remotely. %d .\n", username.c_str(), static_cast<int>(user[username].processstatus)) ; 
            status = pclose(fp);
            user[username].processstatus = Userspace::stopped ; 
            running -- ; 
            if (running<0) running=0 ; // Should never happen, but just to be on the safe side
            
            saveinfo = fopen(infopath.c_str(), "a");
            if (saveinfo != NULL) 
            {
                fprintf(saveinfo, "%g %g\n", tcur, tmax) ;
                fclose(saveinfo) ; 
            }
            
        }
        catch (const std::out_of_range& e)
        {
            printf("User not found\n") ; 
            res.status=403 ; 
        }
        catch (...)
        {
         printf("Some error occured") ; fflush(stdout) ; 
         res.status=400 ; 
        }
    }); // End /run
    
    svr.Get(R"(/kill)", [&](const Request& req, Response& res) {
        try 
        {
            std::string username=req.params.at("username") ;
            user[username].killsignal=true ;
        }
        catch(...) {} ; 
    }) ; 
    
    svr.Get(R"(/status)", [&](const Request& req, Response& res) {
        try 
        {
            std::string username=req.params.at("username") ;
            std::string response="" ; 
            response = "{\"username\":\""+username+"\",\"runstate\":"+std::to_string(static_cast<int>(user[username].processstatus))+",\"completion\":"+std::to_string(user[username].percentcompleted)+"}" ; 
            
            res.set_content(response.c_str(), response.size(), "application/json");
        }
        catch(...) {}
        
    });
    
    
    svr.Get(R"(/downloadwall)", [&](const Request& req, Response& res) {
        try 
        {
            std::string username=req.params.at("username") ;
            FILE * in, *out ; std::string input, output ;
            char letter ; 
            input = username + "/protectiveWall.fstat" ;
            output = username + "/ProtectiveWallStat.txt" ;
            in=fopen(input.c_str(), "r") ; 
            out=fopen(output.c_str(), "w") ; 
            if (in==NULL || out==NULL)
            {
                res.status=404 ; 
                return ; 
            }
            fprintf(out, "Time MaxTime NParticles NDeleted WallForceX WallForceY WallForceZ\n") ;
            while (!feof(in))
            {
             fscanf(in, "%c", &letter) ; 
             fprintf(out, "%c", letter) ; 
            }
            fclose(in) ; fclose(out) ;
        }
        catch(...) {}
    });
    
    svr.Get(R"(/getwallcontact)", [&](const Request& req, Response& res) {
        try 
        {
            int DS = 100 ;  // WARNING Hardcoded downsampling
            
            std::string username=req.params.at("username") ;
            
            std::ifstream lastrun ; 
            std::string path = username + "/LastRun.txt" ; 
            lastrun.open(path.c_str(), std::ios_base::in) ; 
            if (!lastrun.is_open())
            {
                res.status=404 ; 
                return ; 
            }
            int Nump ; double pRadius, height, width, length, slopeAngle, simTime ; 
            double tfinal, ttotal ; 
            lastrun >> Nump >> pRadius >> height >> width >> length >> slopeAngle >> simTime ;
            lastrun >> tfinal >> ttotal ; 
            if (lastrun.eof())
            {
                printf("This was an unfinished simulation") ; fflush(stdout) ;
                res.status=404 ; 
                return ; 
            }
            
            pugi::xml_document doc;
            path = username + "/protectiveWallInteraction_" + std::to_string(static_cast<int>(floor(tfinal*DS)))+".vtu" ; 
            printf("%g %s\n", tfinal, path.c_str()) ; fflush(stdout) ; 
            pugi::xml_parse_result result = doc.load_file(path.c_str());
            auto points = doc.child("VTKFile").child("PolyData").child("Piece").child("Points").child("DataArray") ; 
            auto numpoints = doc.child("VTKFile").child("PolyData").child("Piece").attribute("NumberOfPoints").as_int() ; 
            
            std::istringstream textcontent(points.child_value()) ;
            std::vector<double> locations, forces ; 
            double tmp ; 
            for (int i=0 ; i<numpoints*3 ; i++)
            {
                textcontent >> tmp ; 
                locations.push_back(tmp) ; 
            }
            
            points = doc.child("VTKFile").child("PolyData").child("Piece").child("PointData") ;
            points = points.find_child_by_attribute("DataArray", "Name", "Force") ; 
            std::istringstream textcontent2(points.child_value()) ;
            for (int i=0 ; i<numpoints*3 ; i++)
            {
                textcontent2 >> tmp ; 
                forces.push_back(tmp) ; 
            }
            
            path = username + "/WallContacts.txt" ; 
            FILE * out ; 
            out = fopen(path.c_str(), "w") ; 
            if (out==NULL)
            {
                res.status=507 ; 
                return ; 
            }
            fprintf(out, "X Y Z ForceX ForceY ForceZ\n") ; 
            for (int i=0 ; i<numpoints*3 ; i+=3)
                if (locations[i]<0.5*pRadius)
                    fprintf(out, "%g %g %g %g %g %g\n", locations[i], locations[i+1], locations[i+2], forces[i], forces[i+1], forces[i+2]) ;  
            fclose(out) ; 
        }
        catch(...) {printf("An error occured") ; fflush(stdout) ; }
    });
    
    svr.Get(R"(/loadprevious)", [&](const Request& req, Response& res) {
        try 
        {
            std::string username=req.params.at("username") ;
            
            std::ifstream lastrun ; 
            std::string path = username + "/LastRun.txt" ; 
            lastrun.open(path.c_str(), std::ios_base::in) ; 
            if (!lastrun.is_open())
            {
                printf("File does not exist\n") ; 
                res.status=404 ; 
                return ; 
            }
            int Nump ; double pRadius, height, width, length, slopeAngle, simTime ; 
            double tfinal, ttotal ; 
            lastrun >> Nump >> pRadius >> height >> width >> length >> slopeAngle >> simTime ;
            lastrun >> tfinal >> ttotal ; 
            
            std::string response="" ; 
            response = "{\"username\":\""+username
                    +"\",\"Nump\":"+std::to_string(static_cast<int>(Nump))
                    +",\"r\":"+std::to_string(static_cast<float>(pRadius))
                    +",\"h\":"+std::to_string(static_cast<float>(height))
                    +",\"w\":"+std::to_string(static_cast<float>(width))
                    +",\"l\":"+std::to_string(static_cast<float>(length))
                    +",\"s\":"+std::to_string(static_cast<float>(slopeAngle))
                    +",\"t\":"+std::to_string(static_cast<float>(simTime))
                    +",\"tfinal\":"+std::to_string(static_cast<float>(tfinal))
                    +"}" ; 
            
            res.set_content(response.c_str(), response.size(), "application/json");
            if (lastrun.eof())
            {
                printf("This was an unfinished simulation") ; fflush(stdout) ;
                res.status=425 ; 
                return ; 
            }
            lastrun.close() ; 
        }
        catch (const std::out_of_range& e)
        {
            printf("User not found\n") ; 
            res.status=403 ; 
        }
        catch(...) {res.status=500 ; printf("An error occured") ; fflush(stdout) ; }
    });

    svr.listen("0.0.0.0", 54321);
    
    return 0;
}
//! [AT_PW:Mainfunction]
