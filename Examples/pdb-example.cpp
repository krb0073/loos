/*
  pdb-example.cpp
  (c) 2008 Tod D. Romo

  Grossfield Lab
  Department of Biochemistry and Biophysics
  University of Rochester Medical School


  Some examples of using the PDB/AtomicGroup classes...
*/



#include <pdb.hpp>


struct CASelector : public AtomSelector {
  bool operator()(const pAtom& atom)  {
    return(atom->name() == "CA");
  }
};



struct SolvSelector : public AtomSelector {
  bool operator()(const pAtom& atom)  {
    return(atom->segid() == "SOLV" || atom->segid() == "BULK");
  }
};


int main(int argc, char *argv[]) {
  
  PDB p(argv[1]);

  cout << "Read in " << p.size() << " atoms from " << argv[1] << endl;

  CASelector casel;
  AtomicGroup cas = p.select(casel);
  
  cout << "There are " << cas.size() << " CAs.\n";
  cout << "The max radius for CAs is " << cas.radius() << endl;

  SolvSelector wasel;
  AtomicGroup water = p.select(wasel);

  int nwater = water.numberOfResidues();
  cout << "There are " << nwater << " waters.\n";
  if (nwater > 0) {
    AtomicGroup::BoundingBox bdd = water.boundingBox();
    cout << "Bounding box for the water is:\n";
    cout << "\t" << bdd.min[0] << " <= x <= " << bdd.max[0] << endl;
    cout << "\t" << bdd.min[1] << " <= y <= " << bdd.max[1] << endl;
    cout << "\t" << bdd.min[2] << " <= z <= " << bdd.max[2] << endl;
  }

  GCoord c = p.centroid();
  cout << "The centroid for the PDB is at " << c << endl;

  AtomicGroup::Iterator iter(cas);
  int i;
  cout << "The first 5 CAs are...\n";
  for (i=0; i<5; i++)
    cout << *(iter()) << endl;

  // Note the implicit conversion back to a PDB...
  PDB terminus = cas.subset(-1, 5);
  terminus.autoTerminate(false);
  cout << "\nThe last 5 CA's are...\n";
  cout << terminus << endl;


  PDB split_ends = cas.subset(0, 5) + cas.subset(-1, 5);
  cout << "\nThe ends combined now...\n";
  cout << split_ends << endl;

  AtomicGroup residue = p.getResidue(cas[0]);
  residue.sort();
  cout << "\nThe first residue is:\n";
  cout << residue << endl;


}

