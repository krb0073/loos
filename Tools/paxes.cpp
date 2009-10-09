/*
  paxes.cpp

  (c) 2009 Tod D. Romo, Grossfield Lab
  Department of Biochemistry
  University of Rochster School of Medicine and Dentistry


  Calculates the magnitude of the principal axes (the corresponding
  eigenvalue) over time and writes this out...

  Usage:  paxes mode pdb dcd sel1 sel2 [sel3 ...]
*/


/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2009, Tod Romo
  Department of Biochemistry and Biophysics
  School of Medicine & Dentistry, University of Rochester

  This package (LOOS) is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation under version 3 of the License.

  This package is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <loos.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace loos;



int main(int argc, char *argv[]) {
  if (argc < 4) {
    cerr << "Usage - paxes model trajectory sel1 [sel2 ...]\n";
    exit(-1);
  }

  string hdr = invocationHeader(argc, argv);
  AtomicGroup model = createSystem(argv[1]);
  pTraj traj = createTrajectory(argv[2], model);

  cout << "# " << hdr << endl;
  cout << "# t ";

  vector<AtomicGroup> subsets;
  for (int i=3; i < argc; ++i) {
    AtomicGroup subset = selectAtoms(model, argv[i]);
    subsets.push_back(subset);
    cout << boost::format("a_%d_0 a_%d_1  a_%d_2") % (i-3) % (i-3) % (i-3);
  }
  cout << endl;

  uint t = 0;
  while (traj->readFrame()) {
    traj->updateGroupCoords(model);

    cout << t++ << " ";
    vector<AtomicGroup>::iterator i;
    for (i = subsets.begin(); i != subsets.end(); ++i) {
      vector<GCoord> axes = (*i).principalAxes();
      cout << axes[3][0] << " " << axes[3][1] << " " << axes[3][2] << " ";
    }
    cout << endl;
  }
}


