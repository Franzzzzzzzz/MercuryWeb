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
#include <filesystem>
#include <atomic>
#include "Server.h" //FG

#define MAXPROC 4

struct Userspace {
    enum Policy {admin=0, student=1, none=2} ; 
    enum ProcessStatus {stopped=0, running=1} ; 
    static const std::map <Policy, int> maxTime ;
    Policy userpolicy = none ; 
    ProcessStatus processstatus = stopped ; 
    std::chrono::steady_clock::time_point starttime ; 
    double percentcompleted=0.0 ; 
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
            
            int Nump=sanitize<int>(req.params, "Np", 1,100, 50) ; 
            double pRadius = sanitize<double>(req.params,"r", 0.005,0.5, 0.01) ; //0.01 [m]
            double height  = sanitize<double>(req.params,"h", 0.05, 0.5 , 0.1); //0.1 [m]
            double width   = sanitize<double>(req.params,"w", 0.1, 0.5, 0.25);  //0.25 [m]
            double length  = sanitize<double>(req.params,"l", 0.5, 10, 1.0); //1.0 [m]
            double slopeAngle = sanitize<double>(req.params,"s", 1., 90., 15.0); //15 grades
            double simTime = sanitize<double>(req.params, "t", 1, 100., 5.0); //15 grades
            
            char commande[1001] ; 
            snprintf(commande,1000, "cd %s ; ../../Software/ProtectiveWall -Np %d -r %g -h %g -w %g -l %g -s %g -t %g", username.c_str(), Nump, pRadius, height, width, length, slopeAngle, simTime);
            
            //=========================
            FILE *fp; int status;
            char line[10000] ; 
            user[username].processstatus = Userspace::running ; 
            user[username].starttime = std::chrono::steady_clock::now();
            running++ ; 
            fp = popen (commande, "r") ; 
            if (fp==NULL) {printf("Error: cannot open pipe\n") ; throw ; }
            float tcur=0, tmax=simTime ; 
            while (!feof(fp))
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
            status = pclose(fp);
            user[username].processstatus = Userspace::stopped ; 
            running -- ; 
            if (running<0) running=0 ; // Should never happen, but just to be on the safe side
        }
        catch (const std::out_of_range& e)
        {
            printf("User not found\n") ; 
            res.status=403 ; 
        }
        catch (...)
        {
         printf("Some error occured") ; 
         res.status=400 ; 
        }
    }); // End /run
    
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
    
    
    svr.listen("0.0.0.0", 54321);
    
    return 0;
}
//! [AT_PW:Mainfunction]
