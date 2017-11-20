/* -*- mode: C++ c-basic-offset: 4  -*-
 * main.C - source file for main function
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

#include <map>
#include <string>
#include <fstream>

#include <jlib/util.hh>
#include <jlib/sys.hh>

#include <jlib/Thread.hh>

#include <sigc++/object.h>
#include <sigc++/basic_signal.h>
#include <sigc++/signal_system.h>
#include <sigc++/slot.h>

#include <gtk--/main.h>

#include "SignalChain.hh"

void exit2() {
    static int i = 0;
    i++;
    std::cout << "i=" << i << std::endl;
    if(i == 2) Gtk::Main::quit();
}

void print_hello() { std::cout << "hello, world!" << std::endl; }
bool is_even() { srand(time(NULL)); return (rand()%2); }

void print_even(bool even) { 
    std::cout << "The random number was ";
    if(!even) std::cout << "not ";
    std::cout << "even" << std::endl;
}

int main(int argc, char** argv) {
	try {
        Gtk::Main main(argc,argv);

        gtkmail::SignalChain0<void>* sig0 = 
            new gtkmail::SignalChain0<void>(SigC::slot(&print_hello),true);
        sig0->finish.connect(SigC::slot(&exit2));
        sig0->start();

        gtkmail::SignalChain0<bool>* sigbool =
            new gtkmail::SignalChain0<bool>(SigC::slot(&is_even),true);
        sigbool->succeed.connect(SigC::slot(&print_even));
        sigbool->finish.connect(SigC::slot(&exit2));
        sigbool->start();

        

        main.run();
        
	}
	catch(std::exception& e) {
		std::cout << e.what() << std::endl;
	}
    catch(...) {
        std::cout << "Caught unknown exception, exiting\n";
    }
}
