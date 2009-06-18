/*
  This file is part of LOOS.

  LOOS (Lightweight Object-Oriented Structure library)
  Copyright (c) 2008, Tod D. Romo, Alan Grossfield
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

#if !defined(PDBTRAJ_HPP)
#define PDBTRAJ_HPP


#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>


#include <loos_defs.hpp>
#include <AtomicGroup.hpp>
#include <Trajectory.hpp>

#include <pdb.hpp>


namespace loos {


  //! Class for intepreting separate PDB files as a single trajectory.
  /** This class takes a set of PDB files following a specified naming
   *  pattern and accesses them as though they were a contiguous
   *  trajectory.  You specify a template string (in boost::format
   *  format) and a start and end values along with an optional stride.
   *  Frame numbers are then mapped onto this range.
   *
   *  This first frame is read as part of initialization.
   *
   *  Access to the internal PDB object is permitted, however, see the
   *  CCPDB::currentFrame() for important information regarding its use.
   */
  class PDBTraj : public Trajectory {
  public:

    //! Use the pattern \a s with starting index \a st, ending index \a
    //! en, and option stride \a str.
    /** As an example, suppose you have frames of the trajectory stored
     *  in PDB files named:
     *  -frame_000.pdb
     *  -frame_010.pdb
     *  -frame_020.pdb
     *  You can manipulate these as a trajectory using,
     \code
     PDBTraj foo("frame_%03d.pdb", 0, 20, 10);
     \endcode
    */
    explicit PDBTraj(const std::string& s, uint st, uint en, uint str=1) : Trajectory(), pattern(s), start(st), end(en),
                                                                           stride(str), _natoms(0), _nframes(0), current_index(0),
                                                                           at_end(false) { init(); readFrame(0); cached_first = true; }

    explicit PDBTraj(const char *p, uint st, uint en, uint str=1) : Trajectory(), pattern(std::string(p)),
                                                                    start(st), end(en), stride(str), _natoms(0),
                                                                    _nframes(0), current_index(0), at_end(false) { init(); readFrame(0); cached_first = true; }


    virtual void rewindImpl(void);
    virtual uint nframes(void) const;
    virtual uint natoms(void) const;
    virtual std::vector<GCoord> coords(void);
    virtual void updateGroupCoords(AtomicGroup&);

    virtual void seekNextFrameImpl(void);
    virtual void seekFrameImpl(const uint);
    virtual bool parseFrame(void);

    virtual bool hasPeriodicBox(void) const;
    virtual GCoord periodicBox(void) const;

    virtual float timestep(void) const;

    //! Returns the auto-generated name for the file containing the current frame.
    std::string currentName(void) const;

    //! See CCPDB::currentFrame() for important notes
    PDB currentFrame(void) const;

  private:
    void init(void);

  private:
    std::string pattern;
    uint start, end, stride;
    uint _natoms, _nframes;
    uint current_index;
    bool at_end;
    std::string current_name;
    PDB frame;
  };


}

#endif
